// Copyright (c) 2021 Cesanta Software Limited
// All rights reserved

#include <Arduino.h>
#include "./src/js/elk.h"
#include "./src/js/mongoose.h"
#include "./src/calls/calls.h"
#include <iostream>
#include <string>

#define JS_MEM_SIZE 8192

extern CALLS call;

static struct js *s_js;      // JS instance
static struct mg_mgr s_mgr;  // Mongoose event manager

// A C resource that requires cleanup after JS instance deallocation For
// example, a network connection, or a timer, are resources that are handled by
// C code. JS just holds a reference to it - an ID of some sort.  When a JS
// instance is deleted, we need to cleanup all C resources that instance has
// allocated. Therefore when we allocate a C resource and give away a handle to
// the JS, we insert a "deallocation descriptor", struct resource, to the list
// s_rhead.
struct resource {
  struct resource *next;    // Next resource
  void (*cleanup)(void *);  // Cleanup function
  void *data;               // Resource data
};
static struct resource *s_rhead;  // Allocated C resources

static void addresource(void (*fn)(void *), void *data) {
  struct resource *r = (struct resource *) calloc(1, sizeof(*r));
  r->data = data;
  r->cleanup = fn;
  r->next = s_rhead;
  s_rhead = r;
  MG_INFO(("added r=%p, data=%p, cleanup=%p", r, r->data, r->cleanup));
}

static void delresource(void (*fn)(void *), void *data) {
  struct resource **head = &s_rhead, *r;
  while (*head && (*head)->cleanup != fn && (*head)->data != data)
    head = &(*head)->next;
  if ((r = *head) != NULL) {
    MG_INFO(("deleting r=%p, data=%p, cleanup=%p, next=%p", r, r->data,
             r->cleanup, r->next));
    *head = r->next, r->cleanup(r->data), free(r);
    MG_INFO(("head: %p", s_rhead));
  }
}

static void logstats(void) {
  size_t a = 0, b = 0, c = 0;
  js_stats(s_js, &a, &b, &c);
  MG_INFO(("Free C RAM: %u, JS RAM: total %u, lowest free %u, C stack: %u",
           esp_get_free_heap_size, (unsigned) a, (unsigned) b, (unsigned) c));
}

// These functions below will be imported into the JS engine.
// Note that they are inside the extern "C" section.
static jsval_t gpio_write(struct js *js, jsval_t *args, int nargs) {
  if (!js_chkargs(args, nargs, "dd")) return js_mkerr(js, "bad args");
  int pin = js_getnum(args[0]), val = js_getnum(args[1]);
  MG_INFO(("gpio.write %d -> %d", pin, val));
  digitalWrite(pin, val);
  return js_mknull();
}

static jsval_t gpio_read(struct js *js, jsval_t *args, int nargs) {
  if (!js_chkargs(args, nargs, "d")) return js_mkerr(js, "bad args");
  int pin = js_getnum(args[0]);
  MG_INFO(("gpio.read %d", pin));
  return js_mknum(digitalRead(pin));
}

static jsval_t gpio_mode(struct js *js, jsval_t *args, int nargs) {
  if (!js_chkargs(args, nargs, "dd")) return js_mkerr(js, "bad args");
  int pin = js_getnum(args[0]), mode = js_getnum(args[1]);
  MG_INFO(("gpio.mode %d -> %d", pin, mode));
  pinMode(pin, mode);
  return js_mknull();
}

static void timer_cleanup(void *data) {
  unsigned long id = (unsigned long) data;
  struct mg_timer **head = (struct mg_timer **) &s_mgr.timers, *t;
  while (*head && (*head)->id != id) head = &(*head)->next;
  if ((t = *head) != NULL) {
    MG_INFO(("%lu (%s)", id, (char *) t->arg));
    *head = t->next, free(t->arg), free(t);
  }
}

static void js_timer_fn(void *userdata) {
  char buf[200];
  mg_snprintf(buf, sizeof(buf), "%s();", (char *) userdata);
  MG_INFO(("Calling JS: %s", buf));
  jsval_t res = js_eval(s_js, buf, ~0U);
  if (js_type(res) == JS_ERR)
    MG_ERROR(("%s: %s", (char *) userdata, js_str(s_js, res)));
  logstats();
}

static jsval_t mktimer(struct js *js, jsval_t *args, int nargs) {
  if (!js_chkargs(args, nargs, "ds")) return js_mkerr(js, "bad args");
  int milliseconds = js_getnum(args[0]);
  const char *funcname = js_getstr(js, args[1], NULL);
  struct mg_timer *t = mg_timer_add(&s_mgr, milliseconds, MG_TIMER_REPEAT,
                                    js_timer_fn, strdup(funcname));
  MG_INFO(("mktimer %lu, %d ms, fn %s", t->id, milliseconds, funcname));
  addresource(timer_cleanup, (void *) t->id);
  return js_mknum(t->id);
}

static jsval_t deltimer(struct js *js, jsval_t *args, int nargs) {
  if (!js_chkargs(args, nargs, "d")) return js_mkerr(js, "bad args");
  delresource(timer_cleanup, (void *) (unsigned long) js_getnum(args[0]));
  return js_mknull();
}

static jsval_t js_log(struct js *js, jsval_t *args, int nargs) {
  char buf[1024];
  size_t n = 0;
  for (int i = 0; i < nargs; i++) {
    const char *space = i == 0 ? "" : " ";
    n += mg_snprintf(buf + n, sizeof(buf) - n, "%s%s", space,
                     js_str(js, args[i]));
  }
  buf[sizeof(buf) - 1] = '\0';
  MG_INFO(("JS-> %s", buf));
  return js_mkundef();
}

static jsval_t js_delay(struct js *js, jsval_t *args, int nargs) {
  long ms = (long) js_getnum(args[0]);
  MG_INFO(("%ld", ms));
#ifndef __linux__
  delay(ms);
#endif
  return js_mkundef();
}

static jsval_t read_sensor(struct js *js, jsval_t *args, int nargs){

  if (!js_chkargs(args, nargs, "s")) return js_mkerr(js, "bad args");

  const char *ref = js_getstr(js, args[0], NULL);
  MG_INFO(("read sensor %s",ref));
  int value = 1;
  char run[200];
  mg_snprintf(run,sizeof(run),"e.onSensorRead(\"%s\",%d)",(char*)ref,value);
  jsval_t v = js_eval(s_js, run, ~0);     // Execute JS code
  MG_INFO(("%s -> %s\n",run, js_str(js, v)));        // result: 7
  return js_mkundef();
}

static jsval_t date_millis(struct js *js, jsval_t *args, int nargs){

  return js_mknum(mg_millis());
}

static jsval_t date_now(struct js *js, jsval_t *args, int nargs){

  return js_mknum(now());
}

static jsval_t js_mqtt_send(struct js *js, jsval_t *args, int nargs){

  if(nargs != 3) return js_mkerr(js, "mqtt few args");
  //if (!js_chkargs(args, nargs, "sdd")) return js_mkerr(js, "mqtt bad args");


  String data_str = "";
  const char *topic = js_getstr(js, args[0], NULL);
  //const char *payload = js_getstr(js, args[1], NULL);
  if(js_type(args[1]) == JS_STR){
    char *data = (char*)js_getstr(js, args[1], NULL);
    data_str = String(data);
  }else if(js_type(args[2] == JS_NUM)){
    double value = (double)js_getnum(args[1]);
    data_str = String(value);
  }
  int retain = (int) js_getnum(args[2]);

  MG_INFO(("topic %s : payload %s ",topic,data_str.c_str()));

  call.mqtt_send(0,String(topic),data_str,0,retain);

  return js_mkundef();
}

static struct js *jsinit(void *mem, size_t size) {
  struct js *js = js_create(mem, size);

  js_set(js, js_glob(js), "log", js_mkfun(js_log));
  js_set(js, js_glob(js), "delay", js_mkfun(js_delay));

  jsval_t gpio = js_mkobj(js);
  js_set(js, js_glob(js), "gpio", gpio);
  js_set(js, gpio, "write", js_mkfun(gpio_write));
  js_set(js, gpio, "mode", js_mkfun(gpio_mode));
  js_set(js, gpio, "read", js_mkfun(gpio_read));

  jsval_t timer = js_mkobj(js);
  js_set(js, js_glob(js), "timer", timer);
  js_set(js, timer, "create", js_mkfun(mktimer));
  js_set(js, timer, "delete", js_mkfun(deltimer));

  jsval_t sensor = js_mkobj(js);
  js_set(js, js_glob(js), "sensor", sensor);
  js_set(js, sensor, "read", js_mkfun(read_sensor));

  jsval_t date = js_mkobj(js);
  js_set(js, js_glob(js), "date", date);
  js_set(js, date, "now", js_mkfun(date_now));
  js_set(js, date, "millis", js_mkfun(date_millis));
  
  jsval_t mqtt = js_mkobj(js);
  js_set(js, js_glob(js), "mqtt", mqtt);
  js_set(js, mqtt, "send", js_mkfun(js_mqtt_send));

  return js;
}

static void log_cb(uint8_t ch) {
  static char buf[256];
  static size_t len;
  buf[len++] = ch;
  if (ch == '\n' || len >= sizeof(buf)) {
    fwrite(buf, 1, len, stdout);
    char *data = mg_mprintf("{%Q:%Q,%Q:%V}", "name", "log", "data", len, buf);
    for (struct mg_connection *c = s_mgr.conns; c != NULL; c = c->next) {
      if (!c->is_websocket) continue;
      mg_ws_send(c, data, strlen(data), WEBSOCKET_OP_TEXT);
    }
    free(data);
    len = 0;
  }
}


class JS {
 public:
  JS() {
  }
  void begin(void) {
    s_js = jsinit(malloc(JS_MEM_SIZE), JS_MEM_SIZE);
    // mg_log_set("3");
    mg_mgr_init(&s_mgr);
    mg_log_set_fn(log_cb);
  }

  const char* exec(const char* code){
    if (code) {
      // Deallocate all resources
      while (s_rhead != NULL) delresource(s_rhead->cleanup, s_rhead->data);
      s_js = jsinit(s_js, JS_MEM_SIZE);
      jsval_t v = js_eval(s_js, code, ~0U);
      //free(code);
      return mg_mprintf("%Q", js_str(s_js, v));
    } else {
      return mg_mprintf("%Q", "missing code");
    }
  }

  void loop(void) {
    mg_mgr_poll(&s_mgr, 100);
  }
};

static JS JS;

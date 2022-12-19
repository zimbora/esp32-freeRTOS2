
#include "core.h"
#include <TaskScheduler.h>

Scheduler runner;
CALLS call;
SYSFILE sysfile;
SemaphoreHandle_t spiffsMutex;
APP app;

MODEMfreeRTOS mRTOS; // freeRTOS modem
MQTT_MSG* msg; // mqtt
DynamicJsonDocument doc(2048); // json

String uid = "";

#ifdef ENABLE_BLE
BLE_SERVER ble;
void bleCallback(String uuid, String value){

  String param = "";

  switch(resolveOptionUUID(uuidTopics,uuid)){

    case uuid_fw_reboot:
      if(value == "1")
        call.fw_reboot();
      break;

    case uuid_fw_reset:
      if(value == "1"){
        call.fw_reset();
        call.fw_reboot();
      }
      break;

    case uuid_wifi_mode:
      param = "wifi_mode";
      break;

    case uuid_wifi_ssid:
      param = "wifi_ssid";
      break;

    case uuid_wifi_pwd:
      param = "wifi_pwd";
      break;

    case uuid_modem_apn:
      param = "modem_apn";
      break;

    case uuid_modem_user:
      param = "modem_user";
      break;

    case uuid_modem_pwd:
      param = "modem_pwd";
      break;

    case uuid_modem_band:
      param = "modem_band";
      break;

    case uuid_modem_cops:
      param = "modem_cops";
      break;

    case uuid_mqtt_host:
      param = "mqtt_host";
      break;

    case uuid_mqtt_user:
      param = "mqtt_user";
      break;

    case uuid_mqtt_pass:
      param = "mqtt_pass";
      break;

    case uuid_mqtt_prefix:
      param = "mqtt_prefix";
      break;

    case uuid_mqtt_port:
      param = "modem_port";
      break;

    case uuid_mqtt_active:
      param = "modem_active";
      break;

    case uuid_log_active:
      param = "log_active";
      break;

    case uuid_log_level:
      param = "log_level";
      break;

    case uuid_keepalive_active:
      param = "keepalive_active";
      break;

    case uuid_keepalive_period:
      param = "keepalive_period";
      break;
  }

  Serial.println("param:"+param);
  Serial.println("value:"+value);
  if(settings_set_param(param,value)){
    settings_log();
    if(!sysfile.write_file(FW_SETTINGS_FILENAME,settings.fw.version,sizeof(settings)))
      DBGLOG(Error,"failing writing file: "+String(FW_SETTINGS_FILENAME));
  }


}
#endif

void tRecords(){

  core_check_records();
  String directory = APP_PATH_RECORDS;
  call.clean_dir(directory);
}

void tKeepalive(){


  core_send_mqtt_message(CLIENTID,"/uptime",String(millis()/1000),0,false);
  core_send_mqtt_message(CLIENTID,"/rssi",String(mRTOS.get_rssi()),0,false);
  core_send_mqtt_message(CLIENTID,"/tech",String(mRTOS.get_technology()),0,false);
  //core_send_mqtt_message(CLIENTID,"/timestamp",String(now()),0,false);

}

void tInfo(){

  Serial.printf("\n\n----- Info -----\n");
  Serial.println("heap free: " + String(ESP.getFreeHeap() / 1024) + " KiB");
  DBGLOG(Info,date());
  mRTOS.log_modem_status();
  Serial.printf("--- ----- --- \n\n");

}

Task t1(100, TASK_FOREVER, &core_parse_mqtt_messages); // call it outside functions
Task t2(settings.keepalive.period*1000, TASK_FOREVER, &tKeepalive); // call it outside functions
Task t3(1000, TASK_FOREVER, &tRecords); // call it outside functions
Task t4(60000, TASK_FOREVER, &tInfo); // call it outside functions

String directory[] = {
  SYS_PATH_SETTINGS,
  APP_PATH_SETTINGS,
  APP_PATH_RECORDS,
};


void core_init(){

  spiffsMutex = xSemaphoreCreateMutex();

  sysfile.init();
  //sysfile.format();

  call.init_filesystem(directory,NUMITEMS(directory));

  call.fw_settings_load(FW_SETTINGS_FILENAME, FW_VERSION);

  // log settings
  settings_log();

  uid = get_uid();

  #ifdef ENABLE_BLE
  ble.init(uid);
  ble.setCallback(&bleCallback);
  ble.enable();
  #endif

  runner.init();
  DBGLOG(Debug,"Initialized scheduler");

  runner.addTask(t1);
  DBGLOG(Debug,"added t1");

  t1.enable();
  DBGLOG(Debug,"Enabled parse mqtt messages");

  if(settings.keepalive.active){
    runner.addTask(t2);
    DBGLOG(Debug,"added t2");

    t2.enable();
    DBGLOG(Debug,"Enabled keepalive task");
  }

  runner.addTask(t3);
  DBGLOG(Debug,"added t3");

  t3.enable();
  DBGLOG(Debug,"Enabled records task");

  runner.addTask(t4);
  DBGLOG(Debug,"added t4");

  t4.enable();
  DBGLOG(Debug,"Enabled info logs");

  #ifdef ENABLE_JS
  JS.begin();

  char* code = (char*)malloc(1000);
  if(code != nullptr){
    uint16_t len = 1000;
    if(call.read_file(FW_JS_FILENAME,code,&len)){
      if(len != 0){
        const char* res = JS.exec((const char*)code);
        Serial.printf("%s\n", res);
      }else DBGLOG(Info,"no JS script is empty..");
    }else DBGLOG(Info,"no JS script found..");
  }
  free(code);
  /*
  const char* code = "let e = {onSensorRead: function(name,value){log(name); if(value === 1) mqtt.send(\"/sensor/\"+name,value,0); return true;}}; let timer_id = timer.create(2000,'sensor.read(\"asd\")');";
  const char* res = JS.exec(code);
  Serial.printf("%s\n", res);
  */
  #endif
}

void core_loop(){

  runner.execute();

  #ifdef ENABLE_JS

  JS.loop();
  /*
  if(now() % 15 == 0){
    char* mem = (char*)malloc(200);
    struct js *js = js_create(mem, sizeof(mem));  // Create JS instance
    jsval_t v = js_eval(js, "event.onReadSensor(\"voltage\",230)", ~0);     // Execute JS code
    printf("result: %s\n", js_str(js, v));        // result: 7
  }
  */
  #endif

}

/*
* Do not edit it
*/
void core_parse_mqtt_messages(){
  msg = mRTOS.mqtt_getNextMessage(msg);

  int16_t index = -1;

  // --- PARSE MQTT MESSAGES HERE ---
  if(msg == NULL)
    return;

  DBGLOG(Debug,"new mqtt message received:");
  DBGLOG(Debug,"<< ["+String(msg->clientID)+"] "+String(msg->topic));

  bool set = false;
  bool get = false;
  bool store = false;

  uint8_t clientID = msg->clientID;
  String topic = String(msg->topic);
  String topic_get = "";
  topic.replace("\"","");
  String payload = String(msg->data);
  if(payload.startsWith("\""))
    payload = payload.substring(1,payload.length()-1);

  index = topic.indexOf(uid);
  if(index > -1)
    topic = topic.substring(index+uid.length());

  if(topic == "/status"){
      DBGLOG(Info,"update clock..");
      mRTOS.update_clock_sys();
  }else if(topic.startsWith("/fw")){

    if(topic.endsWith("/set")){
      set = true;
      if(payload == "")
        return;
    }

    if(topic.endsWith("/get")){
      get = true;
      index = topic.lastIndexOf("/");
      topic_get = topic.substring(0,index); // get filtered
    }

    Serial.println("topic: "+topic);
    switch(resolveOption(fwTopics,topic)){
      case fw_:
        DBGLOG(Info,"getting fw info..");
        //mRTOS.mqtt_pushMessage(clientID,topic,"{\"version\":\""+String(PACKAGE_VERSION)+"\",\"model\":"+String(PACKAGE_MODEL)+",\"hash\":\""+String(settings.fw.hash)+"\""+",\"uptime\":"+String(millis())+"}",1,true);
        core_send_mqtt_message(clientID,topic_get,String(FW_VERSION),2,true);
        break;
      case fw_reboot_:

        if(payload != "1") return;

        DBGLOG(Info,"reboot..");
        // !! unpublish topic
        //mqtt_pushMessage(clientID,topic+"/set","",1,true);
        //flag_restart = true;
        call.fw_reboot();
        break;
      case fw_reset_:

        if(payload != "1")
          return;

        DBGLOG(Info,"reset..");
        // !! unpublish topic
        //mqtt_pushMessage(clientID,topic+"/set","",1,true);
        call.fw_reset();
        call.fw_reboot();
        break;
      case fw_info_:
        DBGLOG(Info,"getting fw info..");
        //mqtt_pushMessage(clientID,topic,"{\"gps\":\"\",\"rssi\":0,\"timestamp\":"+String(getTimestamp())+",\"uptime\":"+String(millis())+"}",1,true);
        break;
      case fw_clean_records_:
        if(payload != "1")
          return;
        call.remove_dir(APP_PATH_RECORDS);
        break;
      case fw_fota_update_:
        {
          DeserializationError error = deserializeJson(doc, payload);
          if(error){
            DBGLOG(Error,"Not Json");
            return;
          }

          if(doc.containsKey("url")){
            String url = doc["url"];
            DBGLOG(Info,"updating settings from "+url);
            call.fw_fota(url);
          }

        }
        break;
      case fw_settings_update_:
        {
          DeserializationError error = deserializeJson(doc, payload);
          if(error){
            DBGLOG(Error,"Not Json");
            return;
          }

          if(doc.containsKey("url")){
            String url = doc["url"];
            DBGLOG(Info,"updating fw from "+url);
            call.fw_settings_update(url,FW_SETTINGS_FILENAME);
          }

        }
        break;
      case fw_js_code_:
        {
          #ifdef ENABLE_JS
          Serial.println(payload);
          const char* res = JS.exec(payload.c_str());
          Serial.printf("%s\n", res);
          if(!call.write_file(FW_JS_FILENAME,payload.c_str(),payload.length()))
            DBGLOG(Error,"Error storing js script");
          #else
          Serial.println("JS not enabled");
          #endif
        }
        break;
      case fw_wifi_:
      {
        DeserializationError error = deserializeJson(doc, payload);
        if(error){
          DBGLOG(Error,"Not Json");
          return;
        }
        store = true;
        if(doc.containsKey("mode")){
          #ifndef UNITTEST
            String mode = doc["mode"];
          #else
            String mode = "";
            if(doc["mode"].is_string())
              mode = doc["mode"];
          #endif
          settings_set_param("wifi_mode",mode);
        }

        if(doc.containsKey("ssid")){
          #ifndef UNITTEST
            String ssid = doc["ssid"];
          #else
            String ssid = "";
            if(doc["ssid"].is_string())
              ssid = doc["ssid"];
          #endif
          settings_set_param("wifi_ssid",ssid);
        }

        if(doc.containsKey("pwd")){
          #ifndef UNITTEST
            String pwd = doc["pwd"];
          #else
            String pwd = "";
            if(doc["pwd"].is_string())
              pwd = doc["pwd"];
          #endif
          settings_set_param("wifi_pwd",pwd);
        }
      }
        break;
      case fw_modem_:
      {
        DeserializationError error = deserializeJson(doc, payload);
        if(error){
          DBGLOG(Error,"Not Json");
          return;
        }
        store = true;
        if(doc.containsKey("apn")){
          #ifndef UNITTEST
            String apn = doc["apn"];
          #else
            String apn = "";
            if(doc["apn"].is_string())
              apn = doc["apn"];
          #endif
          settings_set_param("modem_apn",apn);
        }

        if(doc.containsKey("pwd")){
          #ifndef UNITTEST
            String pwd = doc["pwd"];
          #else
            String pwd = "";
            if(doc["pwd"].is_string())
              pwd = doc["pwd"];
          #endif
          settings_set_param("modem_pwd",pwd);
        }

        if(doc.containsKey("user")){
          #ifndef UNITTEST
            String user = doc["user"];
          #else
            String user = "";
            if(doc["user"].is_string())
              user = doc["user"];
          #endif
          settings_set_param("modem_user",user);
        }

        if(doc.containsKey("band")){
          #ifndef UNITTEST
          String band = doc["band"];
          #else
          String band = "";
          if(doc["band"].is_number())
            band = std::to_string((long)doc["band"]);
          #endif
          settings_set_param("modem_band",band);
        }

        if(doc.containsKey("cops")){
        #ifndef UNITTEST
        String cops = doc["cops"];
        #else
        String cops = "";
        if(doc["cops"].is_number())
          cops = std::to_string((long)doc["cops"]);
        #endif
        settings_set_param("modem_cops",cops);
      }

      }
        break;
      case fw_mqtt_:
      {
        DBGLOG(Debug,"updating mqtt");

        DeserializationError error = deserializeJson(doc, payload);
        if(error){
          DBGLOG(Error,"Not Json");
          return;
        }

        //serializeJson(doc,Serial);
        store = true;
        // checked
        if(doc.containsKey("host")){
          #ifndef UNITTEST
            String host = doc["host"];
          #else
            String host = "";
            if(doc["host"].is_string())
              host = doc["host"];
          #endif
          settings_set_param("mqtt_host",host);
        }

        if(doc.containsKey("user")){
          #ifndef UNITTEST
            String user = doc["user"];
          #else
            String user = "";
            if(doc["user"].is_string())
              user = doc["user"];
          #endif
          settings_set_param("mqtt_user",user);
        }

        if(doc.containsKey("pass")){
          #ifndef UNITTEST
            String pass = doc["pass"];
          #else
            String pass = "";
            if(doc["pass"].is_string())
              pass = doc["pass"];
          #endif
          settings_set_param("mqtt_pass",pass);
        }

        if(doc.containsKey("prefix")){
          #ifndef UNITTEST
            String prefix = doc["prefix"];
          #else
            String prefix = "";
            if(doc["prefix"].is_string())
              prefix = doc["prefix"];
          #endif
          settings_set_param("mqtt_prefix",prefix);
        }

        if(doc.containsKey("port")){
          #ifndef UNITTEST
          String port = doc["port"];
          #else
          String port = "";
          if(doc["port"].is_number())
            port = std::to_string((long)doc["port"]);
          #endif
          settings_set_param("mqtt_port",port);
        }

        if(doc.containsKey("active")){
          #ifndef UNITTEST
          String active = doc["active"];
          #else
          String active = "";
          if(doc["active"].is_number())
            active = std::to_string((long)doc["active"]);
          #endif
          settings_set_param("mqtt_active",active);
        }
      }
        break;
      case fw_log_:
      {
        DeserializationError error = deserializeJson(doc, payload);
        if(error){
          DBGLOG(Error,"Not Json");
          return;
        }
        store = true;
        if(doc.containsKey("active")){
          #ifndef UNITTEST
          String active = doc["active"];
          #else
          String active = "";
          if(doc["active"].is_number())
            active = std::to_string((long)doc["active"]);
          #endif
          settings_set_param("log_active",active);
        }

        if(doc.containsKey("level")){
          #ifndef UNITTEST
          String level = doc["level"];
          #else
          String level = "";
          if(doc["level"].is_number())
            level = std::to_string((long)doc["level"]);
          #endif
          settings_set_param("log_level",level);
        }
      }
        break;
      case fw_keepalive_:
      {
        DeserializationError error = deserializeJson(doc, payload);
        if(error){
          DBGLOG(Error,"Not Json");
          return;
        }

        if(doc.containsKey("active")){
          #ifndef UNITTEST
          String active = doc["active"];
          #else
          String active = "";
          if(doc["active"].is_number())
            active = std::to_string((long)doc["active"]);
          #endif
          settings_set_param("keepalive_active",active);
        }

        if(doc.containsKey("period")){
          #ifndef UNITTEST
          String period = doc["period"];
          #else
          String period = "";
          if(doc["period"].is_number())
            period = std::to_string((long)doc["period"]);
          #endif
          settings_set_param("keepalive_period",period);
        }
      }
        break;
      default:
        DBGLOG(Info,"topic not known by fw topics");
        break;
    }
    // store settings
    if(store){
      settings_log();
      if(!sysfile.write_file(FW_SETTINGS_FILENAME,settings.fw.version,sizeof(settings)))
        DBGLOG(Error,"failing writing file: "+String(FW_SETTINGS_FILENAME));
    }

    if(set)
      core_send_mqtt_message(clientID,topic,"",0,true); // unpublish

  }else{

    app.parse_mqtt_messages(clientID,topic,payload);
  }


}

bool core_send_mqtt_message(uint8_t clientID, String topic, String data, uint8_t qos, bool retain){

  call.mqtt_send(clientID,topic,data,qos,retain);
}

bool core_store_record(String filename, const char* data, uint16_t len){
  String root = APP_PATH_RECORDS;
  //String path = root + "/"+ String(millis()/1000);
  String path = root + "/"+ String(now());
  if(!call.create_dir(path))
    return false;
  path += filename;
  if(!call.store_record(path,data,sizeof(data)))
    return false;
  return true;
}

void core_check_records(){

  uint8_t clientID = CLIENTID;
  if(settings.mqtt.active)
    clientID = CLIENTIDEXTERNAL;

  //Serial.println("check records");
  if(!mRTOS.mqtt_isConnected(clientID))
    return;

  //Serial.println("check filesystem for new records");
  uint32_t timeout = millis() + 5000;
  bool (*send_ar)(String);
  send_ar = &core_send_record;
  String path = APP_PATH_RECORDS;
  call.check_filesystem_records(path.c_str(),timeout,send_ar); // iterate through all subdirectories

}

// !! callback - sysfile safe
bool core_send_record(String filename){

  // This is a safe function
  // Inside it you can call sysfile class

  uint16_t len = MAX_RECORD_FILE_SIZE;
  char* data = (char*)malloc(len);
  if(data == nullptr) return false;

  memset(data,0,len);

  sysfile.read_file(filename.c_str(),data,&len);

  String filename_bck = filename;
  String root_path = APP_PATH_RECORDS;
  uint8_t index = filename.indexOf(root_path.c_str());
  String path = filename.substring(index+root_path.length());
  index = path.lastIndexOf(".");
  path = path.substring(0,index);
  String topic = MQTT_PATH_RECORDS;
  topic += path;
  DBGLOG(Debug,"send record: "+topic);
  uint8_t clientID = CLIENTID;
  if(settings.mqtt.active)
    clientID = CLIENTIDEXTERNAL;
  if(core_send_mqtt_message(clientID,topic,String(data),2,1)){
    if(!sysfile.delete_file(filename_bck.c_str()))
      DBGLOG(Error,"Couldn't delete file: "+filename_bck);
  }else{
    free(data);
    return false;
  }

  free(data);
  return true;
}

uint8_t parse_float_array(float* arr, uint8_t len, String payload){

    int8_t index = 0, i = 0;
    String value = "";

    while(i < len){
      index = payload.indexOf(",");

      if(index == -1){
        value = payload;
        if(payload.length() > 0){
          arr[i++] = value.toFloat();
          //Serial.printf("value: %f\n",arr[i-1]);
        }
        break;
      }else{
        value = payload.substring(0,index);
        arr[i++] = value.toFloat();
        //Serial.printf("value: %f\n",arr[i-1]);
      }
      payload = payload.substring(index+1);
    }
    return i;

}

// find fw topic
fwTopics_ resolveOption(std::map<long, fwTopics_> map, String topic) {

  std::string topic_ = std::string(topic.c_str());
  long str_hash = (long)std::hash<std::string>{}(topic_);
  std::map<long,fwTopics_>::iterator it;

  it = map.find(str_hash);
  if(it != map.end())
    return it->second;

  return fw_not_found;
}

#ifdef ENABLE_BLE
// find settings param
uuidTopics_ resolveOptionUUID(std::map<long, uuidTopics_> map, String param) {

  std::string param_ = std::string(param.c_str());
  long str_hash = (long)std::hash<std::string>{}(param_);
  std::map<long,uuidTopics_>::iterator it;

  it = map.find(str_hash);
  if(it != map.end())
    return it->second;

  return uuid_not_found;
}
#endif

String get_uid() {

  if(uid != "")
    return uid;

	String mac = WiFi.macAddress();
	// drop ':''
	mac.replace(":", "");
	// lower letters
	mac.toLowerCase();

	return "uid:"+mac;
}

String date() {
	return String(year()) + "-" + pad2(month()) + "-" + pad2(day()) + " " + pad2(hour()) + ":" + pad2(minute()) + ":" + pad2(second());
}

String pad2(int value) {
	return String(value < 10 ? "0" : "") + String(value);
}

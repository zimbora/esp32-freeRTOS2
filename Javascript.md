
#Javascript

Built using open project https://github.com/cesanta/elk
Check referred link for instructions

Javascript code is uploaded using the mqtt topic described below.
The payload should contain the script to be runned. However, that script
have to follow the rules described in this file.
Some examples are shown in the end of this document

## MQTT Topic
  esp32/freeRTOS2/uid:ac67b2e9d11c/fw/js/code/set

## Supported features

- Operations: all standard JS operations except:
   - `!=`, `==`. Use strict comparison `!==`, `===`
   - No computed member access `a[b]`
   - No exponentiation operation `a ** b`
- Typeof: `typeof('a') === 'string'`
- For loop: `for (...;...;...)  ...`
- Conditional: `if (...) ... else ...`
- Ternary operator `a ? b : c`
- Simple types: `let a, b, c = 12.3, d = 'a', e = null, f = true, g = false;`
- Functions: `let f = function(x, y) { return x + y; };`
- Objects: `let obj = {f: function(x) { return x * 2}}; obj.f(3);`
- Every statement must end with a semicolon `;`
- Strings are binary data chunks, not Unicode strings: `'Київ'.length === 8`

## Not supported features

- No `var`, no `const`. Use `let` (strict mode only)
- No `do`, `switch`, `while`. Use `for`
- No `=>` functions. Use `let f = function(...) {...};`
- No arrays, closures, prototypes, `this`, `new`, `delete`
- No standard library: no `Date`, `Regexp`, `Function`, `String`, `Number`


## Available functions

This editor lets you customise ESP32 firmware with JavaScript. All components are open source:

elk - a JS engine
mongoose - a networking library
Eps32JS - ESP32 firmware and this editor
C API imported to JS (Default):
- gpio.mode(pin, mode) - set pin mode
- gpio.write(pin, value) - set pin state
- gpio.read(pin) - read pin state
- timer.create(milli, func, null) - create timer
- timer.delete(timerID) - delete timer
- log(strval) - log a message

C API imported to JS (Proprietary):
- sensor.read(ref) // get last value of sensor mapped with ref id string
- sensor.read_rs485(unit_id,fc,address,len) // real time reads directly from bus
- sensor.write_rs485(unit_id,fc,address,len,data_str) // real time reads directly from bus
- sensor.read_rs485_all() // read all registered rs485 sensors
- date.now() // unix timestamp
- date.millis() // millis()
- mqtt.send(topic,payload,retain) // send messages to mqtt broker

Note:
- log method send all data to mqtt broker with topic /js/log
- topic /js/debug will display all debug messages from runtime js code

#Examples


### Example 1
  ```
let led = { pin: 26, on: 0 };  // LED state
gpio.mode(led.pin, 2);         // Set LED pin to output mode

let timer_fn = function() {
led.on = led.on ? 0 : 1;
gpio.write(led.pin, led.on);
};

// Start a timer that toggles LED every 1000 milliseconds
// This timer is a C resource. It is cleaned automatically on JS code refresh
let timer_id = timer.create(1000, 'timer_fn');
  ```

### Example 2
  ```
log("run");
let doIt = function(){
  mqtt.send("/sensor/uptime",date.now(),0);
};
timer.create(2000,'doIt');
  ```

### Example 3
  ```
let event = {
  onReadSensor : function(ref,value){
    log("js sensor: "+ref);
    log(value);
    if(typeof(value) === 'number'){
      if(value < 300.0)
        log("value is safe");
    }else{
      log("invalid value");
    }
  },
  onAlarmSensor : function(ref,value){
    log("js alarm: "+ref);
  },
  onAlarmTrigger : function(ref,value){
    log("js alarm state changed: "+ref);
  }
};
  ```

### Example 4

Read all rs485 autorequests, stores on eeprom and sends to mqtt broker as soon as connection is available
  ```
let timer_fn = function() {
  sensor.read_rs485_all();
};
let timer_id = timer.create(60000, 'timer_fn');
  ```

### Example 5

Read rs485
  ```
let res = sensor.read_rs485(1,4,0,2);
log(res);
  ```

### Example 6

Write rs485
```
sensor.write_rs485(1,16,22,2,"043F800000");
```

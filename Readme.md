# freeRTOS2

## IMPORTANT
  !! now() method cannot be called outside main process

## SYSFILE

  On startup 3 directories are created:
  - /etc/settings
  - /home/app/settings
  - /home/app/records

#### /etc/settings

  Stores system configuration

#### /app/settings

  Can be used by user to store app configurations
  Use sysfile.write_file() method to store data
  and sysfile.read_file() to read data

#### /app/records

  Use this directory to store data that should be sent to mqtt broker
  An autonoumous service will be checking for new files and always that is possible it will send to mqtt broker

## Commands

  Commands are passed through MQTT with topic prefix/fw/#
  The following commands are available

  On receive (topic:payload):
  - :prefix/status (update clock)
  - :prefix/fw/reboot/set : 1 (reboots system)
  - :prefix/fw/reset/set : 1 (reset memory)
  - :prefix/fw/fota/update/set : {"url":""} (updates fw)

## Configurations

  Configurations are done through MQTT with topic :prefix/fw/#
  The following commands are available

  On receive
  - :prefix/fw/modem/set : {"apn":"","pwd":"","user":"","band":(uint8_t),"cops":(uint16_t)} (updates modem settings)
  - :prefix/fw/mqtt/set : {"host":"","user":"","pass":"","active":(bool)} (configure 2nd mqtt) connection)
  - :prefix/fw/keepalive/set : {"active":(bool),"period":(seconds)}

## MQTT

  Device can have one or two connections
  The first connection will be always enabled
  A second one can be activated and in that case all publish messages in app directory will
  be redirected for this second connection

### Prefix

  Prefix identifies device in MQTT and it is formed by project + uid
  prefix = MQTT_PROJECT+"/"+uid

  MQTT_PROJECT is defined in credentials file
  uid = "mac:"+macAddress

### Subscriptions

  Device subscribes the following topics:
   - prefix/status
   - prefix/fw/#
   - prefix/app/#

  #/fw/# topic is reserved for device configurations

### On Connection

sends the following topics:
  - /status - online
  - /model - FW_MODEL
  - /version - FW_VERSION
  - /uptime
  - /reboot_cause_cpu0
  - /reboot_cause_cpu1

### Running

sends the following topics with keepalive period:
  - /uptime
  - /rssi
  - /tech

## Clocksync
  Clock is synched when a ':prefix/status' msg is received

## JS

Built using open project https://github.com/cesanta/elk
Check referred link for instructions

### Functions

Default:
- gpio.mode(pin, mode) - set pin mode
- gpio.write(pin, value) - set pin state
- gpio.read(pin) - read pin state
- timer.create(milli, func, null) - create timer
- timer.delete(timerID) - delete timer
- log(strval) - log a message

Custom:
- read.sensor(ref)
- date.now()
- date.millis()
- mqtt.send(topic,payload,retain)

### New function

### Available Functions

### MQTT Topic
  esp32/freeRTOS2/uid:ac67b2e9d11c/fw/js/code/set

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
  log("run"); let doIt = function(){mqtt.send("/sensor/uptime",date.now(),0);}; timer.create(2000,'doIt');
  ```

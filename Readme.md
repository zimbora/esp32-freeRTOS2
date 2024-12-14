# freeRTOS2

## IMPORTANT
  !! now() method cannot be called outside main process

## How to use source code
  - Fork the repository
  - Update your esp32 board manager version
  - Install dependent libraries from your Arduino libraries manager  
  - Compile code. Check for correct output
  - Edit FW_MODEL in src/app/user/app_package.h file
  - Edit credentials.h file according to your needs
  - Use app.cpp and app.h to write your own code
  - If new changes are available sync it with main repo
Note: all changes are made outside src/app folder

## package.h

## Connection
  - ENABLE_LTE - Uses BG95 modem
  - ENABLE_BLE - Uses BLE to set wifi credentials
  - ENABLE_AP - Launches Access Point and http server on IP 192.168.4.1 to set wifi credentials otherwise try to connect to default wifi credentials and credentials stored on sysfile at each 30s. 
      Default ssid and password defined on src/app/user/credentials.h file

## src/app/user/app_package.h

### Features
  - ENABLE_JS // JavaScript for user runtime code
  - EXT_SERIAL_COMM // uart2 assigned to Serial1
  - ENABLE_RS485 // enable rs485 module on Serial1
  - DIGITAL_COUNTERS

## src/app/user/credentials.h

### Default credentials
'''
  #define FW_AR_FILENAME "/etc/settings/ar.txt"
  #define FW_ALARM_FILENAME "/etc/settings/alarm.txt"
  #define FW_JS_FILENAME "/etc/settings/js.txt"

  #define APP_PATH_SETTINGS "/home/app/settings"
  #define APP_SETTINGS_FILENAME "/home/app/settings/app.txt"
  #define APP_PATH_RECORDS "/home/app/records"
  #define APP_CALIB_FILENAME "/home/app/calib.txt"
  #define APP_ACCUM_FILENAME "/home/app/accum.txt"
  #define APP_ACCUM_TP_FILENAME "/home/app/accumtp.txt"

  #define WIFI_SSID "inlocAP"
  #define WIFI_PASSWORD "inlocAPpwd"

  #define MQTT_HOST_1 "devices.dev.inloc.cloud" // mac OS broker
  #define MQTT_PORT_1 1883
  #define MQTT_USER_1 "device"
  #define MQTT_PASSWORD_1 "device"

  #define MQTT_HOST_2 "" // mac OS broker
  #define MQTT_PORT_2 1883
  #define MQTT_USER_2 ""
  #define MQTT_PASSWORD_2 ""
  #define MQTT_ACTIVE_2 false

  // CANNOT be changed after being set
  #define MQTT_PROJECT "freeRTOS2"
  #define MQTT_UID_PREFIX "uid:"
  #define MQTT_WILL_SUBTOPIC "/status"
  #define MQTT_WILL_PAYLOAD "offline"
  #define MQTT_PATH_RECORDS "/records"

  #define LOG_ACTIVE true
  #define LOG_LEVEL 5

  #define KEEPALIVE_ACTIVE true
  #define KEEPALIVE_PERIOD 15

  // also used for RS485
  #define UART2_ACTIVE true
  #define UART2_BAUDRATE 115200
  #define UART2_CONFIG SERIAL_8N1
'''

## SYSFILE

  On startup 3 directories are created:
  - /etc/settings
  - /home/app/settings
  - /home/app/records

#### /etc/settings

  Stores system configuration

#### /app/settings

  Can be used by user to store app configurations\
  Use sysfile.write_file() method to store data\
  and sysfile.read_file() to read data

#### /app/records

  Use this directory to store data that should be sent to mqtt broker\
  An autonomous service will be checking for new files and always that is possible it will send to mqtt broker

## Commands

  Commands are passed through MQTT with topic prefix/fw/#\
  The following commands are available

  On receive (topic:payload):
  - :prefix/status (update clock)
  - :prefix/fw/reboot/set : 1 (reboots system)
  - :prefix/fw/reset/set : 1 (reset memory)
  - :prefix/fw/clean/records/set : 1 (clean records in memory)
  - :prefix/fw/fota/update/set : {"url":""} (updates fw)

## Configurations

  Configurations are done through MQTT with topic :prefix/fw/#\
  The following commands are available

  On receive
  - :prefix/fw/settings/modem/set : {"apn":"","pwd":"","user":"","band":(uint8_t),"cops":(uint16_t)} (updates modem settings)
  - :prefix/fw/settings/wifi/set : {"ssid":"","pwd":""} (updates wifi settings)
  - :prefix/fw/settings/mqtt/set : {"host":"","user":"","pass":"","active":(bool)} (configure 2nd mqtt) connection)
  - :prefix/fw/settings/keepalive/set : {"active":(bool),"period":(seconds)}
  - :prefix/fw/settings/log/set : {"active":(bool),"level":(0-disabled;1-verbose;2-debug;3-info;4-warn;5-error)}
  - :prefix/fw/ar/set : {}
  - :prefix/fw/alarm/set : {}
  - :prefix/fw/js/code/set : (file)

## MQTT

  Device can have one or two connections\
  The first connection will be always enabled\
  A second one can be activated and in that case all publish messages in app directory will\
  be redirected for this second connection

### Prefix

  Prefix identifies device in MQTT broker and it is composed by project + uid\
  prefix = MQTT_PROJECT+"/"+uid

  MQTT_PROJECT is defined in credentials file\

  uid = MQTT_UID_PREFIX+macAddress
  MQTT_UID_PREFIX is also defined in credentials file\


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

### Clocksync
  Clock is synched when a ':prefix/status' msg is received

### Available Topics

- [/fw/get] (#fw_)
- [/fw/reboot/set] (#fw_reboot_)
- [/fw/reset/set] (#fw_reset_)
- [/fw/info/get] (#fw_info_)
- [/fw/clean/records/set] (#fw_info_)
- [/fw/fota/update/set] (#fw_fota_update_)
- [/fw/js/code/set] (#fw_js_code_)
- [/fw/wifi/set] (#fw_wifi_)
- [/fw/modem/set] (#fw_modem_)
- [/fw/mqtt/set] (#fw_mqtt_)
- [/fw/log/set] (#fw_log_)
- [/fw/keepalive/set] (#fw_keepalive_)
- [/fw/ar/set] (#fw_ar_)
- [/fw/alarm/set] (#fw_alarm_)

## Autorequests
  Use mqtt topic "#/fw/ar/set" to configure autorequests

  The available sensors depends on your hardware and can be:
  - rs485
  - ...


  The following format is available for rs485:

  ```
  {
    "autorequests":
    {
      "rs485": [
        {
          "ref": "Itemp",
          "modbus": [1,3,834,2],
          "type": "UINT32BE",
          "period":60
        },
        {
          "ref": "Ipress",
          "modbus": [1,3,836,2],
          "type": "UINT16BE",
          "period":60
        },
        {
          "ref": "Ivb",
          "modbus": [1,3,806,4],
          "type": "INT16BE",
          "period":60
        },
        {
          "ref": "Ivc",
          "modbus": [1,3,810,4],
          "type": "FLOATBE",
          "period":60
        }]
    }
  }
  ```

  All autorequests with `app` key will be managed by app module. So, when a sensor with ref `asd` is configured, the program will call app.getValue method to request the value of `asd` sensor.
  It's up to you provide code to return `asd` value

  ```
  {
    "autorequests": {
      "app": [
        {
          "ref": "asd",
          "type": "FLOATBE",
          "period": 60
        }
      ]
    }
  }
  ```

## Alarms
  Use mqtt topic "#/fw/alarm/set" to configure alarms\
  Alarms will use ref field to get configured sensor from autorequests.\
  The following format is available:

  ```
  {
    "alarms": [
      {
        "ref": "Itemp",
        "min_value":15,
        "max_value":30,
        "period":15
      },
      {
        "ref": "Ipress",
        "min_value":1.1,
        "period":15
      },
      {
        "ref": "Ivb",
        "diff":1,
        "max_value":10,
        "period":15
      },
      {
        "ref": "Ivc",
        "period":15
      }
    ]
  }
  ```

  Such as autorequests, alarms with `app` key will also be managed by app module. So, when a sensor with ref `asd` is configured, the program will call app.getValue method to request the value of `asd` sensor.\
  It's up to you provide code to return `asd` value

  ```
  {
    "alarms": [
      {
        "ref": "asd",
        "period": 15,
        "max_value": 150,
        "min_value": 130
      }
    ]
  }
  ```

##JS

[Check this page to further info](https://github.com/zimbora/esp32-freeRTOS2/blob/main/Javascript.md)

# External Dependencies

### External libraries:
- Board esp32 by Espressif Systems 3.0.7
- WiFi.h v1.2.7
- HTTPClient v2.2.0
- EspMQTTClient.h v1.13.3
- TimeLib v1.6.1
- ESP32httpUpdate v2.1.145
- ArduinoJson.h v6.19.4 - deprecated

### Own Libraries:
- autorequest v1.0.1 [autorequest](https://github.com/zimbora/esp32-autorequest)
- alarm v1.0.1[alarm](https://github.com/zimbora/esp32-alarm)
- modbus-rtu v1.0.1 [modbus-rtu](https://github.com/zimbora/esp32-ModbusRTU)
- modem-freeRTOS v1.0.4 [modem-freeRTOS](https://github.com/zimbora/esp32-modem-freeRTOS)
- esp32-BG95 v1.0.5 [esp32-BG95](https://github.com/zimbora/esp32-BG95)
- sysfile v1.0.2 [sysfile](https://github.com/zimbora/esp32-sysfile)
- ESP32Logger2 v1.0.3 [esp32-logger](https://github.com/zimbora/ESP32Logger2)
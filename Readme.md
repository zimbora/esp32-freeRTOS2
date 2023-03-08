# freeRTOS2

## IMPORTANT
  !! now() method cannot be called outside main process

## Compilation

### Before compile

Chose device on ./package.h file
Go to ./src/app folder and inside your project path edit credentials.h file

### Before publish

Increase fw version in ./package.h file
Increase app version credentials.h file inside ./src/app folder

## TODO
  - Add uid to the header of fota request
  - Fota md5 is being well calculated but the comparison is not being well done

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
  An autonomous service will be checking for new files and always that is possible it will send to mqtt broker

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
  Use mqtt topic "#/fw/alarm/set" to configure alarms

  Alarms will use ref field to get configured sensor from autorequests.

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

  Such as autorequests, alarms with `app` key will also be managed by app module. So, when a sensor with ref `asd` is configured, the program will call app.getValue method to request the value of `asd` sensor.
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


#ifndef SENSORS_H
#define SENSORS_H

#include "Arduino.h"

#ifndef UNITTEST

#include <ESP32Logger.h>
#include <ArduinoJson.h>
#include "autorequest.hpp"
#include "alarm.hpp"

#include "hardware.h"
#ifdef ENABLE_RS485
#include "modbus-rtu.h"
#endif

#else

#include <iostream>
#include "../libraries/nlohmann/json.hpp"
#define String std::string
#define byte uint8_t

#include "./utils.h"
#include "./autorequest.h"
#include "./alarm.h"

#endif

#define MAX_IT_SENSORS               10 // do not change it without edit internal_table.h
#define MAX_IOS_SENSORS              10 // do not change it without edit internal_table.h
#define MAX_RS485_SENSORS            10 // do not change it without edit internal_t able.h

// Sensors actions
//#define READ                         0x03
//#define WRITE                        0x06

// BUS ADDRESSES
#define MODEM_BUS                    0x00
#define IT_BUS                       0x01
#define GPIO_BUS                     0x02
#define RS485_BUS                    0x03
#define SPI_BUS                      0x04
#define I2C_BUS                      0x05
#define UART_BUS                     0x06
#define ONEWIRE_BUS                  0x07
#define EEPROM_BUS                   0x08
#define SERVER_BUS                   0x09


// types of variables
#define byte_type                    0  // not known
#define uint8_type                   1
#define int8_type                    2
#define uint16be_type                3
#define uint16le_type                4
#define int16be_type                 5
#define int16le_type                 6
#define uint32be_type                9
#define uint32le_type                10
#define int32be_type                 11
#define int32le_type                 12
#define floatbe_type                 48
#define floatle_type                 49
#define hex_arr                      64
#define char_arr                     65
#define index_itron                  80

#define LTH                                     1
#define HTL                                     0
#define MAX_SIZE_REF                            15
#define MAX_VALUE_LEN                           4


struct IT{
  char ref[MAX_SIZE_REF];
  uint8_t type;
};

struct IO{
  uint8_t gpio;
  uint8_t gpio_type;
  uint32_t value;
  uint32_t last_value;
  uint16_t filter_time;
  uint16_t last_time;
  uint8_t transition;
  uint8_t pull;  // 0 ->no pull, 1 -> pull up, 2 -> pull down
};

struct RS485{
  char ref[MAX_SIZE_REF];
  uint8_t type;
  uint8_t unit_id;
  uint8_t fc;   // read/write
  uint16_t address;
  uint16_t len;  // size of the value to read or to write
  uint8_t value[MAX_VALUE_LEN];  // value to be written or read
  bool error;
};

typedef bool (*Callback)(String ref);

class SensorCallbacks{
  public:
    virtual ~SensorCallbacks(){};
    virtual void onReadSensor(String ref, String value){}; // called after autorequest be executed
    virtual void onAlarmSensor(String ref, String value){}; // called if sensor is in alarm state
    virtual void onAlarmTrigger(String ref, String value){}; // called if sensor has entered or exited from alarm
    virtual void onRS485ReadAll(String data){}; // called after call rs485_read_all function
};

class SENSORS {
  public:

    Callback onReadRS485;
    Callback onAlarmReport;

    SENSORS();

    void init();
    bool init_ar(String data);
    bool init_alarm(String filename);

    void loop();

    void alarmTrigger(String ref, String value);
    #ifdef ENABLE_RS485
    void rs485_init(HardwareSerial* port, uint8_t rx, uint8_t tx, uint8_t rts);
    void rs485_set_config(uint8_t mode=1, uint32_t baudrate=9600, uint32_t config=SERIAL_8N1, uint8_t retries=3);
    void rs485_change_config(uint8_t mode=1, uint32_t baudrate=9600, uint32_t config=SERIAL_8N1, uint8_t retries=3);
    bool rs485_read_all();
    uint8_t rs485_read(String ref);
    uint8_t rs485_read(uint8_t index);
    uint8_t rs485_read(uint8_t unit_id, uint8_t fc, uint16_t address, uint16_t len, uint8_t* data, uint16_t* size);
    uint8_t rs485_write(uint8_t unit_id, uint8_t fc, uint16_t address, uint16_t len, uint8_t* data, uint16_t* size);
    bool rs485_add(uint8_t index, String ref_str, String modbus, String type_str);
    bool rs485_add(uint8_t index, String ref_str, uint8_t type, uint8_t unit_id, uint8_t fc, uint16_t address, uint16_t len, uint8_t* value);
    #endif
    int get_type(String type);

    // callbacks
    void setCallbacks(SensorCallbacks* pClass){
      pSensorCallbacks = pClass;
    };
    //bool calledInRS485Autorequest(String ref);

    bool parseArray(String array, uint16_t* arr, int16_t* len);
    bool has_only_digits(String value_str);
  private:

    void rs485_table_refresh(uint8_t index);
    void rs485_log(uint8_t index);
    float truncate(float val, byte dec);


    SensorCallbacks* pSensorCallbacks = nullptr;
};

extern DynamicJsonDocument doc;
extern SENSORS sensors;

#ifdef UNITTEST
extern nlohmann::json table;
extern IT it_map[MAX_IT_SENSORS];
extern IO io_map[MAX_IOS_SENSORS];
extern RS485 rs485_map[MAX_RS485_SENSORS];
#endif

#endif

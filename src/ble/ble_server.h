
#ifndef BLE_SERVER_H
#define BLE_SERVER_H

#include "Arduino.h"

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define FW_S_UUID           "3637a2b8-7bbd-11ed-a1eb-0242ac120002"
#define FW_REBOOT_C_UUID         "752eb026-7bc0-11ed-a1eb-0242ac120002"
#define FW_RESET_C_UUID          "752eb3f0-7bc0-11ed-a1eb-0242ac120002"

#define WIFI_S_UUID         "3637cde2-7bbd-11ed-a1eb-0242ac120002"
#define WIFI_MODE_C_UUID         "e73c91c8-7bbc-11ed-a1eb-0242ac120002"
#define WIFI_SSID_C_UUID         "e73c956a-7bbc-11ed-a1eb-0242ac120002"
#define WIFI_PWD_C_UUID          "e73c9740-7bbc-11ed-a1eb-0242ac120002"

#define MODEM_S_UUID        "3637d1ac-7bbd-11ed-a1eb-0242ac120002"
#define MODEM_APN_C_UUID          "fc7cf7e4-7bbc-11ed-a1eb-0242ac120002"
#define MODEM_USER_C_UUID         "fc7cfcee-7bbc-11ed-a1eb-0242ac120002"
#define MODEM_PWD_C_UUID          "fc7d0036-7bbc-11ed-a1eb-0242ac120002"
#define MODEM_BAND_C_UUID         "fc7d01d0-7bbc-11ed-a1eb-0242ac120002"
#define MODEM_COPS_C_UUID         "fc7d0806-7bbc-11ed-a1eb-0242ac120002"

#define MQTT_S_UUID         "3637d378-7bbd-11ed-a1eb-0242ac120002"
#define MQTT_HOST_C_UUID          "11e2d70c-7bbd-11ed-a1eb-0242ac120002"
#define MQTT_USER_C_UUID          "11e2d982-7bbd-11ed-a1eb-0242ac120002"
#define MQTT_PASS_C_UUID          "11e2dc52-7bbd-11ed-a1eb-0242ac120002"
#define MQTT_PREFIX_C_UUID        "11e2dec8-7bbd-11ed-a1eb-0242ac120002"
#define MQTT_PORT_C_UUID          "11e2e166-7bbd-11ed-a1eb-0242ac120002"
#define MQTT_ACTIVE_C_UUID        "11e2e2ce-7bbd-11ed-a1eb-0242ac120002"

#define LOG_S_UUID          "3637d65c-7bbd-11ed-a1eb-0242ac120002"
#define LOG_ACTIVE_C_UUID         "259b60ac-7bbd-11ed-a1eb-0242ac120002"
#define LOG_LEVEL_C_UUID          "259b6390-7bbd-11ed-a1eb-0242ac120002"

#define KA_S_UUID           "3637d7c4-7bbd-11ed-a1eb-0242ac120002"
#define KA_ACTIVE_C_UUID          "2c2032b8-7bbd-11ed-a1eb-0242ac120002"
#define KA_PERIOD_C_UUID          "2c203a88-7bbd-11ed-a1eb-0242ac120002"


class BLE_SERVER {

  public:
    BLE_SERVER(){};
    void init(String uid);

    //std::function<void(String param, String text)> callbackBLE;
    void setCallback(void (*fp)(String, String));

    void enable();
    void disable();


  private:

};

class MyCallbacks:public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic);
};

#endif

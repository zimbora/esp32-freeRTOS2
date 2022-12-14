
#ifndef BLE_SERVER_H
#define BLE_SERVER_H

#include "Arduino.h"

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>




class BLE_SERVER {

  public:
    BLE_SERVER(){};
    void init();

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

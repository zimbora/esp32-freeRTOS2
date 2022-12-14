/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

#include "ble_server.h"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define WIFI_SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define SSID_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define PWD_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a9"

void (*callbackBLE)(String param, String text);
void BLE_SERVER::setCallback(void (*fp)(String, String)){
  callbackBLE = fp;
}


void MyCallbacks::onWrite(BLECharacteristic *pCharacteristic) {

  BLEUUID uid = pCharacteristic->getUUID();
  std::string uid_str = uid.toString();

  std::string rxValue = pCharacteristic->getValue();

  if (rxValue.length() > 0) {
    String a = String(uid_str.c_str());
    String b = String(rxValue.c_str());

    if(callbackBLE != NULL)
      callbackBLE(a,b);
  }
}

void BLE_SERVER::init() {

  BLEDevice::init("ESP32 - device");
  BLEServer *pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(WIFI_SERVICE_UUID);
  BLECharacteristic *pCharacteristic1 = pService->createCharacteristic(
                                         SSID_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic1->setValue("current ssid");


  BLECharacteristic *pCharacteristic2 = pService->createCharacteristic(
                                         PWD_CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pService->start();

  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(WIFI_SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);

  //BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");

  pCharacteristic1->setCallbacks(new MyCallbacks());
  pCharacteristic2->setCallbacks(new MyCallbacks());

}

void BLE_SERVER::enable(){

  BLEDevice::startAdvertising();
  Serial.println("[BLE] Advertising..");
}

void BLE_SERVER::disable(){

  BLEDevice::deinit();
  Serial.println("[BLE] Switched off..");
}

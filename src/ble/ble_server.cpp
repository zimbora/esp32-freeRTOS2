/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

#include "ble_server.h"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

void (*callbackBLE)(String param, String text);
void BLE_SERVER::setCallback(void (*fp)(String, String)){
  callbackBLE = fp;
}


void MyCallbacks::onWrite(BLECharacteristic *pCharacteristic) {

  BLEUUID uid = pCharacteristic->getUUID();
  String uid_str = uid.toString();

  String rxValue = pCharacteristic->getValue();

  if (rxValue.length() > 0) {
    String a = String(uid_str.c_str());
    String b = String(rxValue.c_str());

    if(callbackBLE != NULL)
      callbackBLE(a,b);
  }
}

void BLE_SERVER::init(String uid) {

  String uid_str(uid.c_str(), uid.length());
  //std::string name = "ESP32-"+uid_str;
  BLEDevice::init(uid_str);
  BLEServer *pServer = BLEDevice::createServer();

  // FW COMMANDS
  BLEService *pServiceFW = pServer->createService(FW_S_UUID);
  BLECharacteristic *pCharacteristic1FW = pServiceFW->createCharacteristic(
                                         FW_REBOOT_C_UUID,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  //pCharacteristic1FW->setValue(settings.wifi.ssid);
  BLECharacteristic *pCharacteristic2FW = pServiceFW->createCharacteristic(
                                         FW_RESET_C_UUID,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pServiceFW->start();

  // WIFI CONFIGURATION
  BLEService *pServiceWiFi = pServer->createService(WIFI_S_UUID);
  BLECharacteristic *pCharacteristic1WiFi = pServiceWiFi->createCharacteristic(
                                         WIFI_SSID_C_UUID,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  //pCharacteristic1WiFi->setValue(settings.wifi.ssid);
  BLECharacteristic *pCharacteristic2WiFi = pServiceWiFi->createCharacteristic(
                                         WIFI_PWD_C_UUID,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pServiceWiFi->start();


  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(WIFI_S_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);

  //BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");

  pCharacteristic1WiFi->setCallbacks(new MyCallbacks());
  pCharacteristic2WiFi->setCallbacks(new MyCallbacks());

  pCharacteristic1FW->setCallbacks(new MyCallbacks());
  pCharacteristic2FW->setCallbacks(new MyCallbacks());

}

void BLE_SERVER::enable(){

  BLEDevice::startAdvertising();
  Serial.println("[BLE] Advertising..");
}

void BLE_SERVER::disable(){

  BLEDevice::deinit();
  Serial.println("[BLE] Switched off..");
}

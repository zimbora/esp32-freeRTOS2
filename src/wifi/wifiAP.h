
#ifndef WIFIAP_H
#define WIFIAP_H


#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include "../../src/app/user/credentials.h"

class WiFiCallbacks{
  public:
    virtual ~WiFiCallbacks(){};
    virtual void onWiFiSet(String ssid, String pwd){}; // called after autorequest be executed
};

class WIFIAP{
public:
  void setup(String mac);
  void stop();
  void loop();
  void setCallbacks(WiFiCallbacks* callbacks){
    pWiFiCallbacks = callbacks;
  }
private:

  // Set these to your desired credentials.
  const char *ssid = "ESP32AP";
  const char *password = AP_PASSWORD;

  WiFiCallbacks* pWiFiCallbacks = nullptr;
};



#endif

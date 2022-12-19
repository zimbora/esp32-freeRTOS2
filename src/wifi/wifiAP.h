
#ifndef WIFIAP_H
#define WIFIAP_H


#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

#include "sysfile.hpp"
#include "../settings/settings.h"
#include "../../credentials.h"
#include "../calls/calls.h"

class WIFIAP{
public:
  void setup();
  void loop();
private:

  // Set these to your desired credentials.
  const char *ssid = "ESP32AP";
  const char *password = "testingnewap";

};



#endif

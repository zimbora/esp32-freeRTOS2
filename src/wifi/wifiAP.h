
#ifndef WIFIAP_H
#define WIFIAP_H

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

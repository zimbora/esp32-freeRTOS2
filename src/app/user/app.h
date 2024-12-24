#ifndef APP_H
#define APP_H

#include "Arduino.h"
#include <map>

#include "../../../package.h"
#include "sysfile.hpp"
#include "app_package.h"
#include "credentials.h"
#include "./../../../core.h"


struct app_settings {

  struct fw {
    char         version[8];
    char         md5[16];
  }fw;

};

// user can edit it
enum appTopics_ {
  settings_reset_set_,
  app_not_found
};


// user can edit it
static const std::map<long, appTopics_> appTopics {
  { (long)std::hash<std::string>{}("/app/settings/reset/set"),                  settings_reset_set_ },
};

class APP{

  public:
    APP(){};

    /* do not delete it the following functions */
    void init();
    void loop();
    void parse_mqtt_messages(uint8_t clientID, String topic, String payload);
    bool getValue(JsonObject& obj, String ref){return false;};

  private:
    // do not delete the following function
    appTopics_ resolveOption(std::map<long, appTopics_> map, String topic);
    bool load_settings();
    bool store_settings();
    bool reset_settings();
    void log_settings();

    uint32_t timeoutInfo;
};

#endif

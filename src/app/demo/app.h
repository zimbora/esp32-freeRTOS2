#ifndef APP_H
#define APP_H

#include "Arduino.h"
#include <map>

#include "sysfile.hpp"
#include "app_package.h"
#include "credentials.h"
#include "./../../../core.h"


struct app_settings {

  struct fw {
    char         version[8];
    char         md5[16];
  }fw;

	struct rs485 { // on settings load
		bool          active;
		uint32_t      baudrate;
		uint32_t       config;
	} rs485;

};

// user can edit it
enum appTopics_ {
  settings_reset_set_,
  settings_rs485_set_,
  settings_rs485_get_,
  rs485_read_get_,
  rs485_write_get_,
  app_not_found
};


// user can edit it
static const std::map<long, appTopics_> appTopics {
  { (long)std::hash<std::string>{}("/app/settings/reset/set"),                  settings_reset_set_ },
  { (long)std::hash<std::string>{}("/app/settings/rs485/set"),                  settings_rs485_set_ },
  { (long)std::hash<std::string>{}("/app/settings/rs485/get"),                  settings_rs485_get_ },
  { (long)std::hash<std::string>{}("/app/rs485/read/get"),                      rs485_read_get_ },
  { (long)std::hash<std::string>{}("/app/rs485/write/get"),                     rs485_write_get_ }
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
};

#endif

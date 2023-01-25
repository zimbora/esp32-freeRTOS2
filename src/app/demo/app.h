#ifndef APP_H
#define APP_H

#include "Arduino.h"
#include <map>

#include "sysfile.hpp"
#include "app_package.h"
#include "credentials.h"
#include "./../../../core.h"

// user can edit it
enum appTopics_ {
  rs485_read_,
  rs485_write_,
  app_not_found
};

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
static const std::map<long, appTopics_> appTopics {
  { (long)std::hash<std::string>{}("/app/rs485/read/get"),                      rs485_read_ },
  { (long)std::hash<std::string>{}("/app/rs485/write/get"),                     rs485_write_ }
};

class APP{

  public:
    APP(){};

    /* do not delete it the following 3 functions */
    void init();
    void loop();
    void parse_mqtt_messages(uint8_t clientID, String topic, String payload);

  private:
    // do not delete the following function
    appTopics_ resolveOption(std::map<long, appTopics_> map, String topic);

    bool load_settings();
};

#endif

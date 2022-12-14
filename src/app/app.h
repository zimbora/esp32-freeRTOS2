#ifndef APP_H
#define APP_H

#include "Arduino.h"
#include <map>

#include "../../core.h"

// user can edit it
enum appTopics_ {
  rs485_read_,
  rs485_write_,
  app_not_found
};

// user can edit it
static const std::map<long, appTopics_> appTopics {
  { (long)std::hash<std::string>{}("/rs485/read/get"),                      rs485_read_ },
  { (long)std::hash<std::string>{}("/rs485/write/get"),                     rs485_write_ }
};

class APP{

  public:
    APP(){};

    void init();
    void loop();

    // do not delete it
    void parse_mqtt_messages(uint8_t clientID, String topic, String payload);

  private:
    appTopics_ resolveOption(std::map<long, appTopics_> map, String topic);
};

#endif

#ifndef CORE_H
#define CORE_H

#include "Arduino.h"
#include <WiFi.h>
#include <ArduinoJson.h>
#include <ESP32Logger.h>
#include "esp_log.h"
#include "esp_task_wdt.h"
#include <map>

#include <Time.h>
#include <TimeLib.h>

#include "./src/calls/calls.h"
#include "credentials.h"
#include "sysfile.hpp"
#include "./src/app/app.h"
#include "./src/settings/settings.h"
#include "./src/settings/package.h"

#ifdef ENABLE_BLE
#include "./src/ble/ble_server.h"
#endif


enum fwTopics_ {
  fw_,
  fw_reboot_,
  fw_reset_,
  fw_info_,
  fw_clean_records_,
  fw_fota_update_,
  fw_settings_update_,
  fw_wifi_,
  fw_modem_,
  fw_mqtt_,
  fw_log_,
  fw_keepalive_,
  fw_not_found
};

static const std::map<long, fwTopics_> fwTopics {
  { (long)std::hash<std::string>{}("/fw/get"),                              fw_ },
  { (long)std::hash<std::string>{}("/fw/reboot/set"),                       fw_reboot_ },
  { (long)std::hash<std::string>{}("/fw/reset/set"),                        fw_reset_ },
  { (long)std::hash<std::string>{}("/fw/info/get"),                         fw_info_ },
  { (long)std::hash<std::string>{}("/fw/clean/records/set"),                fw_info_ },
  { (long)std::hash<std::string>{}("/fw/fota/update/set"),                  fw_fota_update_ },
  { (long)std::hash<std::string>{}("/fw/wifi/set"),                         fw_wifi_ },
  { (long)std::hash<std::string>{}("/fw/modem/set"),                        fw_modem_ },
  { (long)std::hash<std::string>{}("/fw/mqtt/set"),                         fw_mqtt_ },
  { (long)std::hash<std::string>{}("/fw/log/set"),                          fw_log_ },
  { (long)std::hash<std::string>{}("/fw/keepalive/set"),                    fw_keepalive_ }
};


void      core_init();
void      core_loop();

void      core_parse_mqtt_messages();
bool      core_send_mqtt_message(uint8_t clientID, String topic, String data, uint8_t qos, bool retain);

// private
void      core_check_records();
bool      core_send_record(String filename);
bool      core_store_record(String filename, const char* data, uint16_t len);

uint8_t   parse_float_array(float* arr, uint8_t len, String payload);
fwTopics_ resolveOption(std::map<long, fwTopics_> map, String topic);
String    get_uid();

String    date();
String    pad2(int value);

// Callback methods prototypes
void      t1Callback();

#ifdef ENABLE_BLE
void       bleCallback(String param, String value);
#endif



#endif

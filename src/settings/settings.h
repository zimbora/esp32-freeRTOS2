
#ifndef SETTINGS_H
#define SETTINGS_H

#include "Arduino.h"

#include <ArduinoJson.h>
#include "mbedtls/md.h"
#include <map>

#include "package.h"
#include <ESP32Logger.h>

enum settingsTopics_ {
  setting_not_found,
  wifi_mode,
  wifi_ssid,
  wifi_pwd,
  modem_apn,
  modem_user,
  modem_pwd,
  modem_band,
  modem_cops,
  mqtt_host,
  mqtt_user,
  mqtt_pass,
  mqtt_prefix,
  mqtt_port,
  mqtt_active,
  log_active,
  log_level,
  keepalive_active,
  keepalive_period
};

static const std::map<long, settingsTopics_> settingsTopics {
  { (long)std::hash<std::string>{}("wifi_mode"),                      wifi_mode },
  { (long)std::hash<std::string>{}("wifi_ssid"),                      wifi_ssid },
  { (long)std::hash<std::string>{}("wifi_pwd"),                       wifi_pwd },
  { (long)std::hash<std::string>{}("modem_apn"),                      modem_apn },
  { (long)std::hash<std::string>{}("modem_user"),                     modem_user },
  { (long)std::hash<std::string>{}("modem_pwd"),                      modem_pwd },
  { (long)std::hash<std::string>{}("modem_band"),                     modem_band },
  { (long)std::hash<std::string>{}("modem_cops"),                     modem_cops },
  { (long)std::hash<std::string>{}("mqtt_host"),                      mqtt_host },
  { (long)std::hash<std::string>{}("mqtt_user"),                      mqtt_user },
  { (long)std::hash<std::string>{}("mqtt_pass"),                      mqtt_pass },
  { (long)std::hash<std::string>{}("mqtt_prefix"),                    mqtt_prefix },
  { (long)std::hash<std::string>{}("mqtt_port"),                      mqtt_port },
  { (long)std::hash<std::string>{}("mqtt_active"),                    mqtt_active },
  { (long)std::hash<std::string>{}("log_active"),                     log_active },
  { (long)std::hash<std::string>{}("log_level"),                      log_level },
  { (long)std::hash<std::string>{}("keepalive_active"),               keepalive_active },
  { (long)std::hash<std::string>{}("keepalive_period"),               keepalive_period }
};


struct user_settings {

  struct fw {
    char         version[8];
    char         md5[16];
  }fw;

  struct wifi { // on settings load
    char         mode[12];
		char         ssid[32];
		char         pwd[32];
	} wifi;

  struct modem { // on settings load
		char         apn[32];
		char         user[32];
		char         pwd[32];
		uint8_t      band;
		uint16_t     cops;
	} modem;

  struct mqtt { // on settings load
    char        host[64];
    char        user[32];
    char        pass[32];
    char        prefix[32];
    uint16_t    port; //1883
    bool        active; //false
  } mqtt;

	struct log { // on settings load
		bool         active;
		uint8_t      level;
	} log;

  struct keepalive {  // on settings load
    bool         active;
		uint32_t     period;
	} keepalive;

};

extern user_settings settings;

bool settings_set_param(String param, String value);
settingsTopics_ resolveOptionSettings(std::map<long, settingsTopics_> map, String param);
void settings_log();

bool has_only_digits(String value_str);
long stoLong(String s);


/*
#ifndef UNITTEST
  extern global_settings settings;
  extern DynamicJsonDocument doc;
#else
  extern nlohmann::json doc;
#endif


#ifndef UNITTEST
  bool settings_clean();

  void jsonListKeys(JsonObject documentRoot, String topic, JsonVariant payload);

  bool settings_update(char* data, int16_t len);

  uint32_t align_time_with(uint32_t period);
#endif
*/
#endif

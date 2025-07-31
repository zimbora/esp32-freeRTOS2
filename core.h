#ifndef CORE_H
#define CORE_H

#include "Arduino.h"

#include "./package.h"

#include <WiFi.h>
#include <ArduinoJson.h>
#include <ESP32Logger.h>
#include "esp_log.h"
#include "esp_task_wdt.h"
#include <map>
#include <TimeLib.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>

#include "./src/calls/calls.h"
#include "./src/sensors/sensors.h"
#include "sysfile.hpp"
#include "./src/settings/settings.h"
#include "./src/wifi/wifiAP.h"

#include "./src/app/user/app.h"

#ifdef ENABLE_JS
#include "JS.h"
#endif

#ifdef ENABLE_BLE
#include "./src/ble/ble_server.h"
#endif

extern CALLS call;
extern SemaphoreHandle_t spiffsMutex;
extern SYSFILE sysfile;

enum fwTopics_ {
  fw_get_,
  fw_reboot_,
  fw_reset_,
  fw_clean_records_,
  fw_fota_update_,
  settings_,
  settings_get_,
  settings_update_,
  settings_wifi_,
  settings_wifi_get_,
  settings_modem_,
  settings_modem_get_,
  settings_mqtt_,
  settings_mqtt_get_,
  settings_log_,
  settings_log_get_,
  settings_keepalive_,
  settings_keepalive_get_,
  settings_serial_,
  settings_serial_get_,
  fw_js_program_,
  fw_js_program_get_,
  fw_ar_,
  fw_ar_get_,
  fw_alarm_,
  fw_alarm_get_,
  fw_serial_read_get_,
  fw_serial_write_get_,
  fw_not_found
};

static const std::map<long, fwTopics_> fwTopics {
  { (long)std::hash<std::string>{}("/fw/get"),                              fw_get_ },
  { (long)std::hash<std::string>{}("/fw/reboot/set"),                       fw_reboot_ },
  { (long)std::hash<std::string>{}("/fw/reset/set"),                        fw_reset_ },
  { (long)std::hash<std::string>{}("/fw/clean/records/set"),                fw_clean_records_ },
  { (long)std::hash<std::string>{}("/fw/fota/update/set"),                  fw_fota_update_ },
  { (long)std::hash<std::string>{}("/settings/set"),                        settings_ },
  { (long)std::hash<std::string>{}("/settings/get"),                        settings_get_ },
  { (long)std::hash<std::string>{}("/settings/update"),                     settings_update_ },
  { (long)std::hash<std::string>{}("/settings/wifi/set"),                   settings_wifi_ },
  { (long)std::hash<std::string>{}("/settings/wifi/get"),                   settings_wifi_get_ },
  { (long)std::hash<std::string>{}("/settings/modem/set"),                  settings_modem_ },
  { (long)std::hash<std::string>{}("/settings/modem/get"),                  settings_modem_get_ },
  { (long)std::hash<std::string>{}("/settings/mqtt/set"),                   settings_mqtt_ },
  { (long)std::hash<std::string>{}("/settings/mqtt/get"),                   settings_mqtt_get_ },
  { (long)std::hash<std::string>{}("/settings/log/set"),                    settings_log_ },
  { (long)std::hash<std::string>{}("/settings/log/get"),                    settings_log_get_ },
  { (long)std::hash<std::string>{}("/settings/keepalive/set"),              settings_keepalive_ },
  { (long)std::hash<std::string>{}("/settings/keepalive/get"),              settings_keepalive_get_ },
  { (long)std::hash<std::string>{}("/settings/serial/set"),                 settings_serial_ },
  { (long)std::hash<std::string>{}("/settings/serial/get"),                 settings_serial_get_ },
  { (long)std::hash<std::string>{}("/fw/ar/set"),                           fw_ar_ },
  { (long)std::hash<std::string>{}("/fw/ar/get"),                           fw_ar_get_ },
  { (long)std::hash<std::string>{}("/fw/alarms/set"),                       fw_alarm_ },
  { (long)std::hash<std::string>{}("/fw/alarms/get"),                       fw_alarm_get_ },
  { (long)std::hash<std::string>{}("/fw/js_program/set"),                   fw_js_program_ },
  { (long)std::hash<std::string>{}("/fw/js_program/get"),                   fw_js_program_get_ },
  { (long)std::hash<std::string>{}("/fw/serial/read/get"),                  fw_serial_read_get_ },
  { (long)std::hash<std::string>{}("/fw/serial/write/get"),                 fw_serial_write_get_ }
};

#ifdef ENABLE_BLE
enum uuidTopics_ {
  uuid_not_found,
  uuid_fw_reboot,
  uuid_fw_reset,
  uuid_wifi_mode,
  uuid_wifi_ssid,
  uuid_wifi_pwd,
  uuid_modem_apn,
  uuid_modem_user,
  uuid_modem_pwd,
  uuid_modem_band,
  uuid_modem_cops,
  uuid_mqtt_host,
  uuid_mqtt_user,
  uuid_mqtt_pass,
  uuid_mqtt_prefix,
  uuid_mqtt_port,
  uuid_mqtt_active,
  uuid_log_active,
  uuid_log_level,
  uuid_keepalive_active,
  uuid_keepalive_period
};

static const std::map<long, uuidTopics_> uuidTopics {
  { (long)std::hash<std::string>{}(FW_REBOOT_C_UUID),                      uuid_fw_reboot },
  { (long)std::hash<std::string>{}(FW_RESET_C_UUID),                       uuid_fw_reset },
  { (long)std::hash<std::string>{}(WIFI_MODE_C_UUID),                      uuid_wifi_mode },
  { (long)std::hash<std::string>{}(WIFI_SSID_C_UUID),                      uuid_wifi_ssid },
  { (long)std::hash<std::string>{}(WIFI_PWD_C_UUID),                       uuid_wifi_pwd },
  { (long)std::hash<std::string>{}(MODEM_APN_C_UUID),                      uuid_modem_apn },
  { (long)std::hash<std::string>{}(MODEM_USER_C_UUID),                     uuid_modem_user },
  { (long)std::hash<std::string>{}(MODEM_PWD_C_UUID),                      uuid_modem_pwd },
  { (long)std::hash<std::string>{}(MODEM_BAND_C_UUID),                     uuid_modem_band },
  { (long)std::hash<std::string>{}(MODEM_COPS_C_UUID),                     uuid_modem_cops },
  { (long)std::hash<std::string>{}(MQTT_HOST_C_UUID),                      uuid_mqtt_host },
  { (long)std::hash<std::string>{}(MQTT_USER_C_UUID),                      uuid_mqtt_user },
  { (long)std::hash<std::string>{}(MQTT_PASS_C_UUID),                      uuid_mqtt_pass },
  { (long)std::hash<std::string>{}(MQTT_PREFIX_C_UUID),                    uuid_mqtt_prefix },
  { (long)std::hash<std::string>{}(MQTT_PORT_C_UUID),                      uuid_mqtt_port },
  { (long)std::hash<std::string>{}(MQTT_ACTIVE_C_UUID),                    uuid_mqtt_active },
  { (long)std::hash<std::string>{}(LOG_ACTIVE_C_UUID),                     uuid_log_active },
  { (long)std::hash<std::string>{}(LOG_LEVEL_C_UUID),                      uuid_log_level },
  { (long)std::hash<std::string>{}(KA_ACTIVE_C_UUID),                      uuid_keepalive_active },
  { (long)std::hash<std::string>{}(KA_PERIOD_C_UUID),                      uuid_keepalive_period }
};
#endif

#ifdef ENABLE_AP
class CALLBACKS_WIFI_AP : public WiFiCallbacks
{
  public:
    void onWiFiSet(String ssid, String pass);
};
#endif


class CALLBACKS_SENSORS : public SensorCallbacks
{
  public:
    void onReadSensor(String ref, String value);
    void onAlarmSensor(String ref, String value);
    void onAlarmTrigger(String ref, String value);
    void onRS485ReadAll(String data);
    bool getAppValue(JsonObject& obj, String ref); // called if a sensor value from app class is required
};

void      core_init();
void      core_loop();
void      core_load_settings();

void      core_parse_mqtt_messages();
bool      core_send_mqtt_message(uint8_t clientID, String topic, String data, uint8_t qos, bool retain);

// private
void      core_check_records();
bool      core_send_record(String filename);
bool      core_store_record(String filename, const char* data, uint16_t len);

String    core_fota(String url);

uint8_t   parse_float_array(float* arr, uint8_t len, String payload);
fwTopics_ resolveOption(std::map<long, fwTopics_> map, String topic);
#ifdef ENABLE_BLE
uuidTopics_ resolveOptionUUID(std::map<long, uuidTopics_> map, String param);
#endif

String    get_uid();

String    date();
String    pad2(int value);

// Callback methods prototypes
void      t1Callback();

#ifdef ENABLE_BLE
void       bleCallback(String param, String value);
#endif



#endif

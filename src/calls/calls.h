#ifndef CALLS_H
#define CALLS_H

#include <ESP32Logger.h>
#include <MD5Builder.h>

#include "modem-freeRTOS.hpp"
#include "sysfile.hpp"
#include "../fota/fota.h"

#define NUMITEMS(arg) ((unsigned int) (sizeof (arg) / sizeof (arg [0])))

#define MAX_RECORD_FILE_SIZE 512

// CONTEXT
#define CLIENTID 0
#define CLIENTIDEXTERNAL 1
#define SSLCLIENTID 0
#define CONTEXTID 1

class CALLS{

public:
  CALLS(){};

  String fw_fota(String url);
  bool fw_settings_update(String url, String filename);
  bool fw_reboot();
  bool fw_reset();

  bool init_filesystem(String directory[],uint8_t len);
  bool check_filesystem_records(String dir, uint32_t timeout, bool (*callback)(String));
  bool create_dir(String directory);
  bool store_record(String filename, const char* data, uint16_t len);
  bool remove_dir(String directory);
  void clean_dir(String directory);
  String get_file_md5(String filename);
  bool read_file(String filename, char* data, uint16_t* len);
  bool write_file(String filename, const char* data, uint16_t len);
  bool delete_file(String filename);

  bool mqtt_send(String topic, String data, uint8_t qos = 0, bool retain = 0);
  bool mqtt_send(uint8_t clientID, String topic, String data, uint8_t qos = 0, bool retain = 0);

  void check_alarms();
  void check_sensors();


private:

  String do_request(String protocol, String host, String path, String method, String header_key, String header_value, String body, bool json);
  String do_fota(String protocol, String host, String path, String method, String header_key, String header_value, String body, bool json);

};

#endif

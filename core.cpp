
#include "core.h"

SemaphoreHandle_t spiffsMutex;
CALLS call;
SYSFILE sysfile;
extern SENSORS sensors;
APP app;
Core core;

MODEMfreeRTOS mRTOS; // freeRTOS modem
MQTT_MSG_RX* msg; // mqtt
extern DynamicJsonDocument doc; // json

#ifdef ENABLE_AP
  void CALLBACKS_WIFI_AP::onWiFiSet(String ssid, String pass){
    settings_set_param("wifi_ssid",ssid);
    settings_set_param("wifi_pwd",pass);

    settings_log();
    if(sysfile.write_file(FW_SETTINGS_FILENAME,settings.fw.version,sizeof(settings))){
      LOG_INFO("Client Disconnected.\n");
      call.fw_reboot();
    }else{
      LOG_ERROR("failing writing file: %s\n", FW_SETTINGS_FILENAME);
    }
  }
#endif

#ifdef ENABLE_BLE
  void bleCallback(String uuid, String value){

    String param = "";

    switch(resolveOptionUUID(uuidTopics,uuid)){

      case uuid_fw_reboot:
        if(value == "1")
          call.fw_reboot();
        break;

      case uuid_fw_reset:
        if(value == "1"){
          call.fw_reset();
          call.fw_reboot();
        }
        break;

      case uuid_wifi_mode:
        param = "wifi_mode";
        break;

      case uuid_wifi_ssid:
        param = "wifi_ssid";
        break;

      case uuid_wifi_pwd:
        param = "wifi_pwd";
        break;

      case uuid_modem_apn:
        param = "modem_apn";
        break;

      case uuid_modem_user:
        param = "modem_user";
        break;

      case uuid_modem_pwd:
        param = "modem_pwd";
        break;

      case uuid_modem_band:
        param = "modem_band";
        break;

      case uuid_modem_cops:
        param = "modem_cops";
        break;

      case uuid_mqtt_host:
        param = "mqtt_host";
        break;

      case uuid_mqtt_user:
        param = "mqtt_user";
        break;

      case uuid_mqtt_pass:
        param = "mqtt_pass";
        break;

      case uuid_mqtt_prefix:
        param = "mqtt_prefix";
        break;

      case uuid_mqtt_port:
        param = "modem_port";
        break;

      case uuid_mqtt_active:
        param = "modem_active";
        break;

      case uuid_log_active:
        param = "log_active";
        break;

      case uuid_log_level:
        param = "log_level";
        break;

      case uuid_keepalive_active:
        param = "keepalive_active";
        break;

      case uuid_keepalive_period:
        param = "keepalive_period";
        break;
    }

    DBLOG(Debug,"param:"+param);
    DBLOG(Debug,"value:"+value);
    if(settings_set_param(param,value)){
      settings_log();
      if(!sysfile.write_file(FW_SETTINGS_FILENAME,settings.fw.version,sizeof(settings)))
        LOG_ERROR("failing writing file: %s\n", FW_SETTINGS_FILENAME);
    }
  }
#endif

String directory[] = {
  SYS_PATH_SETTINGS,
  APP_PATH_SETTINGS,
  APP_PATH_RECORDS
};

void CALLBACKS_SENSORS::onReadSensor(String ref, String value){
  LOG_DEBUG("onReadSensor callback called\n");
  #ifdef ENABLE_JS
  String code = "event.onReadSensor(\""+ref+"\","+value+")";
  const char* res = JS.call(code.c_str());     // Execute JS code
  LOG_DEBUG("%s\n", res);
  #else
  call.mqtt_send("/"+ref,value,2,0);
  #endif
};

void CALLBACKS_SENSORS::onAlarmSensor(String ref, String value){
  LOG_DEBUG("onAlarmSensor callback called\n");
  #ifdef ENABLE_JS
  String code = "event.onAlarmSensor(\""+ref+"\","+value+")";
  const char* res = JS.call(code.c_str());     // Execute JS code
  LOG_DEBUG("%s\n", res);
  #endif
};

void CALLBACKS_SENSORS::onAlarmTrigger(String ref, String value){
  LOG_DEBUG("onAlarmTrigger callback called\n");
  #ifdef ENABLE_JS
  String code = "event.onAlarmTrigger(\""+ref+"\","+value+")";
  const char* res = JS.call(code.c_str());     // Execute JS code
  LOG_DEBUG("%s\n", res);
  #else
  call.mqtt_send("/"+ref,value,2,0);
  #endif
};

void CALLBACKS_SENSORS::onRS485ReadAll(String data_json){
  LOG_DEBUG("onRS485ReadAll callback called\n");
  LOG_DEBUG("core: %s\n", data_json.c_str());
  String filename = ".txt";
  core.store_record(filename,data_json.c_str(),data_json.length());
};


bool CALLBACKS_SENSORS::getAppValue(JsonObject& obj, String ref){

  return app.getValue(obj,ref);
};


/*
* load settings
*/
void Core::load_settings(){

  uint16_t len = sizeof(settings);
  char* data = (char*)malloc(len);
  if(data != nullptr){
    memset(data,0,len);
    call.read_file(FW_SETTINGS_FILENAME,data,&len);
    memcpy(settings.fw.version,data,sizeof(settings.fw.version));
    LOG_INFO("current fw version: %s\n", FW_VERSION);
    LOG_INFO("previous fw version: %s\n", settings.fw.version);
    if(memcmp(FW_VERSION,settings.fw.version,sizeof(FW_VERSION)) != 0){
      LOG_INFO("fw version changed, raise flag..\n");
      versionChanged = true;
    }
    
    String version = String(settings.fw.version);
    if( ((version.startsWith("0.") || version.startsWith("1.") || version.startsWith("2."))) ){
      memcpy(settings.fw.version,data,sizeof(settings));
      memset(settings.fw.version,0,sizeof(settings.fw.version));
      memcpy(settings.fw.version,FW_VERSION,sizeof(FW_VERSION));
      call.write_file(FW_SETTINGS_FILENAME,settings.fw.version,sizeof(settings));
      if( memcmp(settings.fw_build.model,FW_MODEL,sizeof(FW_MODEL)) != 0 ){
        LOG_INFO("fw model changed, raise flag..\n");
        modelChanged = true;
      }
    }
    else{
      LOG_INFO("resetting settings..\n");
      call.fw_reset();
      call.init_filesystem(directory,NUMITEMS(directory));

      memset(settings.fw.version,0,sizeof(settings.fw.version));
      memcpy(settings.fw.version,FW_VERSION,sizeof(FW_VERSION));
      versionChanged = true;
      modelChanged = true;
      variantChanged = true;
      // modem
  #ifdef ENABLE_LTE
      memcpy(settings.modem.apn,SETTINGS_MODEM_APN,sizeof(settings.modem.apn));
      memcpy(settings.modem.user,SETTINGS_MODEM_USERNAME,sizeof(settings.modem.user));
      memcpy(settings.modem.pwd,SETTINGS_MODEM_PASSWORD,sizeof(settings.modem.pwd));
      settings.modem.cops = SETTINGS_MODEM_COPS;
      settings.modem.band = SETTINGS_MODEM_BAND;
      settings.modem.tech = SETTINGS_MODEM_TECH;
  #endif

      call.write_file(FW_SETTINGS_FILENAME,settings.fw.version,sizeof(settings));
    }
    free(data);
  }
}

void Core::init(){
  
  
  sensors.init();

  sensors.setCallbacks(new CALLBACKS_SENSORS());


  uint16_t len = 2048;
  char* data = (char*)malloc(len);
  if(data != nullptr){
    call.read_file(FW_AR_FILENAME,data,&len);
    String file = String(data);
    if(!sensors.init_ar(data)){
      LOG_WARN("Autorequests not running !!\n");
    }

    len = 2048;
    memset(data,0,len);

    call.read_file(FW_ALARM_FILENAME,data,&len);
    file = String(data);
    if(!sensors.init_alarm(data)){
      LOG_WARN("Alarms not running !!\n");
    }
    free(data);
  }
  
  #ifdef ENABLE_JS
    JS.begin();

    char* code = (char*)malloc(1000);
    if(code != nullptr){
      uint16_t len = 1000;
      if(call.read_file(FW_JS_FILENAME,code,&len)){
        if(len != 0){
          const char* res = JS.exec((const char*)code);
          LOG_DEBUG("%s\n", res);
        }else LOG_WARN("no JS script is empty..\n");
      }else LOG_WARN("no JS script found..\n");
    }
    free(code);
    /*
    const char* code = "let e = {onSensorRead: function(name,value){log(name); if(value === 1) mqtt.send(\"/sensor/\"+name,value,0); return true;}}; let timer_id = timer.create(2000,'sensor.read(\"asd\")');";
    const char* res = JS.exec(code);
    LOG_DEBUG("%s\n", res);
    */
  #endif

  #ifdef SYNCHED_APP
    app.init();
  #endif
}

uint32_t keepaliveTimeout = 0;
uint32_t logTimeout = 0;
void Core::loop(){

  if(settings.keepalive.active && keepaliveTimeout < millis()){

    String sHeapFree = String(ESP.getFreeHeap() / 1024);
    String sUptime = String(millis()/1000);
    String sRssi = String(mRTOS.get_rssi());
    
    String topic = "/fw";
    String payload = "{\"heapFree\":"+sHeapFree+",\"uptime\":"+sUptime+",\"rssi\":"+sRssi+"}";

    mRTOS.mqtt_pushMessage(CLIENTID,topic,payload,0,true);

    keepaliveTimeout = millis()+(settings.keepalive.period*1000);

    sensors.loop();
    check_records();
    String directory = APP_PATH_RECORDS;
    call.clean_dir(directory);
  }

  if(settings.log.active && logTimeout < millis()){
    String heapFree = String(ESP.getFreeHeap() / 1024);
    LOG_INFO("\n\n----- Info -----\n\n");
    LOG_INFO("heap free: %s KiB\n", heapFree.c_str());
    LOG_INFO("%s\n", date().c_str());
    mRTOS.log_modem_status();
    LOG_INFO("--- ----- --- \n\n");
    logTimeout = millis()+5000;
  }

  core.parse_mqtt_messages();

  #ifdef ENABLE_JS
    JS.loop();
  #endif

  #ifdef SYNCHED_APP
    app.loop();
  #endif
}

/*
* Do not edit it
*/
void Core::parse_mqtt_messages(){
  msg = mRTOS.mqtt_getNextMessage(msg);

  int16_t index = -1;

  // --- PARSE MQTT MESSAGES HERE ---
  if(msg == NULL)
    return;

  LOG_INFO("<< [%d] %s\n", msg->clientID, msg->topic);

  bool set = false;
  bool get = false;
  bool store = false;

  uint8_t clientID = msg->clientID;
  String topic = String(msg->topic);
  String topic_get = "";
  String topic_set = "";
  topic.replace("\"","");
  String payload = String(msg->data);

  String uid = MQTT_UID_PREFIX+mRTOS.macAddress();
  index = topic.indexOf(uid);
  if(index > -1)
    topic = topic.substring(index+uid.length());

  if(topic == "/status"){
      LOG_INFO("update clock..\n");
      mRTOS.update_clock_sys();
  }else if(topic.startsWith("/settings")){
    if(topic.endsWith("/set")){
      set = true;
      index = topic.lastIndexOf("/");
      topic_set = topic.substring(0,index); // set filtered
      if(payload == "")
        return;
    }

    if(topic.endsWith("/get")){
      get = true;
      index = topic.lastIndexOf("/");
      topic_get = topic.substring(0,index); // get filtered
    }

    switch(resolveOption(fwTopics,topic)){
      case settings_update_:
        {
          DeserializationError error = deserializeJson(doc, payload);
          if(error){
            LOG_WARN("Not Json\n");
            return;
          }

          if(doc.containsKey("url")){
            String url = doc["url"];
            LOG_INFO("updating fw from %s\n", url.c_str());
            call.fw_settings_update(url,FW_SETTINGS_FILENAME);
          }

        }
        break;
      case settings_wifi_get_:
        {
          String ssid = String(settings.wifi.ssid);
          String pwd = String(settings.wifi.pwd);
          String payload = "{\"ssid\":\""+ssid+"\",\"pwd\":\""+pwd+"\"}";
          core_send_mqtt_message(clientID,topic_get,payload,2,false);
        }
        break;
      case settings_wifi_:
        {
          DeserializationError error = deserializeJson(doc, payload);
          if(error){
            LOG_WARN("Not Json\n");
            return;
          }
          store = true;
          if(doc.containsKey("mode")){
            #ifndef UNITTEST
              String mode = doc["mode"];
            #else
              String mode = "";
              if(doc["mode"].is_string())
                mode = doc["mode"];
            #endif
            settings_set_param("wifi_mode",mode);
          }

          if(doc.containsKey("ssid")){
            #ifndef UNITTEST
              String ssid = doc["ssid"];
            #else
              String ssid = "";
              if(doc["ssid"].is_string())
                ssid = doc["ssid"];
            #endif
            settings_set_param("wifi_ssid",ssid);
          }

          if(doc.containsKey("pwd")){
            #ifndef UNITTEST
              String pwd = doc["pwd"];
            #else
              String pwd = "";
              if(doc["pwd"].is_string())
                pwd = doc["pwd"];
            #endif
            settings_set_param("wifi_pwd",pwd);
          }
        }
        break;
      case settings_modem_get_:
        {
          String apn = String(settings.modem.apn);
          String user = String(settings.modem.user);
          String pwd = String(settings.modem.pwd);
          String band = String(settings.modem.band);
          String cops = String(settings.modem.cops);
          String tech = String(settings.modem.tech);

          String payload = "{\"apn\":\""+apn+"\",\"user\":\""+user+"\",\"pwd\":\""+pwd+"\",\"band\":"+band+",\"cops\":"+cops+",\"tech\":"+tech+"}";
          core_send_mqtt_message(clientID,topic_get,payload,2,false);
        }
        break;
      case settings_modem_:
        {
          DeserializationError error = deserializeJson(doc, payload);
          if(error){
            LOG_WARN("Not Json\n");
            return;
          }
          store = true;
          if(doc.containsKey("apn")){
            #ifndef UNITTEST
              String apn = doc["apn"];
            #else
              String apn = "";
              if(doc["apn"].is_string())
                apn = doc["apn"];
            #endif
            settings_set_param("modem_apn",apn);
          }

          if(doc.containsKey("pwd")){
            #ifndef UNITTEST
              String pwd = doc["pwd"];
            #else
              String pwd = "";
              if(doc["pwd"].is_string())
                pwd = doc["pwd"];
            #endif
            settings_set_param("modem_pwd",pwd);
          }

          if(doc.containsKey("user")){
            #ifndef UNITTEST
              String user = doc["user"];
            #else
              String user = "";
              if(doc["user"].is_string())
                user = doc["user"];
            #endif
            settings_set_param("modem_user",user);
          }

          if(doc.containsKey("band")){
            #ifndef UNITTEST
            String band = doc["band"];
            #else
            String band = "";
            if(doc["band"].is_number())
              band = std::to_string((long)doc["band"]);
            #endif
            settings_set_param("modem_band",band);
          }

          if(doc.containsKey("cops")){
            #ifndef UNITTEST
            String cops = doc["cops"];
            #else
            String cops = "";
            if(doc["cops"].is_number())
              cops = std::to_string((long)doc["cops"]);
            #endif
            settings_set_param("modem_cops",cops);
          }

          if(doc.containsKey("tech")){
            #ifndef UNITTEST
            String tech = doc["tech"];
            #else
            String tech = "";
            if(doc["tech"].is_number())
              tech = std::to_string((long)doc["tech"]);
            #endif
            settings_set_param("modem_tech",tech);
          }
        }
        break;
      case settings_mqtt_get_:
        {
          String host = String(settings.mqtt2.host);
          String user = String(settings.mqtt2.user);
          String pass = String(settings.mqtt2.pass);
          String port = String(settings.mqtt2.port);
          String active = String(settings.mqtt2.active);
          String payload = "{\"host\":\""+host+"\",\"user\":\""+user+"\",\"pass\":\""+pass+"\",\"port\":"+port+",\"active\":"+active+"}";
          core_send_mqtt_message(clientID,topic_get,payload,2,false);
        }
        break;
      case settings_mqtt_:
        {
          LOG_INFO("updating mqtt\n");

          DeserializationError error = deserializeJson(doc, payload);
          if(error){
            LOG_WARN("Not Json\n");
            return;
          }

          //serializeJson(doc,Serial);
          store = true;
          // checked
          if(doc.containsKey("host")){
            #ifndef UNITTEST
              String host = doc["host"];
            #else
              String host = "";
              if(doc["host"].is_string())
                host = doc["host"];
            #endif
            settings_set_param("mqtt_host",host);
          }

          if(doc.containsKey("user")){
            #ifndef UNITTEST
              String user = doc["user"];
            #else
              String user = "";
              if(doc["user"].is_string())
                user = doc["user"];
            #endif
            settings_set_param("mqtt_user",user);
          }

          if(doc.containsKey("pass")){
            #ifndef UNITTEST
              String pass = doc["pass"];
            #else
              String pass = "";
              if(doc["pass"].is_string())
                pass = doc["pass"];
            #endif
            settings_set_param("mqtt_pass",pass);
          }

          if(doc.containsKey("prefix")){
            #ifndef UNITTEST
              String prefix = doc["prefix"];
            #else
              String prefix = "";
              if(doc["prefix"].is_string())
                prefix = doc["prefix"];
            #endif
            settings_set_param("mqtt_prefix",prefix);
          }

          if(doc.containsKey("port")){
            #ifndef UNITTEST
            String port = doc["port"];
            #else
            String port = "";
            if(doc["port"].is_number())
              port = std::to_string((long)doc["port"]);
            #endif
            settings_set_param("mqtt_port",port);
          }

          if(doc.containsKey("active")){
            #ifndef UNITTEST
            String active = doc["active"];
            #else
            String active = "";
            if(doc["active"].is_number())
              active = std::to_string((long)doc["active"]);
            #endif
            settings_set_param("mqtt_active",active);
          }
        }
        break;
      case settings_log_get_:
        {
          String level = String(settings.log.level);
          String payload = "{\"level\":"+level+"}";
          core_send_mqtt_message(clientID,topic_get,payload,2,false);
        }
        break;
      case settings_log_:
        {
          DeserializationError error = deserializeJson(doc, payload);
          if(error){
            LOG_WARN("Not Json\n");
            return;
          }
          store = true;
          if(doc.containsKey("active")){
            #ifndef UNITTEST
            String active = doc["active"];
            #else
            String active = "";
            if(doc["active"].is_number())
              active = std::to_string((long)doc["active"]);
            #endif
            settings_set_param("log_active",active);
          }

          if(doc.containsKey("level")){
            #ifndef UNITTEST
            String level = doc["level"];
            #else
            String level = "";
            if(doc["level"].is_number())
              level = std::to_string((long)doc["level"]);
            #endif
            settings_set_param("log_level",level);
          }
        }
        break;
      case settings_keepalive_get_:
        {
          String period = String(settings.keepalive.period);
          String payload = "{\"period\":"+period+"}";
          core_send_mqtt_message(clientID,topic_get,payload,2,false);
          break;
        }
      case settings_keepalive_:
        {
          DeserializationError error = deserializeJson(doc, payload);
          if(error){
            LOG_WARN("Not Json\n");
            return;
          }

          if(doc.containsKey("active")){
            #ifndef UNITTEST
            String active = doc["active"];
            #else
            String active = "";
            if(doc["active"].is_number())
              active = std::to_string((long)doc["active"]);
            #endif
            settings_set_param("keepalive_active",active);
          }

          if(doc.containsKey("period")){
            #ifndef UNITTEST
            String period = doc["period"];
            #else
            String period = "";
            if(doc["period"].is_number())
              period = std::to_string((long)doc["period"]);
            #endif
            settings_set_param("keepalive_period",period);
          }
        }
        break;
      case settings_serial_get_:
        {
          String active = String(settings.uart2.active);
          String baudrate = String(settings.uart2.baudrate);
          String config = String(settings.uart2.config);

          String payload = "{\"active\":"+active+",\"baudrate\":"+baudrate+",\"config\":"+config+"}";
          core_send_mqtt_message(clientID,topic_get,payload,2,false);
          break;
        }
      case settings_serial_:
        {
          DeserializationError error = deserializeJson(doc, payload);
          if(error){
            DBGLOG(Error,"Not Json");
            return;
          }
          store = true;
          if(doc.containsKey("active")){
            #ifndef UNITTEST
            String active = doc["active"];
            #else
            String active = "";
            if(doc["active"].is_number())
              active = std::to_string((long)doc["active"]);
            #endif
            settings.uart2.active = (bool)active.toInt() != 0;
          }
          if(doc.containsKey("baudrate")){
            #ifndef UNITTEST
            String baudrate = doc["baudrate"];
            #else
            String baudrate = "";
            if(doc["baudrate"].is_number())
              baudrate = std::to_string((long)doc["baudrate"]);
            #endif
            settings.uart2.baudrate = (long)baudrate.toInt();
          }

          if(doc.containsKey("config")){
            #ifndef UNITTEST
            String config = doc["config"];
            #else
            String config = "";
            if(doc["config"].is_number())
              config = std::to_string((long)doc["config"]);
            #endif
            settings.uart2.config = (long)config.toInt();
          }

          // This instruction is crashing esp32. I don't understand why.
          // If the next instruction is commented a reboot is need for changes be applied
          //sensors.rs485_change_config(1,settings.uart2.baudrate,settings.uart2.config);

          break;
        }
    }

    // store settings
    if(store){
      settings_log();
      if(!call.write_file(FW_SETTINGS_FILENAME,settings.fw.version,sizeof(settings)))
        LOG_ERROR("failing writing file: %s\n", FW_SETTINGS_FILENAME);
    }

    if(set)
      core_send_mqtt_message(clientID,topic,"",1,true); // msg is delivered at least once

  }else if(topic.startsWith("/fw")){

    if(topic.endsWith("/set")){
      set = true;
      index = topic.lastIndexOf("/");
      topic_set = topic.substring(0,index); // set filtered
      if(payload == "")
        return;
    }

    if(topic.endsWith("/get")){
      get = true;
      index = topic.lastIndexOf("/");
      topic_get = topic.substring(0,index); // get filtered
    }

    switch(resolveOption(fwTopics,topic)){
      case fw_get_:
        {
          LOG_INFO("getting fw info..\n");
          //core_send_mqtt_message(clientID,/version,String(FW_VERSION),0,true);

          String sHeapFree = String(ESP.getFreeHeap() / 1024);
          String sUptime = String(millis()/1000);
          String sRssi = String(mRTOS.get_rssi());
          
          String payload = "{\"heapFree\":"+sHeapFree+",\"uptime\":"+sUptime+",\"rssi\":"+sRssi+"}";
          core_send_mqtt_message(clientID,topic_get,payload,0,false);
        }
        break;
      case fw_reboot_:

        if(payload != "1") return;

        LOG_INFO("reboot..\n");
        // !! unpublish topic
        //mqtt_pushMessage(clientID,topic+"/set","",1,true);
        //flag_restart = true;
        call.fw_reboot();
        break;
      case fw_reset_:

        if(payload != "1")
          return;

        LOG_INFO("reset..\n");
        // !! unpublish topic
        //mqtt_pushMessage(clientID,topic+"/set","",1,true);
        call.fw_reset();
        call.fw_reboot();
        break;
      case fw_clean_records_:
        if(payload != "1")
          return;
        call.remove_dir(APP_PATH_RECORDS);
        break;
      case fw_fota_update_:
        {
          DeserializationError error = deserializeJson(doc, payload);
          if(error){
            LOG_WARN("Not Json\n");
            return;
          }

          if(doc.containsKey("url")){
            String url = doc["url"];
            if(doc.containsKey("token")){
              String token = doc["token"];
            }else{
              LOG_INFO("fota from %s\n", url.c_str());
              String error = "";
              #ifndef ENABLE_LTE
                error = fota(url);
              #else
                error = call.fw_fota(url);
              #endif
              if(error != ""){
                core_send_mqtt_message(clientID,topic_set+"/status",error,2,false);
              }
            }
          }

        }
        break;
      case fw_ar_get_:
        {
        String md5 = call.get_file_md5(FW_AR_FILENAME);
        String payload = "{\"md5\":\""+md5+"\"}";
        core_send_mqtt_message(clientID,topic_get,payload,2,false);
        }
        break;
      case fw_ar_:
        if(!call.write_file(FW_AR_FILENAME,payload.c_str(),payload.length()))
          LOG_ERROR("Error storing Autorequests file\n");
        break;
      case fw_alarm_get_:
        {
        String md5 = call.get_file_md5(FW_ALARM_FILENAME);
        String payload = "{\"md5\":\""+md5+"\"}";
        core_send_mqtt_message(clientID,topic_get,payload,2,false);
        }
        break;
      case fw_alarm_:
        if(!call.write_file(FW_ALARM_FILENAME,payload.c_str(),payload.length()))
          LOG_ERROR("Error storing Alarms file\n");
        break;
      case fw_js_program_get_:
        {
        #ifdef ENABLE_JS
          String md5 = call.get_file_md5(FW_JS_FILENAME);
          String payload = "{\"md5\":\""+md5+"\"}";
          core_send_mqtt_message(clientID,topic_get,payload,2,false);
        #endif
        }
        break;
      case fw_js_program_:
        {
          #ifdef ENABLE_JS
          LOG_DEBUG("%s\n", payload.c_str());
          const char* res = JS.exec(payload.c_str());
          LOG_DEBUG("%s\n", res);
          if(!call.write_file(FW_JS_FILENAME,payload.c_str(),payload.length()))
            LOG_ERROR("Error storing js script\n");
          #else
          LOG_WARN("JS not enabled\n");
          #endif
        }
        break;
#ifdef ENABLE_RS485
      case fw_serial_read_get_:
        {
          LOG_DEBUG("rs485 read\n");
          int16_t len = 4;
          uint16_t arr[len];

          sensors.parseArray(payload,arr,&len);
          if(len != 4)
            break; // array contains invalid data

          uint16_t size = 6+arr[3]*2;

          uint8_t* data = (uint8_t*)malloc(size);
          if(data == nullptr)
            break;

          // read rs485
          uint8_t error = sensors.rs485_read((uint8_t)arr[0],(uint8_t)arr[1],arr[2],arr[3],data,&size);
          String data_str = "";
          if(error == 0){
            data_str = "{\"res\":\"0x";
            for(uint8_t i=0;i<size;i++){
              data_str += String(data[i],HEX);
            }
            data_str += "\"}";
          }else{
            data_str = "{\"error\":\"0x"+String(error,HEX)+"\"}";
          }

          free(data);
          // push message
          core_send_mqtt_message(clientID,topic_get,data_str,2,false);

          free(data);
          //free(arr);
        }
        break;
      case fw_serial_write_get_:
        {
          LOG_DEBUG("rs485 write\n");

          int16_t len = 20;
          uint16_t* arr = (uint16_t*)malloc(len);
          if(arr == nullptr)
            break;

          sensors.parseArray(payload,arr,&len);

          LOG_DEBUG("array: ");
          for(uint8_t i=0;i<len;i++){
            LOG_DEBUG("%d ",arr[i]);
          }
          LOG_DEBUG("\n");

          if(len < 6)
            break; // array is too short to contain valid data

          uint16_t size = arr[4]+1;
          uint8_t* data = (uint8_t*)malloc(size);
          if(data == nullptr)
            break;
          for(uint8_t i=0;i<size;i++){
            data[i] = arr[4+i];
          }

          // write rs485
          uint8_t error = sensors.rs485_write((uint8_t)arr[0],(uint8_t)arr[1],arr[2],arr[3],data,&size);

          free(data);
          free(arr);
          String data_str = "";
          if(error){
            data_str = "{\"error\": "+String(error)+"}";
          }else{
            data_str += "{\"res\":\"packet written\"}";
          }

          // push message
          core_send_mqtt_message(clientID,topic_get,data_str,2,false);
        }
        break;
#endif
      case fw_wifi_get_:
        {
          String ssid    = WiFi.SSID();
          int32_t rssi   = WiFi.RSSI();
          int32_t channel = WiFi.channel();
          String bssid   = WiFi.BSSIDstr();
          bssid.toLowerCase();
          bssid.replace(":","");

          String wifi_json = "{\"ssid\":\"" + ssid +
                             "\",\"channel\":" + String(channel) +
                             ",\"rssi\":" + String(rssi) +
                             ",\"bssid\":\"" + bssid + "\"}"
          ;
          core_send_mqtt_message(clientID, topic_get, wifi_json, 2, false);
        }
        break;
      default:
        break;
    }

    if(set)
      core_send_mqtt_message(clientID,topic,"",1,true); // msg is delivered at least once

  }else{
    app.parse_mqtt_messages(clientID,topic,payload);
  }
}

bool core_send_mqtt_message(uint8_t clientID, String topic, String data, uint8_t qos, bool retain){

  #ifdef DEBUG_MQTT_TOPIC
  LOG_INFO(">> [%d] %s\n", clientID, topic.c_str());
  #endif
  #ifdef DEBUG_MQTT_PAYLOAD
    LOG_DEBUG("[data]: %s\n", data.c_str());
  #endif
  return call.mqtt_send(clientID,topic,data,qos,retain);
}

bool Core::store_record(String filename, const char* data, uint16_t len){
  String root = APP_PATH_RECORDS;
  //String path = root + "/"+ String(millis()/1000);
  String path = root + "/"+ String(now());
  if(!call.create_dir(path))
    return false;
  path += filename;
  if(!call.store_record(path,data,len))
    return false;
  return true;
}

void Core::check_records(){

  uint8_t clientID = CLIENTID;
  if(settings.mqtt.active)
    clientID = CLIENTIDEXTERNAL;

  if(!mRTOS.mqtt_isConnected(clientID))
    return;

  uint32_t timeout = millis() + 5000;
  bool (*send_ar)(String);
  send_ar = &Core::send_record;
  String path = APP_PATH_RECORDS;
  call.check_filesystem_records(path.c_str(),timeout,send_ar); // iterate through all subdirectories

}

// !! callback - sysfile safe
bool Core::send_record(String filename){

  // This is a safe function
  // Inside it you can call sysfile class

  uint16_t len = MAX_RECORD_FILE_SIZE;
  char* data = (char*)malloc(len);
  if(data == nullptr) return false;

  memset(data,0,len);

  sysfile.read_file(filename.c_str(),data,&len);

  String filename_bck = filename;
  String root_path = APP_PATH_RECORDS;
  uint8_t index = filename.indexOf(root_path.c_str());
  String path = filename.substring(index+root_path.length());
  index = path.lastIndexOf(".");
  path = path.substring(0,index);
  String topic = MQTT_PATH_RECORDS;
  topic += path;
  LOG_DEBUG("send record: %s\n", topic.c_str());
  uint8_t clientID = CLIENTID;
  if(settings.mqtt.active)
    clientID = CLIENTIDEXTERNAL;
  if(core_send_mqtt_message(clientID,topic,String(data),2,1)){
    if(!sysfile.delete_file(filename_bck.c_str()))
      LOG_ERROR("Couldn't delete file: %s\n", filename_bck.c_str());
  }else{
    free(data);
    return false;
  }

  free(data);
  return true;
}

uint8_t parse_float_array(float* arr, uint8_t len, String payload){

    int8_t index = 0, i = 0;
    String value = "";

    while(i < len){
      index = payload.indexOf(",");

      if(index == -1){
        value = payload;
        if(payload.length() > 0){
          arr[i++] = value.toFloat();
          //Serial.printf("value: %f\n",arr[i-1]);
        }
        break;
      }else{
        value = payload.substring(0,index);
        arr[i++] = value.toFloat();
        //Serial.printf("value: %f\n",arr[i-1]);
      }
      payload = payload.substring(index+1);
    }
    return i;

}

// find fw topic
fwTopics_ resolveOption(std::map<long, fwTopics_> map, String topic) {

  std::string topic_ = std::string(topic.c_str());
  long str_hash = (long)std::hash<std::string>{}(topic_);
  std::map<long,fwTopics_>::iterator it;

  it = map.find(str_hash);
  if(it != map.end())
    return it->second;

  return fw_not_found;
}

#ifdef ENABLE_BLE
// find settings param
uuidTopics_ resolveOptionUUID(std::map<long, uuidTopics_> map, String param) {

  std::string param_ = std::string(param.c_str());
  long str_hash = (long)std::hash<std::string>{}(param_);
  std::map<long,uuidTopics_>::iterator it;

  it = map.find(str_hash);
  if(it != map.end())
    return it->second;

  return uuid_not_found;
}
#endif

String get_uid() {
  return String(MQTT_UID_PREFIX) + mRTOS.macAddress();
}

String date() {
  return String(year()) + "-" + pad2(month()) + "-" + pad2(day()) + " " + pad2(hour()) + ":" + pad2(minute()) + ":" + pad2(second());
}

String pad2(int value) {
  return String(value < 10 ? "0" : "") + String(value);
}

void update_started() {
  LOG_INFO("CALLBACK:  HTTP update process started\n");
}

void update_finished() {
  LOG_INFO("CALLBACK:  HTTP update process finished\n");
}

void update_progress(int cur, int total) {
  LOG_INFO("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
}

void update_error(int err) {
  LOG_ERROR("CALLBACK:  HTTP update fatal error code %d\n", err);
}

// get certificate in ESP32 code formate from that link
//https://projects.petrucci.ch/esp32/?page=ssl.php&url=https%3A%2F%2Fdevices.dev.inloc.cloud

// certificate for https://devices.dev.inloc.cloud
// E5, valid until Mon May 26 2025, size: 1294 bytes 
const char* rootCACertificate = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDjjCCAxSgAwIBAgISBPM3s2vn9n5mKMBCNrLgPY/HMAoGCCqGSM49BAMDMDIx\n" \
"CzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MQswCQYDVQQDEwJF\n" \
"NTAeFw0yNTAyMjQyMzA0MDlaFw0yNTA1MjUyMzA0MDhaMCIxIDAeBgNVBAMTF2Rl\n" \
"dmljZXMuZGV2LmlubG9jLmNsb3VkMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE\n" \
"x5uPg1lGDMvvvO6e1jrBHQEmFM6mMsrSmtcJks+HWoS3evHmSJrNrRMJql/bQItS\n" \
"PS6aZ+xtNonSdTcQgNEuaqOCAhgwggIUMA4GA1UdDwEB/wQEAwIHgDAdBgNVHSUE\n" \
"FjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwDAYDVR0TAQH/BAIwADAdBgNVHQ4EFgQU\n" \
"jZs6sL1RBB+HnRUwgjrArx24zLUwHwYDVR0jBBgwFoAUnytfzzwhT50Et+0rLMTG\n" \
"cIvS1w0wVQYIKwYBBQUHAQEESTBHMCEGCCsGAQUFBzABhhVodHRwOi8vZTUuby5s\n" \
"ZW5jci5vcmcwIgYIKwYBBQUHMAKGFmh0dHA6Ly9lNS5pLmxlbmNyLm9yZy8wIgYD\n" \
"VR0RBBswGYIXZGV2aWNlcy5kZXYuaW5sb2MuY2xvdWQwEwYDVR0gBAwwCjAIBgZn\n" \
"gQwBAgEwggEDBgorBgEEAdZ5AgQCBIH0BIHxAO8AdgDm0jFjQHeMwRBBBtdxuc7B\n" \
"0kD2loSG+7qHMh39HjeOUAAAAZU6aa9VAAAEAwBHMEUCIQDL0Hpkm5nO2IryokWx\n" \
"wlVK9nTGeJEglrC5KFI4yRBoQAIgHh0kADXgq6zQvOpiIGqfgZQw76+4KjPMjBB/\n" \
"bXWVCRsAdQATSt8atZhCCXgMb+9MepGkFrcjSc5YV2rfrtqnwqvgIgAAAZU6abBb\n" \
"AAAEAwBGMEQCIEmEgsWxhQ4ktlpqV5vH09J4nYo6RiVNECFbT76AOTiHAiBT1Tqa\n" \
"P/2BlwtYp5HfPQIEcsm8kvMnrk2tcG8Tiw0D+DAKBggqhkjOPQQDAwNoADBlAjB1\n" \
"8kUItHXwXe9lYiXjeKfFgsPCOVF5u3wfKXhxtjyblMU3C3ZiE8lskwpLlrDJHXIC\n" \
"MQDvWCDjTrqp6JDYAEJBpRMj3Qg48TZY5WOwbAGFzUVHr4/qzneKsgNelR6vWJ5c\n" \
"Et8=\n" \
"-----END CERTIFICATE-----\n" \
"";

String Core::fota(String url){

  // Add x-MD5 to header 
  httpUpdate.onStart(update_started);
  httpUpdate.onEnd(update_finished);
  httpUpdate.onProgress(update_progress);
  httpUpdate.onError(update_error);

  //NetworkClient client;
  NetworkClientSecure client;
  client.setInsecure();
  //client.setCACert(rootCACertificate); // not working

  String uid = get_uid();
  t_httpUpdate_return ret = httpUpdate.update(client, url, "", [&uid](HTTPClient* http) {
    http->addHeader("x-uid", uid);
    http->addHeader("x-fw-model", FW_MODEL);
    http->addHeader("x-fw-variant", FW_VARIANT);
  });

  String error = "";
  switch (ret) {
    case HTTP_UPDATE_FAILED: 
      error = "HTTP_UPDATE_FAILED Error: ("+ String(httpUpdate.getLastError()) +") "+ httpUpdate.getLastErrorString();
      LOG_ERROR("%s\n", error.c_str()); 
      break;

    case HTTP_UPDATE_NO_UPDATES: 
      error = "HTTP_UPDATE_NO_UPDATES";
      LOG_WARN("%s\n", error.c_str()); 
      break;

    case HTTP_UPDATE_OK: 
      LOG_INFO("HTTP_UPDATE_OK\n"); 
      break;
  }
  return error;
}
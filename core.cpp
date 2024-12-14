
#include "core.h"

SemaphoreHandle_t spiffsMutex;
CALLS call;
SYSFILE sysfile;
extern SENSORS sensors;
APP app;

MODEMfreeRTOS mRTOS; // freeRTOS modem
MQTT_MSG_RX* msg; // mqtt
extern DynamicJsonDocument doc; // json

#ifdef ENABLE_AP
  void CALLBACKS_WIFI_AP::onWiFiSet(String ssid, String pass){
    settings_set_param("wifi_ssid",ssid);
    settings_set_param("wifi_pwd",pass);

    settings_log();
    if(sysfile.write_file(FW_SETTINGS_FILENAME,settings.fw.version,sizeof(settings))){
      Serial.println("Client Disconnected.");
      call.fw_reboot();
    }else{
      Serial.println("failing writing file: "+String(FW_SETTINGS_FILENAME));
    }
  }
#endif

#ifdef ENABLE_BLE
  BLE_SERVER ble;
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
        Serial.println("failing writing file: "+String(FW_SETTINGS_FILENAME));
    }
  }
#endif

String directory[] = {
  SYS_PATH_SETTINGS,
  APP_PATH_SETTINGS,
  APP_PATH_RECORDS
};

void CALLBACKS_SENSORS::onReadSensor(String ref, String value){
  Serial.println("onReadSensor callback called");
  #ifdef ENABLE_JS
  String code = "event.onReadSensor(\""+ref+"\","+value+")";
  const char* res = JS.call(code.c_str());     // Execute JS code
  Serial.println(res);
  #else
  call.mqtt_send("/"+ref,value,2,0);
  #endif
};

void CALLBACKS_SENSORS::onAlarmSensor(String ref, String value){
  Serial.println("onAlarmSensor callback called");
  #ifdef ENABLE_JS
  String code = "event.onAlarmSensor(\""+ref+"\","+value+")";
  const char* res = JS.call(code.c_str());     // Execute JS code
  Serial.println(res);
  #endif
};

void CALLBACKS_SENSORS::onAlarmTrigger(String ref, String value){
  Serial.println("onAlarmTrigger callback called");
  #ifdef ENABLE_JS
  String code = "event.onAlarmTrigger(\""+ref+"\","+value+")";
  const char* res = JS.call(code.c_str());     // Execute JS code
  Serial.println(res);
  #else
  call.mqtt_send("/"+ref,value,2,0);
  #endif
};

void CALLBACKS_SENSORS::onRS485ReadAll(String data_json){
  Serial.println("onRS485ReadAll callback called");
  Serial.println("core: "+data_json);
  String filename = ".txt";
  core_store_record(filename,data_json.c_str(),data_json.length());
};


bool CALLBACKS_SENSORS::getAppValue(JsonObject& obj, String ref){

  return app.getValue(obj,ref);
};


/*
* load settings
*/
void core_load_settings(){

  uint16_t len = sizeof(settings);
  char* data = (char*)malloc(len);
  if(data != nullptr){
    call.read_file(FW_SETTINGS_FILENAME,data,&len);
    memcpy(settings.fw.version,data,sizeof(settings.fw.version));
    String version = String(settings.fw.version);
    Serial.println("fw version: "+version);
    if( ((version.startsWith("0.") || version.startsWith("1.") || version.startsWith("2."))) )
      memcpy(settings.fw.version,data,sizeof(settings));
    else{
      call.fw_reset();
      call.init_filesystem(directory,NUMITEMS(directory));

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

void core_init(){
  
  
  sensors.init();

  sensors.setCallbacks(new CALLBACKS_SENSORS());


  uint16_t len = 2048;
  char* data = (char*)malloc(len);
  if(data != nullptr){
    call.read_file(FW_AR_FILENAME,data,&len);
    String file = String(data);
    if(!sensors.init_ar(data)){
      Serial.println("Autorequests not running !!");
    }

    len = 2048;
    memset(data,0,len);

    call.read_file(FW_ALARM_FILENAME,data,&len);
    file = String(data);
    if(!sensors.init_alarm(data)){
      Serial.println("Alarms not running !!");
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
          Serial.printf("%s\n", res);
        }else Serial.println("no JS script is empty..");
      }else Serial.println("no JS script found..");
    }
    free(code);
    /*
    const char* code = "let e = {onSensorRead: function(name,value){log(name); if(value === 1) mqtt.send(\"/sensor/\"+name,value,0); return true;}}; let timer_id = timer.create(2000,'sensor.read(\"asd\")');";
    const char* res = JS.exec(code);
    Serial.printf("%s\n", res);
    */
  #endif

  #ifdef SYNCHED_APP
    app.init();
  #endif
}

uint32_t timeout = 0;
void core_loop(){

  if(timeout < millis()){
    Serial.println("send info");

    String heapFree = String(ESP.getFreeHeap() / 1024);
    String topic = "/heapFree";
    mRTOS.mqtt_pushMessage(CLIENTID,topic,heapFree,0,0);

    topic = "/uptime";
    String payload = String(millis()/1000);
    mRTOS.mqtt_pushMessage(CLIENTID,topic,payload,0,false);

    Serial.println("\n\n----- Info -----\n");
    Serial.println("heap free: " + heapFree + " KiB");
    Serial.println(date());
    mRTOS.log_modem_status();
    Serial.println("--- ----- --- \n\n");

    timeout = millis()+5000;

    sensors.loop();
    core_check_records();
    String directory = APP_PATH_RECORDS;
    call.clean_dir(directory);
  }

  core_parse_mqtt_messages();

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
void core_parse_mqtt_messages(){
  msg = mRTOS.mqtt_getNextMessage(msg);

  int16_t index = -1;

  // --- PARSE MQTT MESSAGES HERE ---
  if(msg == NULL)
    return;

  Serial.println("<< ["+String(msg->clientID)+"] "+String(msg->topic));

  bool set = false;
  bool get = false;
  bool store = false;

  uint8_t clientID = msg->clientID;
  String topic = String(msg->topic);
  String topic_get = "";
  topic.replace("\"","");
  String payload = String(msg->data);

  String uid = MQTT_UID_PREFIX+mRTOS.macAddress();
  index = topic.indexOf(uid);
  if(index > -1)
    topic = topic.substring(index+uid.length());

  if(topic == "/status"){
      Serial.println("update clock..");
      mRTOS.update_clock_sys();
  }else if(topic.startsWith("/fw")){

    if(topic.endsWith("/set")){
      set = true;
      if(payload == "")
        return;
    }

    if(topic.endsWith("/get")){
      get = true;
      index = topic.lastIndexOf("/");
      topic_get = topic.substring(0,index); // get filtered
    }

    //Serial.println("topic: "+topic);
    switch(resolveOption(fwTopics,topic)){
      case fw_:
        Serial.println("getting fw info..");
        //mRTOS.mqtt_pushMessage(clientID,topic,"{\"version\":\""+String(PACKAGE_VERSION)+"\",\"model\":"+String(PACKAGE_MODEL)+",\"hash\":\""+String(settings.fw.hash)+"\""+",\"uptime\":"+String(millis())+"}",1,true);
        core_send_mqtt_message(clientID,topic_get,String(FW_VERSION),0,true);
        break;
      case fw_reboot_:

        if(payload != "1") return;

        Serial.println("reboot..");
        // !! unpublish topic
        //mqtt_pushMessage(clientID,topic+"/set","",1,true);
        //flag_restart = true;
        call.fw_reboot();
        break;
      case fw_reset_:

        if(payload != "1")
          return;

        Serial.println("reset..");
        // !! unpublish topic
        //mqtt_pushMessage(clientID,topic+"/set","",1,true);
        call.fw_reset();
        call.fw_reboot();
        break;
      case fw_info_:
        Serial.println("getting fw info..");
        //mqtt_pushMessage(clientID,topic,"{\"gps\":\"\",\"rssi\":0,\"timestamp\":"+String(getTimestamp())+",\"uptime\":"+String(millis())+"}",1,true);
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
            Serial.println("Not Json");
            return;
          }

          if(doc.containsKey("url")){
            String url = doc["url"];
            if(doc.containsKey("token")){
              String token = doc["token"];
            }else{
              Serial.println("fota from "+url);
              call.fw_fota(url);
            }
          }

        }
        break;
      case fw_settings_update_:
        {
          DeserializationError error = deserializeJson(doc, payload);
          if(error){
            Serial.println("Not Json");
            return;
          }

          if(doc.containsKey("url")){
            String url = doc["url"];
            Serial.println("updating fw from "+url);
            call.fw_settings_update(url,FW_SETTINGS_FILENAME);
          }

        }
        break;
      case fw_js_program_get_:
        {
        #ifdef ENABLE_JS
          String md5 = call.get_file_md5(FW_JS_FILENAME);
          String payload = "{\"md5\":\""+md5+"\"}";
          core_send_mqtt_message(clientID,topic_get,payload,0,false);
        #endif
        }
        break;
      case fw_js_program_:
        {
          #ifdef ENABLE_JS
          Serial.println(payload);
          const char* res = JS.exec(payload.c_str());
          Serial.printf("%s\n", res);
          if(!call.write_file(FW_JS_FILENAME,payload.c_str(),payload.length()))
            Serial.println("Error storing js script");
          #else
          Serial.println("JS not enabled");
          #endif
        }
        break;
      case fw_wifi_get_:
      {
        String ssid = String(settings.wifi.ssid);
        String pwd = String(settings.wifi.pwd);
        String payload = "{\"ssid\":\""+ssid+"\",\"pwd\":\""+pwd+"\"}";
        core_send_mqtt_message(clientID,topic_get,payload,0,false);
      }
      case fw_wifi_:
      {
        DeserializationError error = deserializeJson(doc, payload);
        if(error){
          Serial.println("Not Json");
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
      case fw_modem_get_:
      {
        String apn = String(settings.modem.apn);
        String user = String(settings.modem.user);
        String pwd = String(settings.modem.pwd);
        String band = String(settings.modem.band);
        String cops = String(settings.modem.cops);
        String tech = String(settings.modem.tech);

        String payload = "{\"apn\":\""+apn+"\",\"user\":\""+user+"\",\"pwd\":\""+pwd+"\",\"band\":"+band+",\"cops\":"+cops+",\"tech\":"+tech+"}";
        core_send_mqtt_message(clientID,topic_get,payload,0,false);
      }
      case fw_modem_:
      {
        DeserializationError error = deserializeJson(doc, payload);
        if(error){
          Serial.println("Not Json");
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
      case fw_mqtt_get_:
      {
        String host = String(settings.mqtt.host);
        String user = String(settings.mqtt.user);
        String pass = String(settings.mqtt.pass);
        String port = String(settings.mqtt.port);
        String active = String(settings.mqtt.active);
        String payload = "{\"host\":\""+host+"\",\"user\":\""+user+"\",\"pass\":\""+pass+"\",\"port\":"+port+",\"active\":"+active+"}";
        core_send_mqtt_message(clientID,topic_get,payload,0,false);
      }
      case fw_mqtt_:
      {
        Serial.println("updating mqtt");

        DeserializationError error = deserializeJson(doc, payload);
        if(error){
          Serial.println("Not Json");
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
      case fw_log_get_:
      {
        String level = String(settings.log.level);
        String payload = "{\"level\":"+level+"}";
        core_send_mqtt_message(clientID,topic_get,payload,0,false);
        break;
      }
      case fw_log_:
      {
        DeserializationError error = deserializeJson(doc, payload);
        if(error){
          Serial.println("Not Json");
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
      case fw_keepalive_get_:
        {
          String period = String(settings.keepalive.period);
          String payload = "{\"period\":"+period+"}";
          core_send_mqtt_message(clientID,topic_get,payload,0,false);
          break;
        }
      case fw_keepalive_:
        {
          DeserializationError error = deserializeJson(doc, payload);
          if(error){
            Serial.println("Not Json");
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
      case fw_serial_get_:
        {
          String active = String(settings.uart2.active);
          String baudrate = String(settings.uart2.baudrate);
          String config = String(settings.uart2.config);

          String payload = "{\"active\":"+active+",\"baudrate\":"+baudrate+",\"config\":"+config+"}";
          core_send_mqtt_message(clientID,topic_get,payload,0,false);
          break;
        }
      case fw_serial_:
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
      case fw_ar_get_:
        {
        String md5 = call.get_file_md5(FW_AR_FILENAME);
        String payload = "{\"md5\":\""+md5+"\"}";
        core_send_mqtt_message(clientID,topic_get,payload,0,false);
        }
        break;
      case fw_ar_:
        if(!call.write_file(FW_AR_FILENAME,payload.c_str(),payload.length()))
          Serial.println("Error storing Autorequests file");
        break;
      case fw_alarm_get_:
        {
        String md5 = call.get_file_md5(FW_ALARM_FILENAME);
        String payload = "{\"md5\":\""+md5+"\"}";
        core_send_mqtt_message(clientID,topic_get,payload,0,false);
        }
        break;
      case fw_alarm_:
        if(!call.write_file(FW_ALARM_FILENAME,payload.c_str(),payload.length()))
          Serial.println("Error storing Alarms file");
        break;
#ifdef ENABLE_RS485
      case fw_serial_read_get_:
        {
          Serial.println("rs485 read");
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
          core_send_mqtt_message(clientID,topic_get,data_str,0,false);

          free(data);
          //free(arr);
        }
        break;
      case fw_serial_write_get_:
        {
          Serial.println("rs485 write");

          int16_t len = 20;
          uint16_t* arr = (uint16_t*)malloc(len);
          if(arr == nullptr)
            break;

          sensors.parseArray(payload,arr,&len);

          Serial.print("array: ");
          for(uint8_t i=0;i<len;i++){
            Serial.printf("%d ",arr[i]);
          }
          Serial.println("");

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
          core_send_mqtt_message(clientID,topic_get,data_str,0,false);
        }
        break;
#endif
      default:
        //Serial.println("topic not known by fw topics");
        //Serial.println(payload);
        break;
    }
    // store settings
    if(store){
      settings_log();
      if(!call.write_file(FW_SETTINGS_FILENAME,settings.fw.version,sizeof(settings)))
        Serial.println("failing writing file: "+String(FW_SETTINGS_FILENAME));
    }

    if(set)
      core_send_mqtt_message(clientID,topic,"",0,true); // unpublish

  }else{
    app.parse_mqtt_messages(clientID,topic,payload);
  }
}

bool core_send_mqtt_message(uint8_t clientID, String topic, String data, uint8_t qos, bool retain){

  Serial.println(">> ["+String(clientID)+"] "+topic);
  return call.mqtt_send(clientID,topic,data,qos,retain);
}

bool core_store_record(String filename, const char* data, uint16_t len){
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

void core_check_records(){

  uint8_t clientID = CLIENTID;
  if(settings.mqtt.active)
    clientID = CLIENTIDEXTERNAL;

  //Serial.println("check records");
  if(!mRTOS.mqtt_isConnected(clientID))
    return;

  //Serial.println("check filesystem for new records");
  uint32_t timeout = millis() + 5000;
  bool (*send_ar)(String);
  send_ar = &core_send_record;
  String path = APP_PATH_RECORDS;
  call.check_filesystem_records(path.c_str(),timeout,send_ar); // iterate through all subdirectories

}

// !! callback - sysfile safe
bool core_send_record(String filename){

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
  Serial.println("send record: "+topic);
  uint8_t clientID = CLIENTID;
  if(settings.mqtt.active)
    clientID = CLIENTIDEXTERNAL;
  if(core_send_mqtt_message(clientID,topic,String(data),2,1)){
    if(!sysfile.delete_file(filename_bck.c_str()))
      Serial.println("Couldn't delete file: "+filename_bck);
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

String date() {
	return String(year()) + "-" + pad2(month()) + "-" + pad2(day()) + " " + pad2(hour()) + ":" + pad2(minute()) + ":" + pad2(second());
}

String pad2(int value) {
	return String(value < 10 ? "0" : "") + String(value);
}

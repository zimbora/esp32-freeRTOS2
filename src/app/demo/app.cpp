
#include "../../../package.h"
#ifdef DEMO

#include "app.h"

extern CALLS call;
extern SENSORS sensors;
extern SYSFILE sysfile;

app_settings app_s = {
  .fw = {
    /* version */   APP_VERSION,
    /* md5 */       ""
  },

  .rs485 = {
    /* active */    true,
    /* baudrate */  9600,
    /* config */    SERIAL_8N1
  }
};

void APP::init(){

  Serial.println("Init app DEMO module");

  load_settings();
  log_settings();

  if(app_s.rs485.active){
    sensors.rs485_init(&Serial1,RS485_GPIO_RX,RS485_GPIO_TX,RS485_GPIO_RTS);
    sensors.rs485_set_config(1,app_s.rs485.baudrate,app_s.rs485.config);
  }
}

void APP::loop(){

  /*
  if(now()%60 == 0 && now() > 1669632451){
    Serial.printf("timestamp: %lu \n",now());
    String filename = "/voltage.txt";
    String data = "231.12";
    if(!core_store_record(filename,data.c_str(),data.length()))
      Serial.println("failure storing record file");
    delay(1000);
  }
  */
}

/*
* if topic ends with '/get', it should be removed from topic avoiding loops
* if topic ends with '/set', an unpublish should be sent
*/
void APP::parse_mqtt_messages(uint8_t clientID, String topic, String payload){

  DBGLOG(Debug,"app topic: "+topic);
  DBGLOG(Debug,"payload: "+payload);
  String subtopic = "";
  bool set = false;
  bool get = false;
  bool store = false;

  if(topic.endsWith("/set")){
    set = true;
    if(payload == "")
      return;
  }

  if(topic.endsWith("/get")){
    get = true;
    uint16_t index = topic.lastIndexOf("/");
    subtopic = topic.substring(0,index); // filter get
  }

  switch(resolveOption(appTopics,topic)){
    case settings_reset_set_:
      reset_settings();
    case settings_rs485_get_:
    {
      String active = String(app_s.rs485.active);
      String baudrate = String(app_s.rs485.baudrate);
      String config = String(app_s.rs485.config);

      String payload = "{\"active\":"+active+",\"baudrate\":"+baudrate+",\"config\":"+config+"}";
      core_send_mqtt_message(clientID,subtopic,payload,1,false);
      break;
    }
    case settings_rs485_set_:
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
        app_s.rs485.active = (bool)active.toInt() != 0;
      }
      if(doc.containsKey("baudrate")){
        #ifndef UNITTEST
        String baudrate = doc["baudrate"];
        #else
        String baudrate = "";
        if(doc["baudrate"].is_number())
          baudrate = std::to_string((long)doc["baudrate"]);
        #endif
        app_s.rs485.baudrate = (long)baudrate.toInt();
      }

      if(doc.containsKey("config")){
        #ifndef UNITTEST
        String config = doc["config"];
        #else
        String config = "";
        if(doc["config"].is_number())
          config = std::to_string((long)doc["config"]);
        #endif
        app_s.rs485.config = (long)config.toInt();
      }

      // This instruction is crashing esp32. I don't understand why.
      // If the next instruction is commented a reboot is need for changes be applied
      //sensors.rs485_change_config(1,app_s.rs485.baudrate,app_s.rs485.config);

      break;
    }
    case rs485_read_get_:
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

        // push message
        core_send_mqtt_message(clientID,subtopic,data_str,1,false);

        free(data);
        //free(arr);
      }
      break;
    case rs485_write_get_:
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

      String data_str = "";
      if(error){
        data_str = "{\"error\": "+String(error)+"}";
      }else{
        data_str += "{\"res\":\"packet written\"}";
      }

      // push message
      core_send_mqtt_message(clientID,subtopic,data_str,1,false);
    }
      break;
  }

  if(store){
    store_settings();
    log_settings();
  }

  if(set)
    core_send_mqtt_message(clientID,topic,"",0,true); // unpublish

}


appTopics_ APP::resolveOption(std::map<long, appTopics_> map, String topic) {

  std::string topic_ = std::string(topic.c_str());
  long str_hash = (long)std::hash<std::string>{}(topic_);
  std::map<long,appTopics_>::iterator it;

  it = map.find(str_hash);
  if(it != map.end())
    return it->second;

  return app_not_found;
}

bool APP::load_settings(){
  uint16_t len = sizeof(app_s);
  Serial.printf("len:%d\n",len);
  Serial.printf("filename %s \n",APP_SETTINGS_FILENAME);
  //char* data = (char*)malloc(len);
  char data[1000];
  //if(data != nullptr){
    call.read_file(APP_SETTINGS_FILENAME,data,&len);
    memcpy(app_s.fw.version,data,sizeof(app_s.fw.version));
    String version = String(app_s.fw.version);
    DBGLOG(Info,"fw version: "+version);
    if(version.startsWith("0.") || version.startsWith("1.") || version.startsWith("2."))
      memcpy(app_s.fw.version,data,sizeof(app_s));
    else{
      String version = APP_VERSION;
      memcpy(app_s.fw.version,version.c_str(),version.length());
      store_settings();
    }
    //free(data);
  //}
  return true;
}


bool APP::store_settings(){

  Serial.printf("filename %s \n",APP_SETTINGS_FILENAME);
  if(!call.write_file(APP_SETTINGS_FILENAME,app_s.fw.version,sizeof(app_s))){
    DBGLOG(Error,"failing writing file: "+String(APP_SETTINGS_FILENAME));
    return false;
  }
  return true;
}

bool APP::reset_settings(){
  memset(app_s.fw.version,0,sizeof(app_s.fw.version));
  if(call.write_file(APP_SETTINGS_FILENAME,app_s.fw.version,sizeof(app_s)))
    call.fw_reboot();
}

void APP::log_settings(){

  DBGLOG(Debug,"fw.version: "+String(app_s.fw.version));
  DBGLOG(Debug,"fw.md5: "+String(app_s.fw.md5));

  DBGLOG(Debug,"rs485.active: "+String(app_s.rs485.active));
  DBGLOG(Debug,"rs485.baudrate: "+String(app_s.rs485.baudrate));
  DBGLOG(Debug,"rs485.config: "+String(app_s.rs485.config));

}

#endif


#include "app.h"

extern CALLS call;
extern SENSORS sensors;
extern SYSFILE sysfile;

app_settings app_s = {
  .fw = {
    /* version */   APP_VERSION,
    /* md5 */       ""
  },
};

void APP::init(){

  Serial.println("Init app DEMO module");

  load_settings();
  log_settings();

}

void APP::loop(){

  if(timeoutInfo < millis()){
    Serial.println("app is running");
    timeoutInfo += 5000;
  }

}

/*
* Function called from core.cpp on new message arrived
* if topic ends with '/get', it should be removed from topic avoiding loops
* if topic ends with '/set', an unpublish should be sent
*/
void APP::parse_mqtt_messages(uint8_t clientID, String topic, String payload){

  Serial.println("app topic: "+topic);
  Serial.println("payload: "+payload);
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
      break;
    // add your code here..
  }

  if(store){
    store_settings();
    log_settings();
  }

  if(set)
    core_send_mqtt_message(clientID,topic,"",2,true); // unpublish

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
  char data[1000];
  call.read_file(APP_SETTINGS_FILENAME,data,&len);
  memcpy(app_s.fw.version,data,sizeof(app_s.fw.version));
  String oldVersion = String(app_s.fw.version);
  String currentVersion = String(APP_VERSION);
  Serial.println("prev app.version: "+oldVersion);
  Serial.println("current app.version: "+currentVersion);
  if(currentVersion.startsWith("0.") || currentVersion.startsWith("1.") || currentVersion.startsWith("2.")){
    memcpy(app_s.fw.version,data,sizeof(app_s));
    memset(app_s.fw.version,0,sizeof(app_s.fw.version));
    memcpy(app_s.fw.version,currentVersion.c_str(),currentVersion.length());
  }else{
    Serial.println("resetting app settings..");
    memset(app_s.fw.version,0,sizeof(app_s.fw.version));
    memcpy(app_s.fw.version,currentVersion.c_str(),currentVersion.length());
  }
  store_settings();
  return true;
}


bool APP::store_settings(){

  Serial.printf("filename %s \n",APP_SETTINGS_FILENAME);
  if(!call.write_file(APP_SETTINGS_FILENAME,app_s.fw.version,sizeof(app_s))){
    Serial.println("failing writing file: "+String(APP_SETTINGS_FILENAME));
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

  Serial.println("app.md5: "+String(app_s.fw.md5));

}

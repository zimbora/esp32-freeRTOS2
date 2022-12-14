
#include "app.h"

extern CALLS call;

void APP::init(){

}

void APP::loop(){

  if(now()%60 == 0 && now() > 1669632451){
    Serial.printf("timestamp: %lu \n",now());
    String filename = "/voltage.txt";
    String data = "231.12";
    if(!core_store_record(filename,data.c_str(),data.length()))
      Serial.println("failure storing record file");
    delay(1000);
  }

}

/*
* This method is called from a task running in parallel with this class
* if topic ends with '/get', it should be removed from topic avoiding loops
* if topic ends with '/set', an unpublish should be done
*/
void APP::parse_mqtt_messages(uint8_t clientID, String topic, String payload){

  String subtopic = "";
  bool set = false;
  bool get = false;

  if(topic.endsWith("/set")){
    set = true;
    if(payload == "");
      return;
  }

  if(topic.endsWith("/get")){
    get = true;
    uint16_t index = topic.lastIndexOf("/");
    subtopic = topic.substring(0,index); // filter get
  }

  switch(resolveOption(appTopics,topic)){
    case rs485_read_:
      {
        Serial.println("rs485 read");
        // read rs485
        // push message
        String data = "1,3,4,0,0,0,1";
        core_send_mqtt_message(clientID,subtopic,data,1,false);
      }
      break;
    case rs485_write_:
      Serial.println("rs485 write");
      // write rs485
      // push message
      String data = "1,3,4";
      core_send_mqtt_message(clientID,subtopic,data,1,false);
      break;
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

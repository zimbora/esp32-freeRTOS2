
#include "calls.h"

/*
* !! All calls to sysfile must be done here
*/

extern MODEMfreeRTOS mRTOS;
extern SYSFILE sysfile;
extern SemaphoreHandle_t spiffsMutex;

HTTP_HEADER_MSG* msg_header;
HTTP_BODY_MSG* msg_body;

FOTA fota;
MD5Builder md5_;

String CALLS::fw_fota(String url){

  String protocol = "";
  if(url.startsWith("HTTPS://") || url.startsWith("https://")){
    protocol = "HTTPS";
    url = url.substring(8);
  }else if(url.startsWith("HTTP://") || url.startsWith("http://")){
    protocol = "HTTP";
    url = url.substring(7);
  }else{
    Serial.println("Invalid URL");
    return "Invalid URL";
  }

  int16_t index = url.indexOf("/");
  String host = url.substring(0,index);
  String path = url.substring(index);
  String method = "GET";
  String header_key = "";
  String header_value = "";
  String body = "";
  bool json = false;

  return do_fota(protocol,host,path,method,header_key,header_value,body,json);
}

/*
* !! check if md5 is being compared
*/
bool CALLS::fw_settings_update(String url, String filename){

  String protocol = "";
  if(url.startsWith("HTTPS://") || url.startsWith("https://")){
    protocol = "HTTPS";
    url = url.substring(8);
  }else if(url.startsWith("HTTP://") || url.startsWith("http://")){
    protocol = "HTTP";
    url = url.substring(7);
  }else{
    Serial.println("Invalid URL");
    return false;
  }

  int16_t index = url.indexOf("/");
  String host = url.substring(0,index);
  String path = url.substring(index);
  String method = "GET";
  String header_key = "";
  String header_value = "";
  String body = "";
  bool json = false;
  String json_str = do_request(protocol,host,path,method,header_key,header_value,body,json);

  Serial.println(json_str);

  if(!xSemaphoreTake( spiffsMutex, 5000)){
    //xSemaphoreGive(spiffsMutex);
    return false;
  }

  sysfile.write_file(filename.c_str(),json_str.c_str(),json_str.length());

  xSemaphoreGive(spiffsMutex);
}

String CALLS::get_file_md5(String filename){
  String md5_str = "";
  uint16_t len = 4096;
  char* data = (char*)malloc(len);
  if(data != nullptr){
    if(read_file(filename, data, &len)){
      md5_.begin();
      md5_.add((uint8_t*)data,len);
      md5_.calculate();
      md5_str = md5_.toString();
    }else md5_str = "";
  }
  return md5_str;
}

/*
* read file
*/
bool CALLS::read_file(String filename, char* data, uint16_t* len){

  if(!xSemaphoreTake( spiffsMutex, 100)){
     //xSemaphoreGive(spiffsMutex);
     return false;
  }

  bool res = sysfile.read_file(filename.c_str(),data,len);

  xSemaphoreGive(spiffsMutex);

  return res;
}

/*
* write file
*/
bool CALLS::write_file(String filename, const char* data, uint16_t len){

  if(!xSemaphoreTake( spiffsMutex, 2000)){
     //xSemaphoreGive(spiffsMutex);
     return false;
  }

  bool res = sysfile.write_file(filename.c_str(),data,len);

  xSemaphoreGive(spiffsMutex);

  return res;
}

/*
* clean records
*/
bool CALLS::delete_file(String filename){

  if(!xSemaphoreTake( spiffsMutex, 2000)){
     //xSemaphoreGive(spiffsMutex);
     return false;
  }

  bool res = sysfile.delete_file(filename.c_str());

  xSemaphoreGive(spiffsMutex);

  return res;
}

/*
* init directories
*/
bool CALLS::init_filesystem(String directory[], uint8_t len){

  if(!xSemaphoreTake( spiffsMutex, 2000)){
     xSemaphoreGive(spiffsMutex);
     return false;
  }

  // check filesystem
  uint8_t i = 0;
  Serial.printf("dir size: %d \n",len);
  for(uint8_t i=0; i < len; i++){
    if(!sysfile.create_dir(directory[i].c_str()))
      DBGLOG(Error,"-- create dir: "+ directory[i] +" has FAILED --");
  }

  // list filesystem
  sysfile.list_filesystem(5);

  xSemaphoreGive(spiffsMutex);

  return true;
}

/*
* init directories
*/
bool CALLS::create_dir(String directory){

  if(!xSemaphoreTake( spiffsMutex, 5000)){
     xSemaphoreGive(spiffsMutex);
     return false;
  }

  bool res = sysfile.create_dir(directory.c_str());

  xSemaphoreGive(spiffsMutex);

  if(!res)
    DBGLOG(Error,"-- create dir: "+ directory +" has FAILED --");

  return res;
}

/*
* Iterate over files, call callback after each iteration
*/
bool CALLS::check_filesystem_records(String dir, uint32_t timeout, bool (*callback)(String)){


  if(!xSemaphoreTake( spiffsMutex, 100)){
    //xSemaphoreGive(spiffsMutex);
    Serial.println("couldn't get semaphore");
    return false;
  }

  sysfile.iterateDir(dir.c_str(),timeout,callback);

  xSemaphoreGive( spiffsMutex );

  return true;
}

bool CALLS::store_record(String filename, const char* data, uint16_t len){

  if(!xSemaphoreTake( spiffsMutex, 5000)){
    xSemaphoreGive(spiffsMutex);
    return false;
  }

  sysfile.write_file(filename.c_str(),data,len);

  xSemaphoreGive( spiffsMutex );

  return true;

}

bool CALLS::remove_dir(String dir){

  if(!xSemaphoreTake( spiffsMutex, 100)){
    //xSemaphoreGive(spiffsMutex);
    return false;
  }

  bool res = sysfile.delete_dir(dir.c_str(),2000);

  xSemaphoreGive( spiffsMutex );

  return res;
}

void CALLS::clean_dir(String dir){

  if(!xSemaphoreTake( spiffsMutex, 100)){
    //xSemaphoreGive(spiffsMutex);
    return;
  }
  sysfile.deleteEmptySubDirectories(LittleFS,dir.c_str(),2000);

  xSemaphoreGive( spiffsMutex );
}

bool CALLS::fw_reboot(){
  ESP.restart();
}

bool CALLS::fw_reset(){

  if(!xSemaphoreTake( spiffsMutex, 5000)){
    xSemaphoreGive(spiffsMutex);
    return false;
  }

  sysfile.format();

  xSemaphoreGive( spiffsMutex );

  return true;
}

bool CALLS::mqtt_send(String topic, String data, uint8_t qos, bool retain){
  if(mRTOS.mqtt_isConnected(CLIENTIDEXTERNAL))
    return mqtt_send(CLIENTIDEXTERNAL,topic,data,qos,retain);
  else
    return mqtt_send(CLIENTID,topic,data,qos,retain);
}

bool CALLS::mqtt_send(uint8_t clientID, String topic, String data, uint8_t qos, bool retain){
  if(qos =! 2 && !mRTOS.mqtt_isConnected(clientID))
    return false;
  else
    return mRTOS.mqtt_pushMessage(clientID,topic,data,qos,retain);
}

void CALLS::check_alarms(){

}

void CALLS::check_sensors(){

}

String CALLS::do_request(String protocol, String host, String path, String method, String header_key, String header_value, String body, bool json){

  if(protocol == "HTTPS")
    mRTOS.https_pushMessage(CONTEXTID,CLIENTID,SSLCLIENTID,host,path,method,header_key,header_value,body,json);
  else if(protocol == "HTTP")
    mRTOS.http_pushMessage(CONTEXTID,CLIENTID,host,path,method);
  else return "";

  uint32_t timeout = millis() + 60000; // 30 seconds timeout
  while(timeout > millis()){

    msg_header = mRTOS.http_header_getNextMessage(msg_header);
    if(msg_header != NULL){
      Serial.printf("client [%d] %s \n",msg_header->clientID,msg_header->http_response.c_str());
      if(msg_header->http_response.indexOf("200") > 0){
        Serial.printf("http body len %d \n",msg_header->body_len);
        uint32_t len = 0;
        char* data = (char*)malloc(msg_header->body_len);
        while(msg_header->body_len != len  && timeout > millis()){

          msg_body = mRTOS.http_body_getNextMessage(msg_body);

          if(msg_body != NULL){

            if(data != nullptr){
              for(uint16_t i=0;i<msg_body->data_len;i++){
                data[len+i] = msg_body->data[i];
              }
            }

            len += msg_body->data_len;
            Serial.printf("http total bytes read of body data: %d \n",len);

          }
          delay(1); // use delay to moderate concurrency access to queues
        }
        Serial.println("http all data was read");

        String body = "";
        for(uint16_t i=0;i<len;i++){
          //Serial.print(data[i]);
          body += data[i];
        }

        //String body = String(data);
        free(data);
        return body;
      }
    }
    delay(1); // use delay to moderate concurrency access to queues
  }
  return "";
}

String CALLS::do_fota(String protocol, String host, String path, String method, String header_key, String header_value, String body, bool json){

  String error = "timeout";
  if(protocol == "HTTPS")
    mRTOS.https_pushMessage(CONTEXTID,CLIENTID,SSLCLIENTID,host,path,method,header_key,header_value,body,json);
  else if(protocol == "HTTP")
    mRTOS.http_pushMessage(CONTEXTID,CLIENTID,host,path,method);
  else return "protocol not known, use http or https";

  uint32_t timeout = millis() + 60000; // 30 seconds timeout
  uint32_t progressTimeout = 0; // 30 seconds timeout
  uint32_t fota_size;
  while(timeout > millis()){

    msg_header = mRTOS.http_header_getNextMessage(msg_header);
    if(msg_header != NULL){
      Serial.printf("client [%d] %s \n",msg_header->clientID,msg_header->http_response.c_str());
      if(msg_header->http_response.indexOf("200") >= 0){
        Serial.printf("http body len %lu \n",msg_header->body_len);
        fota_size = msg_header->body_len;
        fota.start(fota_size);
        md5_.begin();
        uint32_t len = 0;

        while(fota_size != len && timeout > millis()){

          msg_body = mRTOS.http_body_getNextMessage(msg_body);
          if(msg_body != NULL){
            //Serial.printf("http data len %lu \n",msg_body->data_len);
            timeout = millis()+10000;
            md5_.add((uint8_t*)msg_body->data,msg_body->data_len);
            
            int8_t tries = 3;
            while(!fota.write_block((uint8_t*)msg_body->data,msg_body->data_len)){
              delay(300);
              if(tries == 0)
                return "write flash tries exceed";
              tries--;
            }
            
            //Serial.println("heap free: " + String(ESP.getFreeHeap() / 1024) + " KiB");
            len += msg_body->data_len;
            uint32_t progress = len*100/fota_size;
            if( progressTimeout < millis()){
              Serial.printf("Progress: %d% \n",progress);
              progressTimeout = 2000 + millis();
            }
          }
          delay(1); // use delay to moderate concurrency access to queues
        }

        if(timeout < millis())
          return "transfer timeout";
        if(fota_size != len)
          return "fota size failed";

        Serial.println("http all data was read");

        md5_.calculate();
        String md5_calculated = md5_.toString();
        Serial.println("md5 calculated: "+md5_calculated);
        Serial.println("md5 header: "+msg_header->md5);
        if(msg_header->md5 == md5_calculated){
          Serial.println("md5 checked");
        }else{
          Serial.println("md5 check has failed");
          return "md5 not matched";
        } 

        if(fota.has_finished()){
          Serial.println("new fw uploaded");
          Serial.println("rebooting");
          fw_reboot();
        }

        return "fota done"; // unrecheable

      }else{
        Serial.println("fota request has failed");
        error = "http error: "+msg_header->http_response;
        return error;
      } 
    }
    delay(1); // use delay to moderate concurrency access to queues
  }
  return error;
}

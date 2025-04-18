
#include "sensors.h"

IT it_map[MAX_IT_SENSORS];
IO io_map[MAX_IOS_SENSORS];
RS485 rs485_map[MAX_RS485_SENSORS];
APP_SENSORS app_map[MAX_APP_SENSORS];

uint8_t rs485_map_len = 0;
uint8_t app_map_len = 0;

AUTOREQUEST Ar(&Serial);
ALARM Alarm(&Serial);
SENSORS sensors;

#ifndef UNITTEST
#ifdef ENABLE_RS485
ModbusRTU modbus;
#endif

DynamicJsonDocument table(512);
DynamicJsonDocument doc(2048);

#ifdef ENABLE_RS485
bool calledInRS485Autorequest(String ref){

  DBGLOG(Debug,"reading rs485 sensor \""+ref+"\"");
  return sensors.rs485_read(ref);
}
#endif

bool calledInAlarm(String ref){

  String value = table[ref];
  sensors.alarmTrigger(ref,value);
  //call.mqtt_send(topic,value,2,1);

  return true;
}

void SENSORS::alarmTrigger(String ref, String value){
  pSensorCallbacks->onAlarmTrigger(ref,value);
}

SENSORS::SENSORS(){}

void SENSORS::init(){
if(settings.uart2.active){
  #ifdef EXT_SERIAL_COMM
    DBGLOG(Debug,"initing Serial1 - assigned to uart2 struct");
    Serial1.begin(settings.uart2.baudrate, settings.uart2.config, SERIAL1_GPIO_RX, SERIAL1_GPIO_TX);
  #elif defined ENABLE_RS485
    sensors.rs485_init(&Serial1,SERIAL1_GPIO_RX,SERIAL1_GPIO_TX,SERIAL1_GPIO_RTS);
    sensors.rs485_set_config(1,settings.uart2.baudrate,settings.uart2.config);
  #endif
  }
}

// pass json data as ar
bool SENSORS::init_ar(String data){

  DeserializationError error = deserializeJson(doc, data);
  //serializeJson(doc,Serial);

  if(error){
    DBGLOG(Error,"Init ar Error parsing JSON");
    return false;
  }

  DBGLOG(Debug,"JSON usage: "+String(doc.memoryUsage()));

  uint8_t index = 0;

  if(!doc.containsKey("autorequests")){
    DBGLOG(Error,"autorequests key not present");
    return false;
  }

  if(doc["autorequests"].containsKey("rs485")){
    // extract the values
    #ifndef UNITTEST
    JsonArray array = doc["autorequests"]["rs485"].as<JsonArray>();

    for(JsonVariant v : array) {
      //Serial.println(v.as<char*>());
      JsonObject sensor = v.as<JsonObject>();

      if(sensor.containsKey("ref") && sensor.containsKey("type") && sensor.containsKey("modbus") && sensor.containsKey("period")){
        String ref = sensor["ref"];
        String type_str = sensor["type"];
        String modbus = sensor["modbus"];
        int16_t period = sensor["period"];
        #ifdef ENABLE_RS485
        rs485_add(index++,ref,modbus,type_str);
        Ar.add(ref,period);
        #endif
      }
    }
    #else
    nlohmann::json array = doc["autorequests"]["rs485"];
    for (nlohmann::json::iterator it = array.begin(); it != array.end(); ++it) {
      nlohmann::json sensor = *it;
      if(sensor.containsKey("ref") && sensor.containsKey("type") && sensor.containsKey("modbus") && sensor.containsKey("period")){
        String ref = sensor["ref"];
        String type_str = sensor["type"];
        nlohmann::json modbus_array = sensor["modbus"];
        std::string modbus = "";

        for (nlohmann::json::iterator it2 = modbus_array.begin(); it2 != modbus_array.end(); ++it2) {
          std::string number = "";
          if(it2->is_number()){
            number += std::to_string((long)*it2);
          }
          if(it2 == modbus_array.begin() || it2 == modbus_array.end())
          modbus += number;
          else
          modbus += ","+ number;
        }
        int16_t period = sensor["period"];

        if(!rs485_add(index++,ref,modbus,type_str)){
          DBGLOG(Debug,"couldn't add rs485 sensor");
        }

        if(!Ar.add(ref,period)){
          DBGLOG(Debug,"couldn't add autorequest");
        }
      }
    }
    #endif

    for(uint8_t j =0;j++;j<MAX_RS485_SENSORS){
      rs485_log(j);
    }
  }

  index = 0;
  if(doc["autorequests"].containsKey("app")){
    // extract the values
    #ifndef UNITTEST
    JsonArray array = doc["autorequests"]["app"].as<JsonArray>();

    for(JsonVariant v : array) {
      //Serial.println(v.as<char*>());
      JsonObject sensor = v.as<JsonObject>();

      if(sensor.containsKey("ref") && sensor.containsKey("type") && sensor.containsKey("period")){
        String ref = sensor["ref"];
        String type_str = sensor["type"];
        int16_t period = sensor["period"];
        uint8_t type = get_type(type_str);
        app_add(index++,ref,type);
        Ar.add(ref,period);
      }
    }
    #endif
  }

  return true;
}

bool SENSORS::init_alarm(String data){

  DeserializationError error = deserializeJson(doc, data);
  serializeJson(doc,Serial);
  Serial.println();

  if(error){
    DBGLOG(Error,"Alarm Error parsing JSON");
    return false;
  }

  DBGLOG(Debug,"JSON usage: "+String(doc.memoryUsage()));

  uint8_t index = 0;
  if(doc["alarms"] != "null"){
      // extract the values
      #ifndef UNITTEST
      JsonArray array = doc["alarms"].as<JsonArray>();
      for(JsonVariant v : array) {
        //Serial.println(v.as<char*>());
        JsonObject alarm_obj = v.as<JsonObject>();

        if(alarm_obj.containsKey("ref")){
          if(!Alarm.add(alarm_obj)){
            //String ref = String(alarm_obj["ref"]);
            Serial.println("alarm not added: ");
          }
        }
      }
      #else
      nlohmann::json array = nlohmann::json::array();
      array = doc["alarms"];
      for (nlohmann::json::iterator it = array.begin(); it != array.end(); ++it) {
        nlohmann::json object = *it;
        if(object.containsKey("ref"))
          Alarm.add(object);
      }
      #endif

      Alarm.list();
      Serial.println();
    }

  return true;
}


#else
extern nlohmann::json table;
#endif

void SENSORS::loop(){

  #ifdef ENABLE_RS485
  for(uint8_t i=0; i<rs485_map_len; i++){
    String ref = String(rs485_map[i].ref);
    if(Ar.check(ref,&calledInRS485Autorequest)){
      Serial.println("rs485 ar: \""+ref+ "\" executed");
      // store it if you want
      String value = table[ref];
      pSensorCallbacks->onReadSensor(ref,value);
    }

    if(Alarm.timedOut(ref)){
      Serial.println("check alarm: \""+ref+"\"");
      // read sensor
      calledInRS485Autorequest(ref);
      // update table
      JsonObject data = table.as<JsonObject>();
      if(Alarm.check(ref,rs485_map[i].type,data,calledInAlarm)){
      //if(Alarm.check(ref,rs485_map[i].type,data)){
        Serial.println("\""+ref+"\" is in alarm\n");
        String value = table[ref];
        pSensorCallbacks->onAlarmSensor(ref,value);
      }
    }
  }
  #endif


  for(uint8_t i=0; i<MAX_APP_SENSORS; i++){
    String ref = String(app_map[i].ref);
    if(Ar.check(ref)){
      Serial.println("app ar: \""+ref+ "\" executed");
      table[ref] = 0.0;
      JsonObject data = table.as<JsonObject>();
      pSensorCallbacks->getAppValue(data,ref);
      // store it if you want
      String value = table[ref];
      pSensorCallbacks->onReadSensor(ref,value);
    }

    if(Alarm.timedOut(ref)){
      Serial.println("check alarm: \""+ref+"\"");
      table[ref] = 0.0;
      JsonObject data = table.as<JsonObject>();
      pSensorCallbacks->getAppValue(data,ref);
      if(Alarm.check(ref,app_map[i].type,data,calledInAlarm)){
        Serial.println("\""+ref+"\" is in alarm\n");
        String value = table[ref];
        pSensorCallbacks->onAlarmSensor(ref,value);
      }
    }
  }

}

#ifdef ENABLE_RS485
  void SENSORS::rs485_init(HardwareSerial* port, uint8_t rx, uint8_t tx, uint8_t rts){

    DBGLOG(Debug,"initing rs485");
    modbus.setup(port,rx,tx,rts);
  }

  void SENSORS::rs485_set_config(uint8_t mode, uint32_t baudrate, uint32_t config, uint8_t retries){

    modbus.begin(mode,baudrate,config,retries);
  }

  void SENSORS::rs485_change_config(uint8_t mode, uint32_t baudrate, uint32_t config, uint8_t retries){

    modbus.change_config(mode,baudrate,config,retries);
  }


  bool SENSORS::rs485_add(uint8_t index, String ref_str, String modbus, String type_str){

    uint8_t type = get_type(type_str);

    int16_t len = 4;
    uint16_t data[len];

    if(parseArray(modbus,data,&len)){
      if(len < 4)
        return false;

      uint8_t i = 0;
      uint8_t unit_id = data[i++];
      uint8_t fc = data[i++];
      uint16_t address = data[i++];
      uint8_t len = data[i++];
      uint8_t* value = nullptr;
      //if(len > 4)
        //value = (uint8_t*)&data[i];

      return rs485_add(index,ref_str.c_str(),type,unit_id,fc,address,len,value);
    }
    return false;
  }

  bool SENSORS::rs485_add(uint8_t index, String ref, uint8_t type, uint8_t unit_id, uint8_t fc, uint16_t address, uint16_t len, uint8_t* value){

    if(index >= MAX_RS485_SENSORS)
      return false;

    rs485_map[index].type = type;
    rs485_map[index].unit_id = unit_id;
    rs485_map[index].fc = fc;
    rs485_map[index].address = address;
    rs485_map[index].len = len;

    uint8_t size = ref.length();
    if(size > MAX_SIZE_REF)
      size = MAX_SIZE_REF;
    memcpy(rs485_map[index].ref,ref.c_str(),size);

    rs485_map_len++;
    rs485_log(index);

    return true;
  }

  // --- RS485 ---
  bool SENSORS::rs485_read_all(){

    for(uint8_t i=0;i<rs485_map_len;i++){
      sensors.rs485_read(rs485_map[i].ref);
    }

    char content[255];
    memset(content,0,255);
    serializeJson(table, content);
    Serial.println(content);
    String data = String(content);
    pSensorCallbacks->onRS485ReadAll(data);

    return true;
  }

  void SENSORS::rs485_table_refresh(uint8_t i){

    String ref = rs485_map[i].ref;
    uint8_t* result = rs485_map[i].value;
    if(rs485_map[i].type == int16be_type){
      int16_t value = (result[0]<<8)|result[1];
      //Serial.printf("value: %d \n",value);
      table[ref] = value;
    }else if(rs485_map[i].type == uint16be_type){
      uint16_t value = (result[0]<<8)|result[1];
      //Serial.printf("value: %d \n",value);
      table[ref] = value;
    }else if(rs485_map[i].type == int32be_type){
      int32_t value = (result[0]<<24)|(result[1]<<16)|(result[2]<<8)|result[3];
      //Serial.printf("value: %l \n",value);
      table[ref] = value;
    }else if(rs485_map[i].type == uint32be_type){
      uint32_t value = (result[0]<<24)|(result[1]<<16)|(result[2]<<8)|result[3];
      //Serial.printf("value: %lu \n",value);
      table[ref] = value;
    }else if(rs485_map[i].type == floatbe_type){

      uint32_t aux = ((result[0]<<24) | (result[1]<<16) | (result[2]<<8) | result[3]);
      float value = *((float*)&aux);
      value = truncate(value,3);
      table[ref] = value;

    }else
      DBGLOG(Error,"table refresh - type not found");
  }

  uint8_t SENSORS::rs485_read(String ref){

    uint8_t i = 0;
    while(i<rs485_map_len){
      if(String(rs485_map[i].ref) == ref)
        break;
      i++;
    }

    return rs485_read(i);
  }

  uint8_t SENSORS::rs485_read(uint8_t index){

    uint16_t size = 32;
    uint8_t* data = (uint8_t*)malloc(size);

    uint8_t i = index;
    uint8_t error = rs485_read(rs485_map[i].unit_id,
      rs485_map[i].fc,
      rs485_map[i].address,
      rs485_map[i].len,
      data,
      &size);

    if(!error){
      uint8_t len = rs485_map[i].len*2;
      for(uint8_t j=0; j<len; j++){
        rs485_map[i].value[MAX_VALUE_LEN-j-1] = data[len-j-1];
      }
      Serial.println();
    }else{
      for(uint8_t j=0; j<MAX_VALUE_LEN; j++)
        rs485_map[i].value[MAX_VALUE_LEN-j-1] = 0;
    }
    rs485_table_refresh(i);

    return error;
  }

  uint8_t SENSORS::rs485_read(uint8_t unit_id, uint8_t fc, uint16_t address, uint16_t len, uint8_t* data, uint16_t* size){

    uint8_t error = modbus.rs485_read(unit_id,fc,address,len,data,size);

    if(error != 0){
      Serial.printf("error: 0x%x \n",error);
      String error_msg = modbus.getLastError();
      if(error_msg != "")
        Serial.println("error msg: "+error_msg);
    }

    return error;
  }

  uint8_t SENSORS::rs485_write(uint8_t unit_id, uint8_t fc, uint16_t address, uint16_t len, uint8_t* data, uint16_t* size){

    Serial.println("Writing rs485..");
    uint8_t error = modbus.rs485_write(unit_id,fc,address,len,data,size);
    if(error == 0){
      Serial.println("packet written successfully");
    }else{
      Serial.printf("error: 0x%x \n",error);
      String error_msg = modbus.getLastError();
      if(error_msg != "")
        Serial.println("error msg: "+error_msg);
    }

    return error;
  }

  void SENSORS::rs485_log(uint8_t index){
    #ifndef UNITTEST
    DBGLOG(Debug,"ref: "+String(rs485_map[index].ref));
    DBGLOG(Debug,"type: "+String(rs485_map[index].type));
    DBGLOG(Debug,"unit_id: "+String(rs485_map[index].unit_id));
    DBGLOG(Debug,"fc: "+String(rs485_map[index].fc));
    DBGLOG(Debug,"address: "+String(rs485_map[index].address));
    DBGLOG(Debug,"len: "+String(rs485_map[index].len));
    //DBGLOG(Debug,"value: "+String(rs485_map[index].value,MAX_VALUE_LEN));
    #endif
  }
#endif


bool SENSORS::app_add(uint8_t index, String ref, uint8_t type){

  if(index >= MAX_APP_SENSORS)
    return false;

  app_map[index].type = type;
  uint8_t size = ref.length();
  if(size > MAX_SIZE_REF)
    size = MAX_SIZE_REF;
  memcpy(app_map[index].ref,ref.c_str(),size);

  app_map_len++;
  app_log(index);

  return true;
}


void SENSORS::app_log(uint8_t index){
  #ifndef UNITTEST
  DBGLOG(Debug,"ref: "+String(app_map[index].ref));
  DBGLOG(Debug,"type: "+String(app_map[index].type));
  #endif
}

// --- --- ---

int SENSORS::get_type(String type){

  #ifndef UNITTEST
  type.toUpperCase();
  #else
  std::transform(type.begin(), type.end(),type.begin(), ::toupper);
  #endif

  if(type == "INT8")
    return int8_type;
  else if(type == "UINT8")
    return uint8_type;
  else if(type == "INT16BE" || type == "INT16")
    return int16be_type;
  else if(type == "INT16LE")
    return int16le_type;
  else if(type == "UINT16BE" || type == "UINT16")
    return uint16be_type;
  else if(type == "UINT16LE")
    return uint16le_type;
  else if(type == "INT32BE" || type == "INT32")
    return int32be_type;
  else if(type == "INT32LE")
    return int32le_type;
  else if(type == "UINT32BE" || type == "UINT32")
    return uint32be_type;
  else if(type == "UINT32LE")
    return uint32le_type;
  else if(type == "FLOATBE" || type == "FLOAT")
    return floatbe_type;
  else if(type == "FLOATLE")
    return floatle_type;
  else if(type == "HEX*")
    return hex_arr;
  else if(type == "CHAR*")
    return char_arr;
  else return -1;

}

float SENSORS::truncate(float val, byte dec) {
    float x = val * pow(10, dec);
    float y = round(x);
    float z = x - y;
    if ((int)z == 5)
    {
        y++;
    } else {}
    x = y / pow(10, dec);
    return x;
}

bool SENSORS::parseArray(String array, uint16_t* arr, int16_t* len){
#ifndef UNITTEST
  int16_t index = -1;
  uint8_t size = 0;
  String value = "";

  array.replace("[","");
  array.replace("]","");

  while(size < *len){
    // !! [1970-00-00 00:00:00] 6,2
    index = array.indexOf(",");
    if(index == -1){
      value = array;
      if(value.length() > 0 && has_only_digits(value))
        arr[size++] = (uint16_t)value.toInt();
      else{
        DBGLOG(Debug,"error parsing string array: "+String(value));
        return false;
      }
      break;
    }else{
      value = array.substring(0,index);
      if(value.length() > 0 && has_only_digits(value))
        arr[size++] = (uint16_t)value.toInt();
      else{
        DBGLOG(Debug,"error parsing string array: "+String(value));
        return false;
      }

      array = array.substring(index+1);
    }
  }

  *len = size;
#else
  int16_t index = -1;
  uint8_t size = 0;
  std::string value = "";

  array.erase(std::remove(array.begin(), array.end(), '\"'), array.end());
  array.erase(std::remove(array.begin(), array.end(), '\n'), array.end());

  while(size < *len){

    index = array.find(",");
    if(index == -1){
      value = std::string(array);
      if(value.size() > 0 && has_only_digits(value))
        arr[size++] = std::stoull(value);

      break;
    }else{
      //value = string(array.substr(0,index))+'\r'; // '\r' too important !!
      value = array.substr(0,index); // '\r' too important !!
      if(value.size() > 0 && has_only_digits(value))
        arr[size++] = std::stoull(value);

      array = array.substr(index+1);
    }
  }
  *len = size;
  return true;
#endif

  return true;
}

bool SENSORS::has_only_digits(String value_str){

  #ifndef UNITTEST
  unsigned int stringLength = value_str.length();

  if (stringLength < 1)
    return false;

  for(unsigned int i = 0; i < stringLength; ++i) {
    if (isDigit(value_str.charAt(i)))
      continue;
    
    return false;
  }
  #else
    if(value_str.length() == 0)
      return false;
    return (value_str.find_first_not_of( "0123456789" ) == std::string::npos);
  #endif 
  return true;
}

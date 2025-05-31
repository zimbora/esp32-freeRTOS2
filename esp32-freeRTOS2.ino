#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

// HARDWARE
#define PWKEY 4

#include <rom/rtc.h>

#include "./package.h"
#include "core.h"
#include "modem-freeRTOS.hpp"
#include "./src/sensors/sensors.h"
#include "./src/app/user/app.h"

#ifdef ENABLE_AP
  #include "./src/wifi/wifiAP.h"
#endif
#ifdef ENABLE_BLE
  #include "./src/ble/ble_server.h"
#endif

TaskHandle_t MRTOS;

String get_reset_reason(int reason);

extern APP app;

// MQTT
String mqtt_subscribe_topics[] = {
  "/status",
  "/fw/#",
  "/app/#"
};

extern MODEMfreeRTOS mRTOS;

#ifdef ENABLE_AP
  WIFIAP ap;
#endif
#ifdef ENABLE_BLE
  BLE_SERVER ble;
#endif

extern SemaphoreHandle_t spiffsMutex;
extern SYSFILE sysfile;

void (*callback)(uint8_t);
void mqttOnConnect(uint8_t clientID){
  DBGLOG(Debug,"mqtt with clientID: "+String(clientID)+" is connected - sending first message");
  mRTOS.mqtt_pushMessage(clientID,"/status","online",2,true);
  mRTOS.mqtt_pushMessage(clientID,"/model",String(FW_MODEL),2,true);
  mRTOS.mqtt_pushMessage(clientID,"/fw_version",String(FW_VERSION),2,true);
  mRTOS.mqtt_pushMessage(clientID,"/app_version",String(APP_VERSION),2,true);
  mRTOS.mqtt_pushMessage(clientID,"/tech",mRTOS.get_technology(),2,true);
  mRTOS.mqtt_pushMessage(clientID,"/reboot_cause_cpu0",get_reset_reason(rtc_get_reset_reason(0)),2,true);
  mRTOS.mqtt_pushMessage(clientID,"/reboot_cause_cpu1",get_reset_reason(rtc_get_reset_reason(1)),2,true);

  mRTOS.mqtt_subscribeTopics(clientID);
  return;
}

// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
// This function is called once client 1 is connected (MQTT-WIFI)
void onConnectionEstablished(){
  DBGLOG(Debug,"mqtt client 1 is connected - sending first message");

  mRTOS.mqtt_pushMessage(CLIENTID,"/status","online",2,true);
  mRTOS.mqtt_pushMessage(CLIENTID,"/model",String(FW_MODEL),2,true);
  mRTOS.mqtt_pushMessage(CLIENTID,"/fw_version",String(FW_VERSION),2,true);
  mRTOS.mqtt_pushMessage(CLIENTID,"/app_version",String(APP_VERSION),2,true);
  mRTOS.mqtt_pushMessage(CLIENTID,"/tech",mRTOS.get_technology(),2,true);
  mRTOS.mqtt_pushMessage(CLIENTID,"/reboot_cause_cpu0",get_reset_reason(rtc_get_reset_reason(0)),2,true);
  mRTOS.mqtt_pushMessage(CLIENTID,"/reboot_cause_cpu1",get_reset_reason(rtc_get_reset_reason(1)),2,true);

  mRTOS.mqtt_subscribeTopics(0);
}

// This function is called once client 2 is connected (MQTT-WIFI)
void onConnectionEstablished2(){
  DBGLOG(Debug,"mqtt client 2 is connected - sending first message");
  if(!mRTOS.mqtt_pushMessage(CLIENTIDEXTERNAL,"/status","online",2,true))
    DBGLOG(Debug,"!! status message not sent for client CLIENTIDEXTERNAL");

  mRTOS.mqtt_subscribeTopics(1);
}

#ifdef ENABLE_LTE
  void network_lte_task(void *pvParameters);
  void network_lte_task(void *pvParameters){
    (void) pvParameters;

    Serial.println("Initing LTE task..");
    // radio
    mRTOS.init(settings.modem.cops,settings.modem.tech,PWKEY); // initialize modem

    mRTOS.set_ssl(CONTEXTID);
    mRTOS.set_context(CONTEXTID,settings.modem.apn,settings.modem.user,settings.modem.pwd);

    // --- MQTT configuration ---

    // Own cloud
    String preTopic = String(MQTT_PROJECT);
    mRTOS.mqtt_configure_connection(CLIENTID,CONTEXTID,preTopic.c_str(),get_uid(),MQTT_HOST_1,1883,MQTT_USER_1,MQTT_PASSWORD_1);
    mRTOS.mqtt_set_will_topic(CLIENTID,MQTT_WILL_SUBTOPIC,MQTT_WILL_PAYLOAD);
    uint8_t i = 0;
    while(i<NUMITEMS(mqtt_subscribe_topics)){
      mRTOS.mqtt_add_subscribe_topic(CLIENTID,i,mqtt_subscribe_topics[i]);
      i++;
    }

    // External cloud
    if(settings.mqtt.active){
      String host = String(settings.mqtt.host);
      String user = String(settings.mqtt.user);
      String pass = String(settings.mqtt.pass);
      mRTOS.mqtt_configure_connection(CLIENTIDEXTERNAL,CONTEXTID,preTopic.c_str(),get_uid(),host,settings.mqtt.port,user,pass);
      mRTOS.mqtt_set_will_topic(CLIENTIDEXTERNAL,MQTT_WILL_SUBTOPIC,MQTT_WILL_PAYLOAD);
      uint8_t i = 0;
      while(i<NUMITEMS(mqtt_subscribe_topics)){
        mRTOS.mqtt_add_subscribe_topic(CLIENTIDEXTERNAL,i,mqtt_subscribe_topics[i]);
        i++;
      }
    }

    callback = &mqttOnConnect;
    mRTOS.mqtt_setup(callback);
    // --- --- ---

    for(;;){
      mRTOS.loop();
      delay(1); // !! do not remove - switching between tasks
    }

  }
#endif

#ifdef THREAD_APP
  void app_task(void *pvParameters);
  void app_task(void *pvParameters){
  (void) pvParameters;

    Serial.println("Initing APP task..");

    app.init();

    for(;;){
      app.loop();
      delay(1); // !! do not remove - switching between tasks
    }
  }
#endif

void core_task(void *pvParameters);
void core_task(void *pvParameters){
  (void) pvParameters;

  Serial.println("Initing CORE task..");

  core_init();
      
  #ifdef ENABLE_AP
    ap.setCallbacks(new CALLBACKS_WIFI_AP());
  #endif

  for(;;){
    core_loop();
    delay(1); // !! do not remove - switching between tasks
  }
}

#ifndef ENABLE_LTE
void mRTOS_task(void *pvParameters);
void mRTOS_task(void *pvParameters){
  (void) pvParameters;

  Serial.println("Initing mRTOS task..");

  Serial.println("ssid: "+String(settings.wifi.ssid));
  mRTOS.init(settings.wifi.ssid,settings.wifi.pwd);
  String uid = MQTT_UID_PREFIX+mRTOS.macAddress();

  String preTopic = String(MQTT_PROJECT);
  
  mRTOS.mqtt_configure_connection(CLIENTID,preTopic.c_str(),uid.c_str(),MQTT_HOST_1,MQTT_PORT_1,MQTT_USER_1,MQTT_PASSWORD_1);
  mRTOS.mqtt_set_will_topic(CLIENTID,MQTT_WILL_SUBTOPIC,MQTT_WILL_PAYLOAD);
  DBGLOG(Debug,"mqtt client 1 configured");

  for(uint8_t i=0;i<NUMITEMS(mqtt_subscribe_topics);i++){
    mRTOS.mqtt_add_subscribe_topic(CLIENTID,i,mqtt_subscribe_topics[i]);
  }
  
  mRTOS.mqtt_wifi_setup(CLIENTID,onConnectionEstablished);

  // External cloud
  if(settings.mqtt.active){
    String host = String(settings.mqtt2.host);
    String user = String(settings.mqtt2.user);
    String pass = String(settings.mqtt2.pass);
    mRTOS.mqtt_set_will_topic(CLIENTIDEXTERNAL,MQTT_WILL_SUBTOPIC,MQTT_WILL_PAYLOAD);
    mRTOS.mqtt_configure_connection(CLIENTIDEXTERNAL,preTopic.c_str(),uid.c_str(),host.c_str(),settings.mqtt2.port,user.c_str(),pass.c_str());
    DBGLOG(Debug,"mqtt client 2 configured");
    for(uint8_t i=0;i<NUMITEMS(mqtt_subscribe_topics);i++){
      mRTOS.mqtt_add_subscribe_topic(CLIENTIDEXTERNAL,i,mqtt_subscribe_topics[i]);
    }
    mRTOS.mqtt_wifi_setup(CLIENTIDEXTERNAL,onConnectionEstablished2);
  }
  
  #ifdef ENABLE_LTE
    Serial.println("waiting for modem to register on network..");
    while(!mRTOS.isLTERegistered()) delay(100);
    Serial.println("modem is registered");
  #endif

  bool wifiDefault = false;
  uint32_t wifiTimeout = 0;
  for(;;){
    
    mRTOS.loop();

#ifndef ENABLE_LTE
    if(!mRTOS.isWifiConnected() && wifiTimeout < millis()){
      #ifndef ENABLE_AP
        if(wifiDefault){
          mRTOS.wifiReconnect(WIFI_SSID,WIFI_PASSWORD);
          wifiDefault = false;
        }else{
          mRTOS.wifiReconnect(settings.wifi.ssid,settings.wifi.pwd);
          wifiDefault = true;
        }
        wifiTimeout = millis() + 30*1000;
      #else ENABLE_AP
        ap.setup();
        DBGLOG(Debug,now());
        uint32_t timeout = now() + 300;
        for(;;){
          ap.loop();

          if(timeout < now()){
            DBGLOG(Info,"Timeout for Access Point, try WiFi client once again");
            break;
          }
        }
        mRTOS.wifiReconnect(settings.wifi.ssid,settings.wifi.pwd);
        wifiTimeout = millis() + 30*1000;
      #endif
    }
    delay(1); // !! do not remove - switching between tasks
#endif
  }
}
#endif

void setup() {

  Serial.begin(115200);

  DBGINI(&Serial,ESP32Timestamp::TimestampNone);
  DBGSTA
  DBGLEV(Debug);
  DBGLOG(Info,"Initing program..");

  spiffsMutex = xSemaphoreCreateMutex();

  DBGLOG(Info,"initing file system..");
  sysfile.init();

  delay(500);
  DBGLOG(Info,"loading settings..");
  core_load_settings();

  DBGLOG(Info,"logging settings..");
  delay(500);
  settings_log();

  DBGLOG(Info,"wait 2s for system to init..");
  delay(500);

  
  xTaskCreatePinnedToCore(
    core_task
    ,  "core_task"   // A name just for humans
    ,  NETWORK_CORE_TASK_SIZE  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  NETWORK_CORE_TASK_PRIORITY // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.  !! do not edit priority
    ,  &MRTOS
    ,  1);
  
  #ifndef ENABLE_LTE
    xTaskCreatePinnedToCore(
      mRTOS_task
      ,  "mRTOS_task"   // A name just for humans
      ,  MRTOS_TASK_SIZE  // This stack size can be checked & adjusted by reading the Stack Highwater
      ,  NULL
      ,  MRTOS_TASK_PRIORITY // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.  !! do not edit priority
      ,  NULL
      ,  1);
  #endif

  #ifdef ENABLE_LTE
    xTaskCreatePinnedToCore(
        network_lte_task
        ,  "network_lte_task"   // A name just for humans
        ,  NETWORK_LTE_TASK_SIZE  // This stack size can be checked & adjusted by reading the Stack Highwater
        ,  NULL
        ,  NETWORK_LTE_TASK_PRIORITY // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest. !! do not edit priority
        ,  NULL
        ,  1);
  #endif

  #ifdef THREAD_APP
    xTaskCreatePinnedToCore(
        app_task
        ,  "app_task"   // A name just for humans
        ,  NETWORK_APP_TASK_SIZE  // This stack size can be checked & adjusted by reading the Stack Highwater
        ,  NULL
        ,  NETWORK_APP_TASK_PRIORITY // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest. !! do not edit priority
        ,  NULL
        ,  1);
  #endif

  #ifdef ENABLE_BLE
    ble.init(uid);
    ble.setCallback(&bleCallback);
    ble.enable();
  #endif

  #ifdef FAST_APP
    delay(3000); // wait for app to initialize
    app.init();
  #endif  

}

void loop() {
  
  #ifdef FAST_APP
    app.loop();
  #endif

  delay(1);
}

String get_reset_reason(int reason){
  switch ( reason)
  {
    case 1  : return "Vbat power on reset"; break;
    case 3  : return "Software reset digital core"; break;
    case 4  : return "Legacy watch dog reset digital core"; break;
    case 5  : return "Deep Sleep reset digital core"; break;
    case 6  : return "Reset by SLC module, reset digital core"; break;
    case 7  : return "Timer Group0 Watch dog reset digital core"; break;
    case 8  : return "Timer Group1 Watch dog reset digital core"; break;
    case 9  : return "RTC Watch dog Reset digital core"; break;
    case 10 : return "Instrusion tested to reset CPU"; break;
    case 11 : return "Time Group reset CPU"; break;
    case 12 : return "Software reset CPU"; break;
    case 13 : return "RTC Watch dog Reset CPU"; break;
    case 14 : return "for APP CPU, reseted by PRO CPU"; break;
    case 15 : return "Reset when the vdd voltage is not stable"; break;
    case 16 : return "RTC Watch dog reset digital core and rtc module"; break;
    default : return "NO_MEAN";
  }
}

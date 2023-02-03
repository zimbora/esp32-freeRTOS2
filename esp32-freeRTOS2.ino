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

#ifdef ENABLE_AP
#include "./src/wifi/wifiAP.h"
#endif
#ifdef ENABLE_BLE
#include "./src/ble/ble_server.h"
#endif

String get_reset_reason(int reason);

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



void (*callback)(uint8_t);
void mqttOnConnect(uint8_t clientID){
  DBGLOG(Debug,"mqtt with clientID: "+String(clientID)+" is connected - sending first message");
  mRTOS.mqtt_pushMessage(clientID,"/status","online",2,true);
  mRTOS.mqtt_pushMessage(clientID,"/model",String(FW_MODEL),2,true);
  mRTOS.mqtt_pushMessage(clientID,"/fw_version",String(FW_VERSION),2,true);
  mRTOS.mqtt_pushMessage(clientID,"/app_version",String(APP_VERSION),2,true);
  mRTOS.mqtt_pushMessage(clientID,"/uptime",String(millis()/1000),2,true);
  mRTOS.mqtt_pushMessage(clientID,"/reboot_cause_cpu0",get_reset_reason(rtc_get_reset_reason(0)),2,true);
  //mRTOS.mqtt_pushMessage(clientID,"/reboot_cause_cpu1",get_reset_reason(rtc_get_reset_reason(1)),2,true);

  mRTOS.mqtt_subscribeTopics(clientID);
  return;
}

// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished(){
  Serial.println("mqtt client 0 is connected - sending first message");

  mRTOS.mqtt_pushMessage(0,"/status","online",2,true);
  mRTOS.mqtt_pushMessage(0,"/model",String(FW_MODEL),2,true);
  mRTOS.mqtt_pushMessage(0,"/fw_version",String(FW_VERSION),2,true);
  mRTOS.mqtt_pushMessage(0,"/app_version",String(APP_VERSION),2,true);
  mRTOS.mqtt_pushMessage(0,"/uptime",String(millis()/1000),2,true);
  mRTOS.mqtt_pushMessage(0,"/reboot_cause_cpu0",get_reset_reason(rtc_get_reset_reason(0)),2,true);
  mRTOS.mqtt_pushMessage(0,"/reboot_cause_cpu1",get_reset_reason(rtc_get_reset_reason(1)),2,true);


  mRTOS.mqtt_subscribeTopics(0);
}

void onConnectionEstablished2(){
  Serial.println("mqtt client 2 is connected - sending first message");
  mRTOS.mqtt_pushMessage(1,"/status","online",2,true);

  mRTOS.mqtt_subscribeTopics(1);
}

#ifdef ENABLE_LTE
void network_lte_task(void *pvParameters);
void network_lte_task(void *pvParameters){
  (void) pvParameters;

  DBGLOG(Info,"Preparing LTE modem..");
  // radio
  mRTOS.init(SETTINGS_NB_COPS,GPRS,PWKEY); // initialize modem

  mRTOS.set_ssl(CONTEXTID);
  mRTOS.set_context(CONTEXTID,SETTINGS_NB_APN,SETTINGS_NB_USERNAME,SETTINGS_NB_PASSWORD);

  // --- MQTT configuration ---

  // Own cloud
  mRTOS.mqtt_configure_connection(CLIENTID,CONTEXTID,MQTT_PROJECT,get_uid(),MQTT_HOST_1,1883,MQTT_USER_1,MQTT_PASSWORD_1);
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
    mRTOS.mqtt_configure_connection(CLIENTIDEXTERNAL,CONTEXTID,MQTT_PROJECT,get_uid(),host,settings.mqtt.port,user,pass);
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
  }

}
#endif

void core_task(void *pvParameters);
void core_task(void *pvParameters){
  (void) pvParameters;

  core_init();

  for(;;){
    core_loop();
  }
}

void setup() {

  Serial.begin(115200);

  DBGINI(&Serial,ESP32Timestamp::TimestampNone);
  DBGSTA
  DBGLEV(Debug);
  DBGLOG(Info,"Initing program..");


  xTaskCreatePinnedToCore(
    core_task
    ,  "core_task"   // A name just for humans
    ,  2048*8  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.  !! do not edit priority
    ,  NULL
    ,  1);

  //core_init();

  Serial.println("wait 5s for system to init..");
  delay(5000);

  #ifdef ENABLE_LTE
    xTaskCreatePinnedToCore(
        network_lte_task
        ,  "network_lte_task"   // A name just for humans
        ,  2048*4  // This stack size can be checked & adjusted by reading the Stack Highwater
        ,  NULL
        ,  2 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest. !! do not edit priority
        ,  NULL
        ,  1);
  #else
    ap.setCallbacks(new CALLBACKS_WIFI_AP());
    while(!mRTOS.isWifiConnected()){

      if(settings.wifi.ssid != ""){
        Serial.printf("connecting wifi to %s \n",settings.wifi.ssid);
        mRTOS.init(settings.wifi.ssid,settings.wifi.pwd);
        uint32_t timeout = millis()+15000; // 15 seconds
        while(timeout > millis() && !mRTOS.isWifiConnected()){
          delay(1000);
          Serial.print(".");
        }
      }

      if(!mRTOS.isWifiConnected()){
        ap.setup();
        Serial.println(now());
        uint32_t timeout = now() + 300;
        for(;;){
          ap.loop();

          if(timeout < now()){
            Serial.println("Timeout for Access Point, try WiFi client once again");
            break;
          }

        }
      }
    }

  #endif

  #ifndef ENABLE_LTE

    const char project[] = MQTT_PROJECT;
    uint16_t port = 1883;

    mRTOS.mqtt_set_will_topic(CLIENTID,MQTT_WILL_SUBTOPIC,MQTT_WILL_PAYLOAD);
    mRTOS.mqtt_configure_connection(0,project,get_uid().c_str(),MQTT_HOST_1,port,MQTT_USER_1,MQTT_PASSWORD_1);
    Serial.println("mqtt client configured");


    for(uint8_t i=0;i<NUMITEMS(mqtt_subscribe_topics);i++){
      mRTOS.mqtt_add_subscribe_topic(0,i,mqtt_subscribe_topics[i]);
    }

    // External cloud
    if(settings.mqtt.active){
      String host = String(settings.mqtt.host);
      String user = String(settings.mqtt.user);
      String pass = String(settings.mqtt.pass);
      mRTOS.mqtt_set_will_topic(CLIENTIDEXTERNAL,MQTT_WILL_SUBTOPIC,MQTT_WILL_PAYLOAD);
      mRTOS.mqtt_configure_connection(1,project,get_uid().c_str(),host.c_str(),settings.mqtt.port,user.c_str(),pass.c_str());
      Serial.println("mqtt client configured");
      for(uint8_t i=0;i<NUMITEMS(mqtt_subscribe_topics);i++){
        mRTOS.mqtt_add_subscribe_topic(1,i,mqtt_subscribe_topics[i]);
      }
    }
    mRTOS.mqtt_wifi_setup(onConnectionEstablished2);

  #endif

  #ifdef ENABLE_BLE
  ble.init();
  ble.enable();
  #endif

  //app.init();

}


void loop() {

  #ifndef ENABLE_LTE
  mRTOS.loop();
  #endif

  //core_loop();
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

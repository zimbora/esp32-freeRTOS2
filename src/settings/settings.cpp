
#include "settings.h"
#include "../app/user/credentials.h"
#include "../../package.h"

user_settings settings = {

  .fw = {
    /* version */     FW_VERSION,
    /* md5 */         ""
  },

  .wifi = {
    /* mode */        "",
    /* ssid */        WIFI_SSID,
    /* pwd */         WIFI_PASSWORD
  },

  .modem = {
    /* apn */         "",
    /* user */        "",
    /* pwd */         "",
    /* band */        0,
    /* cops */        0,
    /* tech */        0 // radio technology GSM:1/GPRS:2/NB:3/CATM1:4/AUTO:5
  },

  .mqtt = {
    /* host */        MQTT_HOST_1,
    /* user */        MQTT_USER_1,
    /* pass */        MQTT_PASSWORD_1,
    /* port */        MQTT_PORT_1,
    /* active */      true
  },

  .mqtt2 = {
    /* host */        MQTT_HOST_2,
    /* user */        MQTT_USER_2,
    /* pass */        MQTT_PASSWORD_2,
    /* port */        MQTT_PORT_2,
    /* active */      MQTT_ACTIVE_2
  },

  .log = {
    /* active */      LOG_ACTIVE,
    /* level */       LOG_LEVEL
  },

  .keepalive = {
    /* active */      KEEPALIVE_ACTIVE,
    /* period */      KEEPALIVE_PERIOD // in seconds
  },

  .uart2 = {
    /* active */    UART2_ACTIVE,
    /* baudrate */  UART2_BAUDRATE,
    /* config */    UART2_CONFIG
  }
};

bool settings_set_param(String param, String value){

    switch(resolveOptionSettings(settingsTopics,param)){
      case wifi_mode:
        if(value.length() <= sizeof(settings.wifi.mode)){
          memset(settings.wifi.mode,0,sizeof(settings.wifi.mode));
          memcpy(settings.wifi.mode,value.c_str(),value.length());
          return true;
        }
        break;
      case wifi_ssid:
        if(value.length() <= sizeof(settings.wifi.ssid)){
          memset(settings.wifi.ssid,0,sizeof(settings.wifi.ssid));
          memcpy(settings.wifi.ssid,value.c_str(),value.length());
          return true;
        }
        break;
      case wifi_pwd:
        if(value.length() <= sizeof(settings.wifi.pwd)){
          memset(settings.wifi.pwd,0,sizeof(settings.wifi.pwd));
          memcpy(settings.wifi.pwd,value.c_str(),value.length());
          return true;
        }
        break;

      case modem_apn:
        if(value.length() <= sizeof(settings.modem.apn)){
          memset(settings.modem.apn,0,sizeof(settings.modem.apn));
          memcpy(settings.modem.apn,value.c_str(),value.length());
          return true;
        }
        break;
      case modem_user:
        if(value.length() <= sizeof(settings.modem.user)){
          memset(settings.modem.user,0,sizeof(settings.modem.user));
          memcpy(settings.modem.user,value.c_str(),value.length());
          return true;
        }
        break;
      case modem_pwd:
        if(value.length() <= sizeof(settings.modem.pwd)){
          memset(settings.modem.pwd,0,sizeof(settings.modem.pwd));
          memcpy(settings.modem.pwd,value.c_str(),value.length());
          return true;
        }
        break;
      case modem_band:
        if(value != "" && has_only_digits(value)){
          settings.modem.band = (uint8_t)stoLong(value);
          return true;
        }
        break;
      case modem_cops:
        if(value != "" && has_only_digits(value)){
          settings.modem.cops = (uint8_t)stoLong(value);
          return true;
        }
        break;
      case modem_tech:
        if(value != "" && has_only_digits(value)){
          settings.modem.tech = (uint8_t)stoLong(value);
          return true;
        }
        break;
      case mqtt_host:
        if(value.length() <= sizeof(settings.mqtt2.host)){
          memset(settings.mqtt2.host,0,sizeof(settings.mqtt2.host));
          memcpy(settings.mqtt2.host,value.c_str(),value.length());
          return true;
        }
        break;
      case mqtt_user:
        if(value.length() <= sizeof(settings.mqtt2.user)){
          memset(settings.mqtt2.user,0,sizeof(settings.mqtt2.user));
          memcpy(settings.mqtt2.user,value.c_str(),value.length());
          return true;
        }
        break;
      case mqtt_pass:
        if(value.length() <= sizeof(settings.mqtt2.pass)){
          memset(settings.mqtt2.pass,0,sizeof(settings.mqtt2.pass));
          memcpy(settings.mqtt2.pass,value.c_str(),value.length());
          return true;
        }
        break;
      case mqtt_port:
        if(value != "" && has_only_digits(value)){
          settings.mqtt2.port = (uint16_t)stoLong(value);
          return true;
        }
        break;
      case mqtt_active:
        if(value != "" && has_only_digits(value)){
          long active_ = stoLong(value);
          if(active_ == 0 || active_ == 1){
            settings.mqtt2.active = (bool)active_;
            return true;
          }
        }
        break;

      case log_level:
        if(value != "" && has_only_digits(value)){
          long level_ = stoLong(value);
          if(level_ >= 0 || level_ <= 5){
            settings.log.level = (uint8_t)level_;
            return true;
          }
        }
        break;
      case log_active:
        if(value != "" && has_only_digits(value)){
          long active_ = stoLong(value);
          if(active_ == 0 || active_ == 1){
            settings.log.active = (bool)active_;
            return true;
          }
        }
        break;

      case keepalive_period:
        if(value != "" && has_only_digits(value)){
          settings.keepalive.period = stoLong(value);
          return true;
        }
        break;
      case keepalive_active:
        if(value != "" && has_only_digits(value)){
          long active_ = stoLong(value);
          if(active_ == 0 || active_ == 1){
            settings.keepalive.active = (bool)active_;
            return true;
          }
        }
        break;
    }

    return false;
}

// find settings param
settingsTopics_ resolveOptionSettings(std::map<long, settingsTopics_> map, String param) {

  std::string param_ = std::string(param.c_str());
  long str_hash = (long)std::hash<std::string>{}(param_);
  std::map<long,settingsTopics_>::iterator it;

  it = map.find(str_hash);
  if(it != map.end())
    return it->second;

  return setting_not_found;
}

void settings_log(){

  if(!settings.log.active)
    return;

  Serial.println("wifi.mode: "+String(settings.wifi.mode));
  Serial.println("wifi.ssid: "+String(settings.wifi.ssid));
  Serial.println("wifi.pwd: "+String(settings.wifi.pwd));

#ifdef ENABLE_LTE
  Serial.println("modem.apn: "+String(settings.modem.apn));
  Serial.println("modem.user: "+String(settings.modem.user));
  Serial.println("modem.pwd: "+String(settings.modem.pwd));
  Serial.println("modem.band: "+String(settings.modem.band));
  Serial.println("modem.cops: "+String(settings.modem.cops));
#endif

  Serial.println("MQTT Client1 Connection");
  Serial.println("mqtt.host: "+String(settings.mqtt.host));
  Serial.println("mqtt.user: "+String(settings.mqtt.user));
  Serial.println("mqtt.pass: "+String(settings.mqtt.pass));
  Serial.println("mqtt.port: "+String(settings.mqtt.port));
  Serial.println("mqtt.active: "+String(settings.mqtt.active));

  Serial.println("MQTT Client2 Connection");
  Serial.println("mqtt.host: "+String(settings.mqtt2.host));
  Serial.println("mqtt.user: "+String(settings.mqtt2.user));
  Serial.println("mqtt.pass: "+String(settings.mqtt2.pass));
  Serial.println("mqtt.port: "+String(settings.mqtt2.port));
  Serial.println("mqtt.active: "+String(settings.mqtt2.active));

  Serial.println("log.active: "+String(settings.log.active));
  Serial.println("log.level: "+String(settings.log.level));

  Serial.println("keepalive.active: "+String(settings.keepalive.active));
  Serial.println("keepalive.period: "+String(settings.keepalive.period));

  Serial.println("uart2.active: "+String(settings.uart2.active));
  Serial.println("uart2.baudrate: "+String(settings.uart2.baudrate));
  Serial.println("uart2.config: "+String(settings.uart2.config));
}

bool has_only_digits(String value_str){

  #ifndef UNITTEST
  unsigned int stringLength = value_str.length();

  if (stringLength < 1)
    return false;

  for(unsigned int i = 0; i < stringLength; ++i) {
    if (isDigit(value_str.charAt(i)))
      continue;
    Serial.println("is not digit: "+value_str.charAt(i));
    return false;
  }
  #else
    if(value_str.length() == 0)
      return false;
    return (value_str.find_first_not_of( "0123456789" ) == std::string::npos);
  #endif 
  return true;
}

long stoLong(String s){

  #ifndef UNITTEST
    return s.toInt();
  #else
    return std::stoi(s);
  #endif

  return 0;
}

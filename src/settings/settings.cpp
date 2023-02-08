
#include "settings.h"


user_settings settings = {

  .fw = {
    /* version */     "",
    /* md5 */         ""
  },

  .wifi = {
    /* mode */        "",
    /* ssid */        "",
    /* pwd */         ""
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
    /* host */        "",
    /* user */        "",
    /* pass */        "",
    /* prefix */      "",
    /* port */        0,
    /* active */      false
  },

  .log = {
    /* active */      false,
    /* level */       0
  },

  .keepalive = {
    /* active */      false,
    /* period */      0 // in seconds
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
        if(value.length() <= sizeof(settings.mqtt.host)){
          memset(settings.mqtt.host,0,sizeof(settings.mqtt.host));
          memcpy(settings.mqtt.host,value.c_str(),value.length());
          return true;
        }
        break;
      case mqtt_user:
        if(value.length() <= sizeof(settings.mqtt.user)){
          memset(settings.mqtt.user,0,sizeof(settings.mqtt.user));
          memcpy(settings.mqtt.user,value.c_str(),value.length());
          return true;
        }
        break;
      case mqtt_pass:
        if(value.length() <= sizeof(settings.mqtt.pass)){
          memset(settings.mqtt.pass,0,sizeof(settings.mqtt.pass));
          memcpy(settings.mqtt.pass,value.c_str(),value.length());
          return true;
        }
        break;
      case mqtt_prefix:
        if(value.length() <= sizeof(settings.mqtt.prefix)){
          memset(settings.mqtt.prefix,0,sizeof(settings.mqtt.prefix));
          memcpy(settings.mqtt.prefix,value.c_str(),value.length());
          return true;
        }
        break;
      case mqtt_port:
        if(value != "" && has_only_digits(value)){
          settings.mqtt.port = (uint8_t)stoLong(value);
          return true;
        }
        break;
      case mqtt_active:
        if(value != "" && has_only_digits(value)){
          long active_ = stoLong(value);
          if(active_ == 0 || active_ == 1){
            settings.mqtt.active = (bool)active_;
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

  DBGLOG(Info,"wifi.mode: "+String(settings.wifi.mode));
  DBGLOG(Info,"wifi.ssid: "+String(settings.wifi.ssid));
  DBGLOG(Info,"wifi.pwd: "+String(settings.wifi.pwd));

  DBGLOG(Info,"modem.apn: "+String(settings.modem.apn));
  DBGLOG(Info,"modem.user: "+String(settings.modem.user));
  DBGLOG(Info,"modem.pwd: "+String(settings.modem.pwd));
  DBGLOG(Info,"modem.band: "+String(settings.modem.band));
  DBGLOG(Info,"modem.cops: "+String(settings.modem.cops));

  DBGLOG(Info,"mqtt.host: "+String(settings.mqtt.host));
  DBGLOG(Info,"mqtt.user: "+String(settings.mqtt.user));
  DBGLOG(Info,"mqtt.pass: "+String(settings.mqtt.pass));
  DBGLOG(Info,"mqtt.prefix: "+String(settings.mqtt.prefix));
  DBGLOG(Info,"mqtt.port: "+String(settings.mqtt.port));
  DBGLOG(Info,"mqtt.active: "+String(settings.mqtt.active));

  DBGLOG(Info,"log.active: "+String(settings.log.active));
  DBGLOG(Info,"log.level: "+String(settings.log.level));

  DBGLOG(Info,"keepalive.active: "+String(settings.keepalive.active));
  DBGLOG(Info,"keepalive.period: "+String(settings.keepalive.period));
}

bool has_only_digits(String value_str){

  #ifndef UNITTEST
  unsigned int stringLength = value_str.length();

  if (stringLength == 0)
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

}

long stoLong(String s){

  #ifndef UNITTEST
    if(!has_only_digits(s))
      return 0;
    return s.toInt();
  #else
    return std::stoi(s);
  #endif

  return 0;
}

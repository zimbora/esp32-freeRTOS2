
#define SYS_PATH_SETTINGS "/etc/settings"
#define FW_SETTINGS_FILENAME "/etc/settings/fw.txt"
#define FW_AR_FILENAME "/etc/settings/ar.txt"
#define FW_ALARM_FILENAME "/etc/settings/alarm.txt"
#define FW_JS_FILENAME "/etc/settings/js.txt"

#define APP_PATH_SETTINGS "/home/app/settings"
#define APP_SETTINGS_FILENAME "/home/app/settings/app.txt"
#define APP_PATH_RECORDS "/home/app/records"
#define APP_CALIB_FILENAME "/home/app/calib.txt"
#define APP_ACCUM_FILENAME "/home/app/accum.txt"
#define APP_ACCUM_TP_FILENAME "/home/app/accumtp.txt"


#define MQTT_HOST_1 "192.168.1.100" // mac OS broker
//#define MQTT_HOST_1 "iot.inloc.cloud" // mac OS broker
#define MQTT_USER_1 "device"
#define MQTT_PASSWORD_1 "device"

#define MQTT_HOST_2 ""
#define MQTT_USER_2 ""
#define MQTT_PASSWORD_2 ""

#define MQTT_PROJECT "freeRTOS2"
#define MQTT_WILL_SUBTOPIC "/status"
#define MQTT_WILL_PAYLOAD "offline"
#define MQTT_PATH_RECORDS "/records"

#define DEPLOY_DEVELOPMENT_VODAFONE

#if defined DEPLOY_DEVELOPMENT_NOS
	#define SETTINGS_MODEM_APN         						"tdtnb"
	#define SETTINGS_MODEM_USERNAME    						""
	#define SETTINGS_MODEM_PASSWORD    						""
	#define SETTINGS_MODEM_COPS		    						26803
	#define SETTINGS_MODEM_BAND		    						0
	#define SETTINGS_MODEM_TECH		    						1 // 0-ND;1-GSM;2-GPRS;3-NB2;4-CATM1;5-AUTO
#elif defined DEPLOY_DEVELOPMENT_VODAFONE
	#define SETTINGS_MODEM_APN         						"m2m.vodafone.pt"
	//#define SETTINGS_MODEM_APN         						"nbiot1.vodafone.pt"  //
	//#define SETTINGS_MODEM_APN         						"nbiot2.vodafone.pt"  // 12 hours sleep mode
	//#define SETTINGS_MODEM_APN         						"nbiot3.vodafone.pt"  // 12 hours sleep mode
	#define SETTINGS_MODEM_USERNAME    						""
	#define SETTINGS_MODEM_PASSWORD    						""
	//#define SETTINGS_MODEM_COPS		    					26801
	#define SETTINGS_MODEM_COPS		    						0
	#define SETTINGS_MODEM_BAND		    						0
	#define SETTINGS_MODEM_TECH		    						1 // 0-ND;1-GSM;2-GPRS;3-NB2;4-CATM1;5-AUTO
#endif

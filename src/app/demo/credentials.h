
#define SYS_PATH_SETTINGS "/etc/settings"
#define FW_SETTINGS_FILENAME "/etc/settings/fw.txt"
#define FW_AR_FILENAME "/etc/settings/ar.txt"
#define FW_ALARM_FILENAME "/etc/settings/alarm.txt"
#define FW_JS_FILENAME "/etc/settings/js.txt"

#define APP_PATH_SETTINGS "/home/app/settings"
#define APP_SETTINGS_FILENAME "/home/app/settings/app.txt"
#define APP_PATH_RECORDS "/home/app/records"

#define MQTT_HOST_1 "192.168.1.108" // mac OS broker
#define MQTT_USER_1 "device"
#define MQTT_PASSWORD_1 "device"

#define MQTT_HOST_2 ""
#define MQTT_USER_2 ""
#define MQTT_PASSWORD_2 ""

#define MQTT_PROJECT "esp32/freeRTOS"
#define MQTT_WILL_SUBTOPIC "/status"
#define MQTT_WILL_PAYLOAD "offline"
#define MQTT_PATH_RECORDS "/records"

#define DEPLOY_DEVELOPMENT_VODAFONE

#if defined DEPLOY_DEVELOPMENT_NOS
	#define SETTINGS_NB_APN         						"tdtnb"
	#define SETTINGS_NB_USERNAME    						""
	#define SETTINGS_NB_PASSWORD    						""
	#define SETTINGS_NB_COPS		    						26803
#elif defined DEPLOY_DEVELOPMENT_VODAFONE
	#define SETTINGS_NB_APN         						"m2m.vodafone.pt"
	//#define SETTINGS_NB_APN         						"nbiot1.vodafone.pt"  //
	//#define SETTINGS_NB_APN         						"nbiot2.vodafone.pt"  // 12 hours sleep mode
	//#define SETTINGS_NB_APN         						"nbiot3.vodafone.pt"  // 12 hours sleep mode
	#define SETTINGS_NB_USERNAME    						""
	#define SETTINGS_NB_PASSWORD    						""
	//#define SETTINGS_NB_COPS		    						26801
	#define SETTINGS_NB_COPS		    						0
#endif

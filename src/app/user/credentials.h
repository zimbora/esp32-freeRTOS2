
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

//#define WIFI_SSID "inlocAP"
#define WIFI_SSID "Vodafone-rocks"
//#define WIFI_PASSWORD "inlocAPpwd"
#define WIFI_PASSWORD "famelgasilva"

#define MQTT_HOST_1 "devices.dev.inloc.cloud" // mac OS broker
#define MQTT_PORT_1 1883
#define MQTT_USER_1 "device"
#define MQTT_PASSWORD_1 "device"

#define MQTT_HOST_2 "" // mac OS broker
#define MQTT_PORT_2 1883
#define MQTT_USER_2 ""
#define MQTT_PASSWORD_2 ""
#define MQTT_ACTIVE_2 false

// CANNOT be changed after being set
#define MQTT_PROJECT "freeRTOS2"
#define MQTT_UID_PREFIX "uid:"
#define MQTT_WILL_SUBTOPIC "/status"
#define MQTT_WILL_PAYLOAD "offline"
#define MQTT_PATH_RECORDS "/records"

#define LOG_ACTIVE true
#define LOG_LEVEL 5

#define KEEPALIVE_ACTIVE true
#define KEEPALIVE_PERIOD 15

// also used for RS485
#define UART2_ACTIVE true
#define UART2_BAUDRATE 115200
#define UART2_CONFIG SERIAL_8N1
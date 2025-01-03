
#ifndef APP_PACKAGE_H
#define APP_PACKAGE_H

/////////////////////////////////////////////////////////////////////
//                                                                 //
//                            APP Version    											 //
//                                                                 //
/////////////////////////////////////////////////////////////////////

#define APP_VERSION        			 	"1.0.0"

/////////////////////////////////////////////////////////////////////
//                                                                 //
// FW MODEL       																						 		 //
//                                                                 //
/////////////////////////////////////////////////////////////////////

/* supported Models by our cloud
* - devices.inloc.cloud
* If you chose a different model you have to develop your own web
* interface or request support. Contact developer in that case:
* - lucas.ua.eet@gmail.com
*/
#define FW_MODEL                  "DEMO"


/////////////////////////////////////////////////////////////////////
//                                                                 //
// Enabled modules																						 		 //
//                                                                 //
/////////////////////////////////////////////////////////////////////
//#define ENABLE_LTE // Use LTE modem radio
//#define ENABLE_BLE // BLE for device configuration
//#define ENABLE_AP // Access Point for wifi configuration

/////////////////////////////////////////////////////////////////////
//                                                                 //
//                         APP PROCESS		                       //
//                                                                 //
/////////////////////////////////////////////////////////////////////

// select just one option
//#define FAST_APP // App runing on main thread
//#define THREAD_APP // 1 dedicated thread for app
#define SYNCHED_APP // App running on core thread

// TASKS CONFIGS
#define NETWORK_CORE_TASK_SIZE 2048*4
#define NETWORK_CORE_TASK_PRIORITY 3

#define MRTOS_TASK_SIZE 2048*4
#define MRTOS_TASK_PRIORITY 2

#define NETWORK_LTE_TASK_SIZE 2048*4
#define NETWORK_LTE_TASK_PRIORITY 2

#define NETWORK_APP_TASK_SIZE 2048*4
#define NETWORK_APP_TASK_PRIORITY 1

/////////////////////////////////////////////////////////////////////
//                                                                 //
//                         HARDWARE SETTINGS                       //
//                                                                 //
/////////////////////////////////////////////////////////////////////

//#define ENABLE_JS // JavaScript for user runtime code
#define EXT_SERIAL_COMM // uart2 assigned to Serial1
//#define ENABLE_RS485 // enable rs485 module on Serial1
//#define DIGITAL_COUNTERS

/////////////////////////////////////////////////////////////////////
//                                                                 //
//                         IOS MAPPING                             //
//                                                                 //
/////////////////////////////////////////////////////////////////////

#define SERIAL1_GPIO_RX 27
#define SERIAL1_GPIO_TX 14
#define SERIAL1_GPIO_RTS 13

/////////////////////////////////////////////////////////////////////
//                                                                 //
//                         DEBUG                             	   //
//                                                                 //
/////////////////////////////////////////////////////////////////////
#define DEBUG_SNIFFER

/////////////////////////////////////////////////////////////////////
//                                                                 //
//                   Libraries macros                              //
//                                                                 //
/////////////////////////////////////////////////////////////////////
#define MQTT_TX_QUEUE_SIZE 10
//#define DEBUG_HTTP
//#DEBUG_MQTT_PAYLOAD

#endif

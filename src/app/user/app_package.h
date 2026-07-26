
#ifndef APP_PACKAGE_H
#define APP_PACKAGE_H

#include "board.h"

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
* - devices.dev.inloc.cloud
* Don't change model name if you want to test our cloud services. 
* If you want to build a product with this code set a custom model name
* A good practice is to use your MPN (Manufacturer Part Number) as model name,
* preceded by your company name or brand. For example: "mycompany-myproduct-001"
* If you want to use our cloud services contact us for support.
* Contacts:
* - lucas.ua.eet@gmail.com
* I hope you appreciate our work and services.
* 
* Note: Fota services are available for all models, but our cloud services 
* are only available for supported models. Each model has a unique identifier
* and can only be updated with firmware that matches its model name. 
*/
#define FW_MODEL                  "DEMO"


/////////////////////////////////////////////////////////////////////
//                                                                 //
// Enabled modules																						 		 //
//                                                                 //
/////////////////////////////////////////////////////////////////////
/*
* ENABLE LTE is not tested for several releases and not recommended 
* without further testing. It is recommended to use the WiFi module for now.
* You are invited to collaborate with us to test and improve the LTE module.
* https://github.com/zimbora/esp32-modem-freeRTOS
*
* Access Point has been our primary choice for reconfiguration or setup.
* BLE is an alternative but requires additional testing on your side.
*/
//#define ENABLE_LTE // Use LTE modem radio
//#define ENABLE_BLE // BLE for device configuration
#define ENABLE_AP // Access Point for wifi configuration

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

// GPIO pins are defined in board.h
#ifndef SERIAL1_GPIO_RX
  #define SERIAL1_GPIO_RX 27
#endif
#ifndef SERIAL1_GPIO_TX
  #define SERIAL1_GPIO_TX 14
#endif
#ifndef SERIAL1_GPIO_RTS
  #define SERIAL1_GPIO_RTS 13
#endif

/////////////////////////////////////////////////////////////////////
//                                                                 //
//                         DEBUG                             	   //
//                                                                 //
/////////////////////////////////////////////////////////////////////
//#define DEBUG_MQTT_TOPIC
//#define DEBUG_MQTT_PAYLOAD

/////////////////////////////////////////////////////////////////////
//                                                                 //
//                   Libraries macros                              //
//                                                                 //
/////////////////////////////////////////////////////////////////////
#define MQTT_TX_QUEUE_SIZE 10
//#define DEBUG_HTTP

#endif

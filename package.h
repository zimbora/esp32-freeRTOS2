
#ifndef PACKAGE_H
#define PACKAGE_H

#define ESP32DEBUGGING


/////////////////////////////////////////////////////////////////////
//                                                                 //
// Enabled modules																						 		 //
//                                                                 //
/////////////////////////////////////////////////////////////////////
// !!To use LTE modem, ENABLE_LTE must also be defined in modem_freeRTOS library
//#define ENABLE_LTE // Use LTE modem radio
//#define ENABLE_BLE // BLE for device configuration
#define ENABLE_AP // Access Point for wifi configuration
#define ENABLE_JS // JavaScript for user runtime code

/////////////////////////////////////////////////////////////////////
//                                                                 //
// Firmware Version																						 		 //
//                                                                 //
/////////////////////////////////////////////////////////////////////

#define FW_VERSION        			 	"1.0.3"

/////////////////////////////////////////////////////////////////////
//                                                                 //
//                          CHOOSE DEVICE                          //
//                                                                 //
/////////////////////////////////////////////////////////////////////

#define APP HOMEHEALTH

#if APP == DEMO
	#define DEMO
#elif APP == SLIM_GW // in production
	#define SLIM_GW
#elif APP == MEA_GW // in production
	#define MEA_GW
#elif APP == HOMEHEALTH // in dev
	#define HOMEHEALTH
#endif

//#define FW_MODEL "M5STACK"
//#define FW_MODEL "FIAMMA" 								// - NA
//#define FW_MODEL "MODULE_IO" 								// - NA
//#define FW_MODEL "HOMEHEALTH" 							// - NA
//#define FW_MODEL "IRHUB"  								// - NA
//#define FW_MODEL "ETHERNET_GW" 							// - NA
//#define FW_MODEL "ENERGY_ANALYZER" 						// - NA
//#define FW_MODEL "MULTI_ENERGY_ANALYZER"					// - NA
//#define FW_MODEL "SMART_MOTION"							// - NA

/////////////////////////////////////////////////////////////////////
//                                                                 //
//                         HARDWARE SETTINGS                       //
//                                                                 //
/////////////////////////////////////////////////////////////////////

#ifdef DEMO
#define RS485_GPIO_RX 27
#define RS485_GPIO_TX 14
#define RS485_GPIO_RTS 13
#endif

#ifdef SLIM_GW_WIFI
#define RS485_GPIO_RX 27
#define RS485_GPIO_TX 14
#define RS485_GPIO_RTS 13
#endif

#endif

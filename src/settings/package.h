
#ifndef PACKAGE_H
#define PACKAGE_H

#define ESP32DEBUGGING


/////////////////////////////////////////////////////////////////////
//                                                                 //
// Enabled modules																						 		 //
//                                                                 //
/////////////////////////////////////////////////////////////////////
//#define ENABLE_BLE // BLE for device configuration
#define ENABLE_AP // Access Point for wifi configuration
#define ENABLE_JS // JavaScript for user runtime code
#define ENABLE_RS485 // enable rs485 module

/////////////////////////////////////////////////////////////////////
//                                                                 //
// Firmware Version																						 		 //
//                                                                 //
/////////////////////////////////////////////////////////////////////

#define FW_VERSION        			 	"0.1.2"

/////////////////////////////////////////////////////////////////////
//                                                                 //
//                          CHOOSE DEVICE                          //
//                                                                 //
/////////////////////////////////////////////////////////////////////

//#define TEST
#define SLIM_GW




#ifdef TEST
#define FW_MODEL "TEST"
#elif defined SLIM_GW
#define FW_MODEL "SLIM_GW"
#endif
//#define FW_MODEL "M5STACK"
//#define FW_MODEL "FIAMMA" 										// - NA
//#define FW_MODEL "MODULE_IO" 								// - NA
//#define FW_MODEL "HOMEHEALTH" 								// - NA
//#define FW_MODEL "IRHUB"  										// - NA
//#define FW_MODEL "ETHERNET_GW" 							// - NA
//#define FW_MODEL "ENERGY_ANALYZER" 					// - NA
//#define FW_MODEL "MULTI_ENERGY_ANALYZER"
//#define FW_MODEL "SMART_MOTION"							// - NA

/////////////////////////////////////////////////////////////////////
//                                                                 //
//                         HARDWARE SETTINGS                       //
//                                                                 //
/////////////////////////////////////////////////////////////////////

#ifdef SLIM_GW
#define RS485_GPIO_RX 27
#define RS485_GPIO_TX 14
#define RS485_GPIO_RTS 13
#endif

#endif

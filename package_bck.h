
#ifndef PACKAGE_H
#define PACKAGE_H

#define ESP32DEBUGGING


/////////////////////////////////////////////////////////////////////
//                                                                 //
// Enabled modules																						 		 //
//                                                                 //
/////////////////////////////////////////////////////////////////////
//#define ENABLE_LTE // Use LTE modem radio
//#define ENABLE_BLE // BLE for device configuration
//#define ENABLE_AP // Access Point for wifi configuration
//#define ENABLE_JS // JavaScript for user runtime code

/////////////////////////////////////////////////////////////////////
//                                                                 //
// Firmware Version																						 		 //
//                                                                 //
/////////////////////////////////////////////////////////////////////

#define FW_VERSION	"1.0.3"

/////////////////////////////////////////////////////////////////////
//                                                                 //
//                          CHOOSE DEVICE                          //
//                                                                 //
/////////////////////////////////////////////////////////////////////

#define APP_AUX_MACRO
#define APP_TEST
//#define APP_DEMO
//#define SLIM_GW
/*
#if APP == APP_DEMO
#define APP_DEMO
#elif APP == APP_TEST
#define APP_TEST
#elif APP == SLIM_GW // in production
#define SLIM_GW
#elif APP == MEA_GW // in production
#define MEA_GW
#elif APP == HOMEHEALTH // in dev
#define HOMEHEALTH
#endif
*/

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

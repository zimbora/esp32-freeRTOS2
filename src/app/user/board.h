
#ifndef BOARD_H
#define BOARD_H

/////////////////////////////////////////////////////////////////////
//                                                                 //
//              BOARD DEFINITION                                   //
//                                                                 //
/////////////////////////////////////////////////////////////////////

// Auto-select board by target chip:
//  - esp32      -> BOARD_ESP32_WROOM_32D
//  - esp32c5    -> BOARD_ESP32C5_WIFI6_KIT_N16R8
#if defined(CONFIG_IDF_TARGET_ESP32C5)
  #define BOARD_ESP32C5_WIFI6_KIT_N16R8
#elif defined(CONFIG_IDF_TARGET_ESP32)
  #define BOARD_ESP32_WROOM_32D
#endif

/////////////////////////////////////////////////////////////////////
//                                                                 //
//        ESP32-C5-DevKitC-1 (WiFi6-Kit-N16R8)                    //
//                                                                 //
//  UART0 (Serial / logs): TX=GPIO11, RX=GPIO12                  //
//    -> routed to USB-to-UART bridge (CP2102N)                   //
//                                                                 //
//  Pin headers J1 & J3 expose:                                   //
//    J1: GPIO 0,1,2,3,6,7,8,9,10,25,26                          //
//    J3: GPIO 4,5,13,14,23,24,27,28                              //
//                                                                 //
//  RGB LED: GPIO27                                                //
//  USB D+/D-: GPIO14/GPIO13                                       //
//  LP_UART: RX=GPIO4, TX=GPIO5                                   //
//                                                                 //
/////////////////////////////////////////////////////////////////////

#ifdef BOARD_ESP32C5_WIFI6_KIT_N16R8

  // Modem power key (adjust if modem is connected)
  #define PWKEY 8

  // Serial1 - external UART (exposed on J3 header)
  #define SERIAL1_GPIO_RX  23
  #define SERIAL1_GPIO_TX  24
  #define SERIAL1_GPIO_RTS 28

  // RGB LED
  #define LED_PIN 27

  // Serial (UART0) for logs is the default:
  //   TX = GPIO11, RX = GPIO12
  //   Connected to USB-to-UART bridge on the dev kit.
  //   Just use Serial.begin(115200) — no pin override needed.
  #define SERIAL_LOG_BAUD 115200

#endif // BOARD_ESP32C5_WIFI6_KIT_N16R8

/////////////////////////////////////////////////////////////////////
//                                                                 //
//        ESP32-WROOM-32D (DevKitC or equivalent)                 //
//                                                                 //
//  UART0 (Serial / logs): TX=GPIO1, RX=GPIO3                    //
//    -> routed to USB-to-UART bridge (CP2102 / CH340)            //
//    -> Just use Serial.begin(115200) — no pin override needed.  //
//                                                                 //
//  UART2 (Serial1 / external): TX=GPIO17, RX=GPIO16             //
//                                                                 //
//  Built-in LED: GPIO2                                            //
//  Available headers: GPIO 4,5,12,13,14,15,18,19,21,22,23,      //
//                     25,26,27,32,33,34,35,36,39                 //
//                                                                 //
/////////////////////////////////////////////////////////////////////

#ifdef BOARD_ESP32_WROOM_32D

  // Modem power key (adjust if modem is connected)
  #define PWKEY 4

  // Serial1 - external UART (UART2 remapped to these pins)
  #define SERIAL1_GPIO_RX  16
  #define SERIAL1_GPIO_TX  17
  #define SERIAL1_GPIO_RTS 18

  // Built-in LED
  #define LED_PIN 2

  // Serial (UART0) for logs is the default:
  //   TX = GPIO1, RX = GPIO3
  //   Connected to USB-to-UART bridge on the dev kit.
  //   Just use Serial.begin(115200) — no pin override needed.
  #define SERIAL_LOG_BAUD 115200

#endif // BOARD_ESP32_WROOM_32D

#endif // BOARD_H


#ifndef BOARD_H
#define BOARD_H

/////////////////////////////////////////////////////////////////////
//                                                                 //
//              BOARD DEFINITION                                   //
//                                                                 //
/////////////////////////////////////////////////////////////////////

// Uncomment the board you are using
#define BOARD_ESP32C5_WIFI6_KIT_N16R8

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

#endif // BOARD_H

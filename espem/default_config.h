// Default options
// rename this file to user_config.h and make your changes there

#pragma once

/*
 * PZEM library version
 * if defined 'USE_PZEMv3', than PZEM hardware version v3.0 is used with a library PZEM-004T-v30
 * otherwise used an old PZEM004T with Oleh's library
 */
//#define USE_PZEMv3

// By default Hardware Serial is used for interacting with PZEM
/* ***** SoftWare Serial is deprecated *****
   it might still work flawlessly but no longer supported

*/ Uncomment this to use Software Serial pins to connect to PZEM
/*
#ifdef ESPEM_USE_HWSERIAL
 #undef ESPEM_USE_HWSERIAL
#endif
*/

/* ESP8266 has two UART ports - UART0, UART1
   Note: UART0 pins are usually connected to an onboard USB2serial converter
   It's better not to share USB2serial pins for PZEM communication
   Also it's not possible to use 'DEBUG' and 'UART0' at the same time
   Use either UART1 pin for debug or software serial for pzem connection on any other GPIO's available
   for details, see http://esp8266.github.io/Arduino/versions/2.1.0-rc2/doc/reference.html#serial

    HW Serial to use:
    for esp8266 values are: UART0, UART1 (can not actually be used)
    for esp32 values are: (0U), (1U), (2U)
    default is:
     UART0 for esp8266
     (2U) for esp32     /pins IO16 (RX2) and IO17 (TX2)/
 */
//#define HWSERIAL_PORT UART0

// ESP8266 - Swap hw_serial from default gpio1,3 to gpio13(tx),gpio15(rx)
// Note: gpio15 must be pulled low on esp start, otherwise it won't boot from eeprom
// Pzem pulls it's RX pin high via optocoupler's led, so make sure to workaround it anyhow
// https://github.com/olehs/PZEM004T/wiki/Connecting-PZEM-to-ESP8266

//#define ESPEM_HWSERIAL_SWAP


// Debuging messages via hw serail
                                  // undefined - disabled (default)
//#define ESPEM_DEBUG Serial      // output via hwserial0
//#define ESPEM_DEBUG Serial1     // output via hwserial1  TX pin is GPIO2 for 8266


// RX/TX pins for sw_serial OR custom pin maping for ESP32 HWSerial
// If 'USE_HWSERIAL' is not defined than use those GPIO pins for Software serial
// ***** SoftWare Serial is deprecated *****
//#define PIN_RX  13    // only pins 0-5,12-15 are allowed for RX,TX on ESP8266
//#define PIN_TX  15    // 5, 4 is D1,D2 on NodeMCU board
                        // 5,14 is D1,D5 on WittyCloud board
                        // Lolin
                        // RX D7   GPIO 13
                        // TX D8   GPIO 15


// Build with FTP server support
/*
#ifndef USE_FTP
#define USE_FTP
*/

// FTP server credentials
// #define FTP_USER "ftp"
// #define FTP_PASSWORD "ftp"

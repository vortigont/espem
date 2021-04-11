// Default options
// rename this file to user_config.h and make your changes there

#pragma once

// Use HW serial pins to connect to PZEM
// see notes below
//#define ESPEM_USE_HWSERIAL


// ESP has two UART ports - UART0, UART1
// Note: UART0 pins are usually connected to an onboard USB2serial converter
// It's better not to share USB2serial pins for PZEM communication
// Also it's not possible to use 'DEBUG' and 'UART0' at the same time
// Use either UART1 pin for debug or software serial for pzem connection on any other GPIO's available
// for details, see http://esp8266.github.io/Arduino/versions/2.1.0-rc2/doc/reference.html#serial

// Swap hw_serial from default gpio1,3 to gpio13(tx),gpio15(rx)
// Note: gpio15 must be pulled low on esp start, otherwise it won't boot from eeprom
// Pzem pulls it's RX pin high via optocoupler's led, so make sure to workaround it anyhow
// https://github.com/olehs/PZEM004T/wiki/Connecting-PZEM-to-ESP8266

//#define ESPEM_HWSERIAL_SWAP

 

// Debuging messages via hw serail
                                  // undefined - disabled (default)
//#define ESPEM_DEBUG Serial      // output via hwserial0
//#define ESPEM_DEBUG Serial1     // output via hwserial1  TX pin is GPIO2 for 8266


// RX/TX pins for sw_serial 
// If 'USE_HWSERIAL' is not defined than use those GPIO pins for Software serial
//#define PIN_RX  13    // only pins 0-5,12-15 are allowed for RX,TX on ESP
//#define PIN_TX  15    // 5, 4 is D1,D2 on NodeMCU board
                        // 5,14 is D1,D5 on WittyCloud board
                        // Lolin
                        // RX D7   GPIO 13
                        // TX D8   GPIO 15

// TimeZone and country for NTP (optional)
//#define TZONE TZ_Europe_Moscow  // Zones defined in cores/esp8266/TZ.h
//#define COUNTRY "ru"            // Country double-letter code

// Build with FTP server suuport
// default credentials 'ftp:ftp'
//#define USE_FTP

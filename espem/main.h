/* 
 *  ESPEM - ESP Energy monitor
 *  A code for ESP8266 based boards to interface with PeaceFair PZEM PowerMeters
 *  It can poll/collect PowerMeter data and provide it for further processing in 
 *  text/JSON format
 *  Author           : Emil Muratow
 * 
 *  This file        : main.h
 *  This file Author : Emil Muratow
 *  
 *  Description      : Common includes and defines required by whole project
 *  (c) Emil Muratov 2017
 * 
 */

#define HWSERIAL_PORT UART0
#include <stdint.h>
#include <sys/types.h>
// Sketch configuration
#include "config.h"
// HTTP related def's
#include "http.h"

#include <ESP8266WiFi.h>        // from Ardurino SDK
#include <ESP8266WebServer.h>   // from Ardurino SDK
// OTA Updates
#include <ESP8266HTTPClient.h>  // from Ardurino SDK
#include <ESP8266httpUpdate.h>  // from Ardurino SDK

//#include <SoftwareSerial.h>   // Must be a special lib for ESP!     https://github.com/plerup/espsoftwareserial
#include <PZEM004T.h>		// Peacefair PZEM-004T Energy monitor https://github.com/vortigont/PZEM004T
				// a forked version of oleh's lib with a fix for esp platform
				// (should switch back if merged to oleh's master https://github.com/olehs/PZEM004T)

#include <SimpleTimer.h>     	// Simple timers                      https://github.com/jfturcot/SimpleTimer

// NTP time
#include <TimeLib.h>            // Time library for Arduino           https://github.com/PaulStoffregen/Time.git
#include <NtpClientLib.h>	// this lib uses internal Espressif SDK SNTP    https://github.com/gmag11/NtpClient
#include <EEPROM.h>             // from Ardurino SDK
#include <ArduinoJson.h>        // https://github.com/bblanchon/ArduinoJson.git

#include "Configuration.h"
#ifndef uint16_t
#define uint16_t unsigned short
#endif
#ifndef size_t
#define size_t   unsigned int
#endif
#ifndef uint8_t
#define uint8_t  unsigned char
#endif

// callback functions
void wifibegin(const cfg &conf);   // Initialize WiFi

void datapoller();                 // Poll meter for data and store samples
#ifdef  _USE_MILLI_SLEEP_
void milli_sleep(unsigned long millseconds=0);
#endif

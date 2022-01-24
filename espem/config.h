// Default config options
// do NOT change anything here, copy/rename 'default_config.h' into 'user_config.h' and change you options there

#pragma once

#if defined __has_include
#  if __has_include("user_config.h")
#    include "user_config.h"
#  endif
#endif


#define FW_NAME "espem"

#define ESPEM_USE_HWSERIAL

#ifndef HWSERIAL_PORT
  #ifdef ESP8266
    #define HWSERIAL_PORT UART0
  #endif
  #ifdef ESP32
    #define HWSERIAL_PORT (2U)
  #endif
#endif

// ESP32/SoftSerial pins
#ifndef PIN_RX
#define PIN_RX  22
#endif

#ifndef PIN_TX
#define PIN_TX  17
#endif

// LOG macro's
#if defined(LOG)
#undef LOG
#endif

#if defined(ESPEM_DEBUG)
    #define LOG(func, ...) ESPEM_DEBUG.func(__VA_ARGS__)
#else
    #define LOG(func, ...) ;
#endif

/*  ESPEM - ESP Energy monitor
 *  A code for ESP8266 based boards to interface with PeaceFair PZEM PowerMeters
 *  It can poll/collect PowerMeter data and provide it for futher processing in text/json format
 *
 *  (c) Emil Muratov 2018 - 2021
 *
 */

#pragma once

#define UPD_RESTART_DELAY   5   // restart delay when updating firmware
#define BAUD_RATE       115200  // serial debug port baud rate
#define HTTP_VER_BUFSIZE  200

// Sketch configuration
#include "globals.h"    // EmbUI macro's for LOG
#include "config.h"
#include "uistrings.h"  // non-localized text-strings
#include "ui_i18n.h"    // localized GUI text-strings
#include <ESPAsyncWebServer.h>

#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

// EMBUI
void create_parameters();       // декларируем для переопределения weak метода из фреймворка для WebUI
void sync_parameters();

// WiFi connection callback
void onSTAGotIP();
// Manage network disconnection
void onSTADisconnected();

void wver(AsyncWebServerRequest *request);
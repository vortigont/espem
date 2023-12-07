/*  ESPEM - ESP Energy monitor
 *  A code for ESP32 based boards to interface with PeaceFair PZEM PowerMeters
 *  It can poll/collect PowerMeter data and provide it for futher processing in text/json format
 *
 *  (c) Emil Muratov 2018 - 2023
 *
 */

#pragma once

#define FW_NAME "espem"

#define FW_VERSION_MAJOR     3
#define FW_VERSION_MINOR     2
#define FW_VERSION_REVISION  0

/* make version as integer*/
#define FW_VERSION ((FW_VERSION_MAJOR) << 16 | (FW_VERSION_MINOR) << 8 | (FW_VERSION_REVISION))

/* make version as string*/
#define FW_VERSION_STRING   TOSTRING(FW_VERSION_MAJOR) "." TOSTRING(FW_VERSION_MINOR) "." TOSTRING(FW_VERSION_REVISION)

#define ESPEM_JSAPI_VERSION     2
#define ESPEM_UI_VERSION        2


#define BAUD_RATE       115200  // serial debug port baud rate
#define HTTP_VER_BUFSIZE  256

#define WEBUI_PUBLISH_INTERVAL  20

// Sketch configuration
#include "globals.h"    // EmbUI macro's for LOG
#include "config.h"
#include "uistrings.h"  // non-localized text-strings
#include <ESPAsyncWebServer.h>

// EMBUI
void create_parameters();       // декларируем для переопределения weak метода из фреймворка для WebUI
void sync_parameters();

// WiFi connection callback
void onSTAGotIP();
// Manage network disconnection
void onSTADisconnected();

void wver(AsyncWebServerRequest *request);
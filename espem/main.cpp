/*  ESPEM - ESP Energy monitor
 *  A code for ESP8266 based boards to interface with PeaceFair PZEM PowerMeters
 *  It can poll/collect PowerMeter data and provide it for futher processing in text/json format
 *
 *  (c) Emil Muratov 2017-2021
 *
 */

// Main headers
#include "main.h"
#include "espem.h"

#include <EmbUI.h>

#ifdef USE_FTP
 #include "ftpSrv.h"
#endif

extern "C" int clock_gettime(clockid_t unused, struct timespec *tp);

// PROGMEM strings
// sprintf template for json version data
#ifdef ESP8266
static const char PGverjson[] PROGMEM = "{\"ChipID\":\"%x\",\"FlashSize\":%u,\"Core\":\"%s\",\"SDK\":\"%s\",\"firmware\":\"%s\",\"version\":\"%s\",\"CPUMHz\":%u,\"Heap\":%u,\"Uptime\":%u,}";
#elif defined ESP32
static const char PGverjson[] PROGMEM = "{\"ChipID\":\"%s\",\"FlashSize\":%u,\"SDK\":\"%s\",\"firmware\":\"%s\",\"version\":\"%s\",\"CPUMHz\":%u,\"Heap\":%u,\"Uptime\":%u,}";
#endif

// Our instance of espem
ESPEM *espem = nullptr;

// ----
// MAIN Setup
void setup() {

#ifdef ESPEM_DEBUG
  ESPEM_DEBUG.begin(BAUD_RATE);  // start hw serial for debugging
#endif

  LOG(println, F("Starting EspEM..."));

  // Start framework, load config and connect WiFi
  embui.begin();


  // restore ESPEM defaults configuration
  ESPEM_CFG _cfg(embui.param(FPSTR(V_EPOLLRT)).toInt(),
                  embui.param(FPSTR(V_EPOOLSIZE)).toInt(),
                  (bool)embui.param(FPSTR(V_EPFFIX)).toInt(),
                  (bool)embui.param(FPSTR(V_EPOLLENA)).toInt(),
                  (mcstate_t)embui.param(FPSTR(V_ECOLLECTORSTATE)).toInt()
  );

  // create and run ESPEM object
  espem = new ESPEM(_cfg);
  espem->begin();


  embui.server.on(PSTR("/fw"), HTTP_GET, [](AsyncWebServerRequest *request){
    wver(request);
  });


  #ifdef USE_FTP
      ftp_setup(); // запуск ftp-сервера
  #endif

  //sync_parameters();    // sync UI params

  embui.setPubInterval(20);
}


// MAIN loop
void loop() {

  embui.handle();

#ifdef USE_FTP
    ftp_loop(); // цикл обработки событий фтп-сервера
#endif
}

// send HTTP responce, json with controller/fw versions and status info
void wver(AsyncWebServerRequest *request) {
  char buff[HTTP_VER_BUFSIZE];

  timespec tp;
  clock_gettime(0, &tp);
#ifdef ESP8266
  snprintf_P(buff, sizeof(buff), PGverjson,
    ESP.getChipId(),
    ESP.getFlashChipSize(),
    ESP.getCoreVersion().c_str(),
    system_get_sdk_version(),
    FW_NAME,
    TOSTRING(FW_VER),
    ESP.getCpuFreqMHz(),
    ESP.getFreeHeap(),
    (uint32_t)tp.tv_sec);
#else
  snprintf_P(buff, sizeof(buff), PGverjson,
    ESP.getChipModel(),
    ESP.getFlashChipSize(),
    ESP.getSdkVersion(),
    FW_NAME,
    TOSTRING(FW_VER),
    ESP.getCpuFreqMHz(),
    ESP.getFreeHeap(),
    (uint32_t)tp.tv_sec);
#endif


  request->send(200, FPSTR(PGmimejson), buff );
}
//
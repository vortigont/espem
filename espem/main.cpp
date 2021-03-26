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
static const char PGverjson[] PROGMEM = "{\"ChipID\":\"%x\",\"FlashSize\":%u,\"Core\":\"%s\",\"SDK\":\"%s\",\"firmware\":\"%s\",\"version\":\"%s\",\"CPUMHz\":%u,\"Heap\":%u,\"Uptime\":%u,}";


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
}


// MAIN loop
void loop() {

  embui.handle();
//  ts.execute();           // run task scheduler

#ifdef USE_FTP
    ftp_loop(); // цикл обработки событий фтп-сервера
#endif
}

// reboot esp task
void espreboot() {
  Task *t = new Task(0, TASK_ONCE, [](){ESP.restart();}, &ts, false);
  t->enableDelayed(UPD_RESTART_DELAY * TASK_SECOND);
}


// send HTTP responce, json with controller/fw versions and status info
void wver(AsyncWebServerRequest *request) {
  char buff[HTTP_VER_BUFSIZE];
  //char* firmware = (char*) malloc(strlen_P(PGver)+1);
  //strcpy_P(firmware, PGver);
  timespec tp;
  clock_gettime(0, &tp);

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

  request->send(200, FPSTR(PGmimejson), buff );
}
//
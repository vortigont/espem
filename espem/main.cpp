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
static const char PGverjson[] PROGMEM = "{\"ChipID\":\"%x\",\"Flash\":%u,\"Core\":\"%s\",\"SDK\":\"%s\",\"firmware\":\"" FW_NAME "\",\"version\":\"" FW_VERSION_STRING "\",\"git\":\"%s\",\"CPUMHz\":%u,\"Heap\":%u,\"Uptime\":%u}";
#elif defined ESP32
static const char PGverjson[] PROGMEM = "{\"ChipID\":\"%s\",\"Flash\":%u,\"SDK\":\"%s\",\"firmware\":\"" FW_NAME "\",\"version\":\"" FW_VERSION_STRING "\",\"git\":\"%s\",\"CPUMHz\":%u,\"Heap\":%u,\"Uptime\":%u}";
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

  // create and run ESPEM object
  espem = new ESPEM();

  if (espem && espem->begin(embui.paramVariant(FPSTR(V_UART)), embui.paramVariant(FPSTR(V_RX)), embui.paramVariant(FPSTR(V_TX))) ){
    if ( espem->tsSet( embui.paramVariant(FPSTR(V_EPOOLSIZE)), embui.paramVariant(FPSTR(V_SMPL_PERIOD)) ) ){
      espem->set_collector_state(mcstate_t::MC_RUN);
    }
    espem->setEnergyOffset(embui.paramVariant(FPSTR(V_EOFFSET)));
  }

  embui.server.on(PSTR("/fw"), HTTP_GET, [](AsyncWebServerRequest *request){
    wver(request);
  });


  #ifdef USE_FTP
      ftp_setup(); // запуск ftp-сервера
  #endif

  //sync_parameters();    // sync UI params

  embui.setPubInterval(WEBUI_PUBLISH_INTERVAL);
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
#ifdef GIT_REV
    GIT_REV,
#else
    "-",
#endif
    ESP.getCpuFreqMHz(),
    ESP.getFreeHeap(),
    (uint32_t)tp.tv_sec);
#else
  snprintf_P(buff, sizeof(buff), PGverjson,
    ESP.getChipModel(),
    ESP.getFlashChipSize(),
    ESP.getSdkVersion(),
#ifdef GIT_REV
    GIT_REV,
#else
    "-",
#endif
    ESP.getCpuFreqMHz(),
    ESP.getFreeHeap(),
    (uint32_t)tp.tv_sec);
#endif


  request->send(200, FPSTR(PGmimejson), buff );
}
//
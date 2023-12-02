/*  ESPEM - ESP Energy monitor
 *  A code for ESP32 based boards to interface with PeaceFair PZEM PowerMeters
 *  It can poll/collect PowerMeter data and provide it for futher processing in text/json format
 *
 *  (c) Emil Muratov 2017-2022
 *
 */

// Main headers
#include "main.h"
#include "espem.h"

#include <EmbUI.h>
#include "interface.h"

extern "C" int clock_gettime(clockid_t unused, struct timespec *tp);


// PROGMEM strings
// sprintf template for json version data
static const char PGverjson[] = "{\"ChipID\":\"%s\",\"Flash\":%u,\"SDK\":\"%s\",\"firmware\":\"" FW_NAME "\",\"version\":\"" FW_VERSION_STRING "\",\"git\":\"%s\",\"CPUMHz\":%u,\"RAM Heap size\":%u,\"RAM Heap free\":%u,\"PSRAM size\":%u,\"PSRAM free\":%u,\"Uptime\":%u}";

// Our instance of espem
Espem *espem = nullptr;

// ----
// MAIN Setup
void setup() {

#ifdef ESPEM_DEBUG
  ESPEM_DEBUG.begin(BAUD_RATE);  // start hw serial for debugging
#endif

  LOG(println, F("Starting EspEM..."));

  // Start framework, load config and connect WiFi
  embui.begin();
  embui_actions_register();

  // create and run ESPEM object
  espem = new Espem();

  if (espem && espem->begin(  embui.paramVariant(V_UART),
                              embui.paramVariant(V_RX),
                              embui.paramVariant(V_TX)) 
                            )
  {
    espem->ds.setEnergyOffset(embui.paramVariant(V_EOFFSET));

    // postpone TimeSeries setup until NTP aquires valid time
    TimeProcessor::getInstance().attach_callback([](){
      espem->set_collector_state(mcstate_t::MC_RUN);
      // we only need that setup once
      TimeProcessor::getInstance().dettach_callback();
    });

  }

  embui.server.on("/fw", HTTP_GET, [](AsyncWebServerRequest *request){ wver(request); });

  embui.setPubInterval(WEBUI_PUBLISH_INTERVAL);
}

// MAIN loop
void loop() {
  embui.handle();
}

// send HTTP responce, json with controller/fw versions and status info
void wver(AsyncWebServerRequest *request) {
  char buff[HTTP_VER_BUFSIZE];

  timespec tp;
  clock_gettime(0, &tp);
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
    ESP.getHeapSize(), ESP.getFreeHeap(),      // RAM
    ESP.getPsramSize(), ESP.getFreePsram(),    // PSRAM
    (uint32_t)tp.tv_sec);


  request->send(200, FPSTR(PGmimejson), buff );
}
//
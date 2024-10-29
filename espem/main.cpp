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

#include "EmbUI.h"
#include "interface.h"
#include "log.h"


// PROGMEM strings
// sprintf template for json version data
static constexpr const char* PGverjson = "{\"ChipID\":\"%s\",\"Flash\":%u,\"SDK\":\"%s\",\"firmware\":\"" FW_NAME "\",\"version\":\"" FW_VERSION_STRING "\",\"git\":\"%s\",\"CPUMHz\":%u,\"RAM Heap size\":%u,\"RAM Heap free\":%u,\"PSRAM size\":%u,\"PSRAM free\":%u,\"Uptime\":%u}";

// Our instance of espem
Espem *espem = nullptr;

// load configuration for espem and start it
void setup_espem();

// ----
// MAIN Setup
void setup() {

#ifdef ESPEM_DEBUG_PORT
  ESPEM_DEBUG.begin(BAUD_RATE);  // start hw serial for debugging
#endif

  LOGI(C_EspEM, println, "Starting EspEM...");

  // Start framework, load config and connect WiFi
  embui.begin();

  // start EspEM object
  setup_espem();

  // attach EmbUI callback actions
  embui_actions_register();

  // status call
  embui.server.on("/fw", HTTP_GET, [](AsyncWebServerRequest *request){ wver(request); });

  // adjust EmbUI's publish rate
  embui.setPubInterval(WEBUI_PUBLISH_INTERVAL);
}

// MAIN loop
void loop() {
  embui.handle();
}

// load configuration for espem and start it
void setup_espem(){
  if (!espem){
    // create and run ESPEM object
    espem = new Espem();
  }

  if (!espem) return;

  if (espem->begin( 
    embui.getConfig()[V_UART] | UART_NUM_1,   // by default UART_NUM_1 uses SECOND uart port on ESP32, not the one that outputs console serial
    embui.getConfig()[V_RX] | -1,
    embui.getConfig()[V_TX] | -1)
  ){
    LOGI(C_EspEM, printf, "attaching to UART:%d\n",embui.getConfig()[V_UART] | UART_NUM_1 );

    espem->ds.setEnergyOffset(embui.getConfig()[V_EOFFSET]);
    LOGI(C_EspEM, printf, "Configured energy offset is:%d\n", embui.getConfig()[V_EOFFSET] | 0 );

    // postpone TimeSeries setup until NTP aquires valid time
    TimeProcessor::getInstance().attach_callback([](){
      LOGI(C_EspEM, println, "Aquired time from NTP, running TimeSeries collector..." );
      espem->set_collector_state(mcstate_t::MC_RUN);
      // we only need that setup once
      TimeProcessor::getInstance().dettach_callback();
    });

  }
}

// send HTTP responce, json with controller/fw versions and status info
void wver(AsyncWebServerRequest *request) {
  char buff[HTTP_VER_BUFSIZE];

  timespec tp;
  clock_gettime(0, &tp);
  snprintf(buff, sizeof(buff), PGverjson,
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


  request->send(200, asyncsrv::T_application_json, buff );
}

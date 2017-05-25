/*  ESPEM - ESP Energy monitor
 *  A code for ESP8266 based boards to interface with PeaceFair PZEM PowerMeters
 *  It can poll/collect PowerMeter data and provide it for futher processing in text/json format
 *
 *  (c) Emil Muratov 2017
 *
 */

// Main headers
#include "main.h"

#ifdef BOARD_WITTY
// Onboard LED for debug
const uint8_t LED_g = 12;    // green LED pin - on, when PZEM initialized
const uint8_t LED_r = 15;    // red LED pin   - blinks while trying to connect to PZEM
const uint8_t LED_b = 13;    // blue LED pin  - on, when WiFi connected
#endif  // BOARD_WITTY


//prepare a struct for meter data
pmeterData pdata;

// ----
// Constructs

#if USE_HWSERIAL
  HardwareSerial hwser(HWSERIAL_PORT);
  PZEM004T pzem(&hwser);  // Connect to PZEM via HW_serial
#else
  PZEM004T pzem(PIN_RX, PIN_TX);  // Connect to PZEM via sw_serial pins
#endif
IPAddress ip(192,168,1,1);      // This one is the default

// Create an instance of the web-server
ESP8266WebServer server(80);

// Create timer object
SimpleTimer timer;

// ----
// MAIN Setup
void setup() {
  #ifdef BOARD_WITTY
  pinMode(LED_r, OUTPUT);
  pinMode(LED_g, OUTPUT);
  pinMode(LED_b, OUTPUT);
  #endif  // BOARD_WITTY

  // fill the struct with pointers
  pdata.voltage = &meterings[0];
  pdata.current = &meterings[1];
  pdata.power   = &meterings[2];
  pdata.energy  = &meterings[3];

  #if HWSERIAL_SWAP
    hwser.swap();  //swap hw_serial pins to gpio13(rx),15(tx)
  #endif

  // start hw serial for debugging
  Serial.begin(115200);


  cfg conf;           // struct for config data
  cfgload(conf);      // Load config from EEPROM
  wifibegin(conf);    // Enable WiFi


  // Restore global vars from config
  poll=conf.cpoll;                // Polling enabled
  poll_period=conf.cpoll_period;  // Polling period
  PF_fix=conf.cpf_fix;            // Power Factor correction

  //Define server "pages"
  server.onNotFound( [](){server.send_P(200, PGmimehtml, PGindex);});  //return index for non-ex pages
  server.on("/getpmdata",  wpmdata);		// text-formatted data-output (Cacti like)
  server.on("/ota",        wota);		// OTA firmware update
  server.on("/ver",    [](){server.send_P(200, PGmimetxt, PGver);});	// FW version
  server.on("/samples",    wsamples);		// json with sampled meter data
  server.on("/cfg", HTTP_GET,  wcfgget);	// get config (json)
  server.on("/cfg", HTTP_POST, wcfgset);	// set config (json)

  // Start the Web-server
  server.begin();
  Serial.println("Server started");

  pzemrdy = pzeminit(); // connect to the power meter

  // set polling scheduler
  if ( poll )  poller_id = timer.setInterval(poll_period*1000, &datapoller );
}


// MAIN loop
void loop() {

  //serve web-client
  server.handleClient();

  // do scheduled tasks
  timer.run();

  yield();
}

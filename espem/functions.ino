/*  ESPEM - ESP Energy monitor
 *  A code for ESP8266 based boards to interface with PeaceFair PZEM PowerMeters
 *  It can poll/collect PowerMeter data and provide it for futher processing in text/json format
 *
 *  (c) Emil Muratov 2017
 *
 */


// Load config from EEPROM into a cfg struct
// If EEPROM data is damaged, than reset to default configuration and update EEPROM
cfg &cfgload( cfg &conf ) {
  EEPROM.begin(sizeof(conf)+4);
  size_t cfgsize = EEPROM_readAny(0, conf);   // Load EEPROM data
  byte ee_crc;
  EEPROM_readAny(cfgsize, ee_crc);            // read crc from eeprom
  EEPROM.end();

  byte _crc = CRC_Any(conf);                  // calculate crc for data
  if ( _crc != ee_crc) {
        Serial.println("Config CRC error, loading defaults");
        cfgReset(conf);                       // reset config to defaults
        cfgsave(conf);                        // save default config to eeprom
  }
  return conf;
}

// Saves config from cfg struct into a EEPROM
void cfgsave( const cfg &conf ) {
  byte _crc = CRC_Any(conf);
  size_t cfgsize = sizeof(conf);
  EEPROM.begin(cfgsize+4);  //reserve some extra bytes for crc and...
  EEPROM_writeAny(0, conf);
  EEPROM_writeAny(cfgsize, _crc);
  EEPROM.end();
}

void onSTAGotIP(WiFiEventStationModeGotIP ipInfo) {

  #ifdef BOARD_WITTY
    digitalWrite(LED_b, HIGH);  // blue led indicates WiFi connection
  #endif  // BOARD_WITTY

  Serial.print("WiFi connected, ip:");
  Serial.println(WiFi.localIP());
  WiFi.mode(WIFI_STA);        // Shutdown internal Access Point

  NTP.begin(NTP_SERVER, TZ, TZ_DL); // Start NTP only after IP network is connected
  NTP.setInterval(NTP_INTERVAL);
}

// Manage network disconnection
void onSTADisconnected(WiFiEventStationModeDisconnected event_info) {
    WiFi.mode(WIFI_AP_STA);   // Enable internal AP if station connection is lost
    NTP.stop(); // NTP sync can be disabled to avoid sync errors
}



// Initialize WiFi
void wifibegin(const cfg &conf) {
  Serial.println("Enabling WiFi");
  if (!conf.cWmode) {   //Monitor station events only if in Client/Auto mode
    Serial.println("Listening for WiFi station events");
    static WiFiEventHandler e1, e2;
    e1 = WiFi.onStationModeGotIP(onSTAGotIP);   // WiFi client gets IP event
    e2 = WiFi.onStationModeDisconnected(onSTADisconnected); // WiFi client disconnected event
  }
  WiFi.hostname(conf.chostname);

  WiFi.mode(conf.cWmode ? WIFI_AP : WIFI_AP_STA);
  WiFi.begin();

  WiFi.softAP(conf.cWssid, conf.cWpwd);
}



// function to read meterings from PZEM object and fill array with data
// read from object 'meter', write to array 'result', correct PF if 'PF_fix'
bool pollMeter(PZEM004T* meter, float result[] , bool fixpf) {

  //check for poll delay timeout
  if ( millis()-lastpoll < polldelay ){return false;};

  if ( not pzemrdy ) { pzemrdy = pzeminit(); yield();}  // reconnect to the power meter

  //make pointer array to funtions
  PZPTMF pzdatafunc[4]= {&PZEM004T::voltage, &PZEM004T::current, &PZEM004T::power, &PZEM004T::energy};

  float newData[4];

  //fill the array with new meter data
  for ( uint8_t i = 0; i != 4; ++i) {

      #ifdef BOARD_WITTY
        digitalWrite(LED_g, HIGH);	// blink green led when reading meter data
      #endif  // BOARD_WITTY

      newData[i] = (meter->*pzdatafunc[i])(ip);

      #ifdef BOARD_WITTY
        digitalWrite(LED_g, LOW);
      #endif  // BOARD_WITTY

      yield();  	// feed the watchdog and make other code happy after a long read operation
      if ( newData[i] < 0) { pzemrdy = false; return false; }         // return on error reading meter
  }

  // correct pF value if required   I = P/U
  if (fixpf && (pfcalc(newData) > 1.1) ) newData[1] = newData[2] / newData[0];

  //copy new data to the array provided
  std::copy(&newData[0], &newData[4], result);

  lastpoll=millis();	//reset poll timer
  return true;
}

// function calls pollMeter()
// it must return void to make timer lib happy
void datapoller() {
  pollMeter(&pzem, meterings, PF_fix);

  if ( !POLL_SAMPLES ) return;

  // store samples in RAM ring buffer
  std::copy(&meterings[0], &meterings[3], samples[s_counter]);
  if ( ++s_counter == POLL_SAMPLES ) s_counter=0;
}

//Do some housekeeping tasks while waiting for something
void sleep(unsigned long delaytime) {
  unsigned long t = millis();
  for(;;) {
    if ( (millis() - t ) > delaytime ){return;}
    yield();
  }
}


// Initialize powermeter connection by setting ip addr
bool pzeminit() {

  if (pzemrdy) return true;	// nothing to do

  #ifdef BOARD_WITTY
    digitalWrite(LED_r, HIGH);	// turn red led on if no connection to PM
  #endif  // BOARD_WITTY


  //check for poll delay timeout
  if ( millis()-lastpoll < polldelay ){return false;};

  // Set PowerMeter address
  Serial.println("Init PZEM...");
  // reconnect to the power meter
  if ( not pzem.setAddress(ip) ) {
       return false;
  }

  lastpoll=millis();	//reset poll timer

  yield();
  Serial.println("OK!");
  #ifdef BOARD_WITTY
    digitalWrite(LED_r, LOW);
  #endif  // BOARD_WITTY

  return true;
}


// make a string with last-polled data (cacti poller format)
String& mktxtdata ( String& txtdata) {
    txtdata = "U:";
    txtdata += *pdata.voltage;
    txtdata += " I:";
    txtdata += *pdata.current;
    txtdata += " P:";
    txtdata += *pdata.power;
    txtdata += " W:";
    txtdata += *pdata.energy;
    return txtdata;
}


// makes a pseudo-json from samples array (1 raw)
String& mkjsondata( const float result[], const unsigned long tstamp, String& jsn, const bool W ) {
  jsn = "{\"t\":";     jsn += tstamp;
  jsn += "000,\"U\":"; jsn += result[0];    // Voltage (trailing 000 required for js timestamp in ms)
  jsn += ",\"I\":";    jsn += result[1];    // Current
  jsn += ",\"P\":";    jsn += result[2];    // Power
  if (W) { jsn += ",\"W\":";	jsn += result[3];}   // Energy
  jsn += ",\"pF\":";   jsn += pfcalc(result); // Add PowerFactor calculation
  jsn += "},\n";
  return jsn;
}

// calculate PowerFactor from UIP values
float pfcalc(const float result[]) {
  // result[] must be an array of UIP
  // PF = P / UI
  if (result[0] == 0 || result[1] == 0) return 0;
  return( result[2] / result[0] / result[1]);
}


// OTA update
void otaclient( const String& url) {

  timer.disable(poller_id);   // disable poller
  Serial.println("Trying OTA Update...");
  t_httpUpdate_return ret = ESPhttpUpdate.update(url, OTA_ver);
  switch(ret) {
    case HTTP_UPDATE_FAILED:
        Serial.println("[update] Update failed");
        break;
    case HTTP_UPDATE_NO_UPDATES:
        Serial.println("[update] No Updates");
        break;
    case HTTP_UPDATE_OK:
        Serial.println("[update] Update OK"); // may reboot the ESP
        break;
    }
}

// web-pages


// pmdata web-page
// Returns an http-response with Powermeter data from pdata struct
// If we are not in a poll-mode than make one poll request to update the struct with data
void wpmdata() {
  if ( !poll && !pollMeter(&pzem, meterings, PF_fix) )  {     //get the data from meter if required
    server.send_P(503, PGmimetxt, PGdre );
    return;
  }
    String data;
    server.send(200, FPSTR(PGmimetxt), mktxtdata(data) );
}

// try OTA Update
void wota() {
    String otaurl;
    if(server.hasArg("url")) {    // update via url arg if provided
        otaurl = server.arg("url");
    } else {                      // otherwise use url from EEPROM config
        cfg conf;
        cfgload(conf);
        otaurl = conf.cOTAurl;
    }
    server.send_P(200, PGmimetxt, PGota );
    otaclient(otaurl);
}

/*  Webpage: Provide json encoded config data
 *  Get data from EEPROM and return it in via json
 */
void wcfgget() {
  cfg conf;           // struct for config data
  cfgload(conf);      // Load config from EEPROM
  cfg2json(conf);     // send it as json
}

// dumb func for timer
void espreboot() {
  Serial.println("Reboot initiated");
  ESP.restart();
}

/*  Webpage: Update config in EEPROM
 *  Use form-posted json object to update data in EEPROM
 */
void wcfgset() {
  //Serial.print("HTTP cfg update:"); Serial.println(server.arg("plain"));    //Debug

  //bool cfgupdate( const String& json) { //Using String gives big mem overhead
  const size_t bufferSize = 2*JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(7) + 180;
  StaticJsonBuffer<bufferSize> buff;
  JsonObject& jsoncfg = buff.parseObject(server.arg("plain"));

  if (!jsoncfg.success()) {
    server.send_P(500, PGmimejson, PGdre);   // return http-error if json is unparsable
      return;
  }
  //jsoncfg.printTo(Serial); //Debug

  cfg conf;           // struct for config data
  cfgload(conf);      // Load config from EEPROM

  snprintf(conf.chostname, sizeof conf.chostname, "%s", jsoncfg["wH"].as<const char*>());
  conf.cpf_fix = jsoncfg.containsKey("pf") ? 1 : 0;
  conf.cpoll = jsoncfg.containsKey("pM") ? 1 : 0;
  conf.cpoll_period = atoi(jsoncfg["pP"].as<const char*>());
  snprintf(conf.cOTAurl, sizeof(conf.cOTAurl), "%s", jsoncfg["uU"].as<const char*>());

  if (jsoncfg.containsKey("wA")) {             //We have new WiFi settings
      conf.cWmode = atoi(jsoncfg["wM"].as<const char*>());
      if (conf.cWmode) {                      // we have non-station config => save SSID/passwd to eeprom
          snprintf(conf.cWssid, sizeof(conf.cWssid), "%s", jsoncfg["wS"]);
          snprintf(conf.cWpwd,  sizeof(conf.cWpwd),  "%s", jsoncfg["wP"]);  // save password only for internal AP-mode, but never for client
      } else {                                // try to connect to the AP with a new settings
            WiFi.mode(WIFI_AP_STA);           // Make sure we are in a client mode
            WiFi.begin(jsoncfg["wS"].as<const char*>(), jsoncfg["wP"].as<const char*>()); // try to connect to the AP, event scheduler will
                                                                                          // take care of disabling internal AP-mode if success
      }
  }

  cfgsave(conf);    // Update EEPROM
  cfg2json(conf);   // return current config as serialised json

  if (jsoncfg.containsKey("uA")) {
      otaclient(jsoncfg["uU"].as<String>());  //Initiate OTA update
  } else {                                                  // there was a config update, so I need either do all the tasks to update setup
      int restartId = timer.setTimeout(20000, &espreboot);  // or reboot it all...
  }                                                          // just give it some time to try new WiFi setup if required

}

// Takes config struct and sends it as json via http
void cfg2json(const cfg &conf) {
  char buff[sizeof(conf) + 50];      // let's keep it simple and do some sprintf magic
  sprintf_P(buff, PGcfgjson, conf.chostname, conf.cWmode, conf.cWssid, conf.cpf_fix, conf.cpoll, conf.cpoll_period, conf.cOTAurl);
  Serial.print("EEPROM cfg:"); Serial.println(buff);  //Debug
  server.send(200, FPSTR(PGmimejson), buff );
}

// return json-formatted response for in-RAM sampled data
void wsamples() {

  if ( !POLL_SAMPLES ) {
    server.send_P(503, PGmimetxt, PGsmpld );
    return;
  }

  // json response maybe pretty large and needs too much of a precious ram to store it in a temp 'string'
  // So I'm going to generate it on-the-fly and stream to client with chunks
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);           // I do not know the length in advance
  server.sendHeader(FPSTR(PGacao), "*");  // CORS headers
  server.send(200, FPSTR(PGmimejson), "");                  // empty body enables chunked encoding

  unsigned long meter_time = now() - (millis()-lastpoll)/1000;  // find out timestamp for last sample

  // So let's make some pseudo json responder
  server.sendContent("[");                  // open json array
  for ( uint16_t i = 0; i != POLL_SAMPLES; ++i) {
    String jsn;
    mkjsondata(samples[(s_counter+i) % POLL_SAMPLES], meter_time - (POLL_SAMPLES-i)*poll_period, jsn );
    if (i==POLL_SAMPLES-1) {
        jsn.remove(jsn.length() - 2); // if it's the last one than chop off ',\n'
        jsn += "]";                      //  and close json with ']'
    };
    server.sendContent(jsn);    // stream chunk to the client
  }
  server.sendContent(""); // send empty chunk and notify the client that there will be no more data
  server.client().stop(); // Force connection close 'cause the were no content length header
}


// return json with espem status
void wver() {

  char buff[150];
  char* espmver = (char*) malloc(strlen_P(PGver)+1);
  strcpy_P(espmver, PGver);

  snprintf_P(buff, sizeof(buff), PGverjson,
		ESP.getChipId(),
		ESP.getFlashChipSize(),
		ESP.getCoreVersion().c_str(),
		system_get_sdk_version(),
		espmver,
		ESP.getCpuFreqMHz(),
		ESP.getFreeHeap(),
		NTP.getUptime() );

  server.send(200, FPSTR(PGmimejson), buff );
}




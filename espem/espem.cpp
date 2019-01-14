/* 
 *  ESPEM - ESP Energy monitor
 *  A code for ESP8266 based boards to interface with PeaceFair PZEM PowerMeters
 *  It can poll/collect PowerMeter data and provide it for further processing in 
 *  text/JSON format
 *  Author           : Emil Muratow
 * 
 *  This file        : espem.cpp
 *  This file Author : Emil Muratow
 *  
 *  Description      : Entry point functions
 *  (c) Emil Muratov 2017
 * 
 */

// Main headers
#include "main.h"
#include "Globals.h"
#include "Configuration.h"
#include "BoardWitty.h"
extern Globals global;
/**
 * MAIN Setup
 */
void setup()
{
    BoardWitty::initialize();
    // fill the struct with pointers

#if HWSERIAL_SWAP
	global.m_pHWSer->swap(); //swap hw_serial pins to gpio13(rx),15(tx)
#endif

    // start hw serial for debugging
    Globals::SerialBegin(115200);


    Configuration::Load(); // Load config from EEPROM
    wifibegin(Configuration::getConfig()); // Enable WiFi


    // Restore global vars from config
    global.poll = Configuration::getConfig().cpoll; // Polling enabled
    global.poll_period = Configuration::getConfig().cpoll_period; // Polling period
    global.PF_fix = Configuration::getConfig().cpf_fix; // Power Factor correction

    //Define server "pages"
    global.server().onNotFound([]() {
        global.server().send_P(200, EspemHttp::PGmimehtml, EspemHttp::PGindex);
    }); //return index for non-ex pages
    global.setHttpServerCallBack("/getpmdata", EspemHttp::wpmdata);  // text-formatted data-output (Cacti like)
    global.setHttpServerCallBack("/ota",       EspemHttp::wota);     // OTA firmware update
    global.setHttpServerCallBack("/ver",       EspemHttp::wver);     // version and status info
    global.setHttpServerCallBack("/samples",   EspemHttp::wsamples); // json with sampled meter data
    global.setHttpServerCallBack("/cfg", HTTP_GET,  EspemHttp::wcfgget); // get config (json)
    global.setHttpServerCallBack("/cfg", HTTP_POST, EspemHttp::wcfgset); // set config (json)

    // Start the Web-server
    global.server().begin();
    Globals::SerialPrintln("Server started");

    global.pzemrdy = global.initPowerMeter(); // connect to the power meter

    // set polling scheduler
    global.enablePoller(&datapoller);
}


/**
 * MAIN loop
 */
void loop()
{
    //serve web-client
    global.server().handleClient();
    // do scheduled tasks
    global.timer.run();
    // 
    yield();
}

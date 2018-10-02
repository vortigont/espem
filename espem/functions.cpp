/* 
 *  ESPEM - ESP Energy monitor
 *  A code for ESP8266 based boards to interface with PeaceFair PZEM PowerMeters
 *  It can poll/collect PowerMeter data and provide it for further processing in 
 *  text/JSON format
 *  Author           : Emil Muratow
 * 
 *  This file        : functions.cpp
 *  This file Author : Emil Muratow
 *  
 *  Description      : Call back functions implementation
 *  (c) Emil Muratov 2017
 * 
 */
#include "main.h"
#include "Globals.h"
#include "BoardWitty.h"

Globals global; // Some global values and functions

/**
 * 
 * @param ipInfo
 */
void onSTAGotIP(WiFiEventStationModeGotIP ipInfo)
{
    BoardWitty::triggerLight(BoardWitty::LED_b, true);

    Globals::SerialPrint("WiFi connected, ip:");
    Globals::SerialPrintln(WiFi.localIP().toString().c_str());
    WiFi.mode(WIFI_STA); // Shutdown internal Access Point

    NTP.begin(NTP_SERVER, TZ, TZ_DL); // Start NTP only after IP network is connected
    NTP.setInterval(NTP_INTERVAL);
}

/**
 * Manage network disconnection
 * @param event_info
 */
void onSTADisconnected(WiFiEventStationModeDisconnected event_info)
{
    WiFi.mode(WIFI_AP_STA); // Enable internal AP if station connection is lost
    NTP.stop(); // NTP sync can be disabled to avoid sync errors
}

/**
 * Initialize WiFi
 * @param conf - configuration to set
 */
void wifibegin(const cfg &conf)
{
    Globals::SerialPrintln("Enabling WiFi");
    if (!conf.cWmode)   //Monitor station events only if in Client/Auto mode
    {
        Globals::SerialPrintln("Listening for WiFi station events");
        static WiFiEventHandler e1, e2;
        e1 = WiFi.onStationModeGotIP(onSTAGotIP); // WiFi client gets IP event
        e2 = WiFi.onStationModeDisconnected(onSTADisconnected); // WiFi client disconnected event
    }
    WiFi.hostname(conf.chostname);

    WiFi.mode(conf.cWmode ? WIFI_AP : WIFI_AP_STA);
    WiFi.begin();

    WiFi.softAP(conf.cWssid, conf.cWpwd);
}

/**
 * data poll call back function
 * acquires data from a hardware
 */
void datapoller() 
{
    if (!POLL_SAMPLES) 
        return;
    //check for poll delay timeout
    if (millis() - global.lastpoll < POLL_DELAY) {
        return ;//false;
    };

    global.initPowerMeter();

    //make pointer array to funtions
    float newData[4];
    bool isdata = false; // non-zero data from meter

    //fill the array with new meter data
    for (uint8_t i = 0; i != 4; ++i) 
    {
        BoardWitty::triggerLight(BoardWitty::LED_g, true);
        switch(i)
        {
            case 0:
                newData[i] = global.pzem()->voltage(global.ip());
                break;
            case 1:
                newData[i] = global.pzem()->current(global.ip());
                break;
            case 2:
                newData[i] = global.pzem()->power(global.ip());
                break;
            case 3:
                newData[i] = global.pzem()->energy(global.ip());
                break;
        }
        BoardWitty::triggerLight(BoardWitty::LED_g, false);

        if ((newData[i] - Globals::cfZero) < Globals::cfPrec)
        {
            global.pzemrdy = false;
            return ;//false;
        } // return on error reading meter
        isdata = newData[i] || isdata;
    }

    // consider it as an error if all data from meter was equal to zero
    if (!isdata) return ;//false;
    // correct pF value if required   I = P/U
    if (global.PF_fix && (Globals::pfcalc(newData) > 1.1)) 
    {
        newData[1] = newData[2] / newData[0];
    }

    //copy new data to the array provided
    global.copySamples(newData, global.getCounter(), 3);

    global.lastpoll = millis(); //reset poll timer

    global.updateCounter();
}

/**
 * yet another sleep implementation
 * Do some housekeeping tasks while waiting for something
 * 
 * @param millseconds - milliseconds to sleep
 */
#ifdef  _USE_MILLI_SLEEP_
void milli_sleep(unsigned long millseconds)
{
    unsigned long t = millis();
    for (;;) {
        if ((millis() - t) > millseconds) {
            return;
        }
        yield();
    }
}
#endif

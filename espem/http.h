/* 
 *  ESPEM - ESP Energy monitor
 *  A code for ESP8266 based boards to interface with PeaceFair PZEM PowerMeters
 *  It can poll/collect PowerMeter data and provide it for further processing in 
 *  text/JSON format
 * 
 *  File:        http.h
 *  Author:      Emil Muratow
 * 
 *  Description: HTTP-pages related stuff
 * 
 *  (c) Emil Muratov 2017
 */
#ifndef _ESPEM_HTTP_H_
#define _ESPEM_HTTP_H_
struct   EspemHttp   {
    static const char PGver[];
    static const char PGmimetxt[];
    static const char PGmimehtml[];
    static const char PGmimejson[];
    static const char PGsmpld[];
    static const char PGdre[];
    static const char PGacao[];
    static const char PGota[];
    static const char PGcfgjson[];
    static const char PGverjson[];
    static const char PGindex[];
    static void wpmdata();
    static void wota();
    static void wver();
    static void wsamples();
    static void wcfgget();
    static void wcfgset();
    static void convertAndSendCfg();
};


#endif
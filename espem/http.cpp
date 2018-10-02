/* 
 *  ESPEM - ESP Energy monitor
 *  A code for ESP8266 based boards to interface with PeaceFair PZEM PowerMeters
 *  It can poll/collect PowerMeter data and provide it for further processing in 
 *  text/JSON format
 * 
 *  File:        http.cpp
 *  Author:      Emil Muratow
 * 
 *  Description: HTTP server call back functions
 * 
 *  (c) Emil Muratov 2017
 */
#include "main.h"
#include "http.h"
#include "Globals.h"
#include "Configuration.h"

// constants
const char PROGMEM EspemHttp::PGver[] = PMeterESP_VER;

// content types
const char PROGMEM EspemHttp::PGmimetxt[]  = "text/plain";
const char PROGMEM EspemHttp::PGmimehtml[] = "text/html";
const char PROGMEM EspemHttp::PGmimejson[] = "application/json";

// HTTP responce messages
const char PROGMEM EspemHttp::PGsmpld[] = "Sampling disabled";
const char PROGMEM EspemHttp::PGdre[] = "Data read error";
const char PROGMEM EspemHttp::PGacao[] = "Access-Control-Allow-Origin";
const char PROGMEM EspemHttp::PGota[] = "OTA update in progress...";

// sprintf template for json config data
const char PROGMEM EspemHttp::PGcfgjson[] = "{\"wH\":\"%s\",\"wM\":%u,\"wS\":\"%s\",\"pf\":[\"%u\"],\"pM\":[\"%u\"],\"pP\":%u,\"uU\":\"%s\"}";

// sprintf template for json version data
const char PROGMEM EspemHttp::PGverjson[] = "{\"ChipID\":\"%x\",\"FlashSize\":%u,\"SDK\":\"%s\",\"espem\":\"%s\",\"CPUclk\":%u,\"Heap\":%u,\"Uptime\":%ld,}";


// Index page with config form
// (this is a minimized version)
const char PROGMEM EspemHttp::PGindex[] = "===( \
<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><title>ESP - PZEM PowerMeter</title><style>html,body{font-family:Arial,Helvetica,sans-serif;font-size:14px;color:white;margin:auto;background:#000}.bd{width:95%;position:relative;margin:auto}h4{text-align:center;margin:-2px 0 1em 0;background:#326}.page{width:100%}.sect{float:left;padding:5px;border:1px solid #6363dd;width:300px}.itm{clear:both;text-align:right;line-height:25px}label{float:left;padding-right:5px}</style></head> \
<body><div class=\"bd\"><h3>ESP Peacefair PowerMeter</h3><div class=\"page\" style=\"position:relative;display:flex;\"> <div id=\"info\" class=\"sect\"> <h4>ESP PZEM info</h4> Sometext here </div><div id=\"cfg\" class=\"sect\"> <h4>WiFi config</h4> <form id=\"fcfg\"><div class=\"itm\"><label for=\"wH\">ESP Hostname</label><input type=\"text\" name=\"wH\" maxlength=\"15\"></div><div class=\"itm\"><label for=\"wM\">WiFi Mode</label><select name=\"wM\"><option value=\"0\">Client/Auto</option><option value=\"1\">Access Point</option></select></div><div class=\"itm\"><label for=\"wS\">SSID</label><input type=\"text\" name=\"wS\" maxlength=\"15\"></div><div class=\"itm\"><label for=\"wP\">Password</label><input type=\"password\" name=\"wP\" maxlength=\"15\"></div><div class=\"itm\"><label for=\"wA\">Apply WiFi config</label><input type=\"checkbox\" name=\"wA\" value=\"1\"></div><hr> <h4>PZEM config</h4> <div class=\"itm\"><label for=\"pf\">Power Factor fix</label><input type=\"checkbox\" name=\"pf\" value=\"1\"></div><div class=\"itm\"><label for=\"pM\">Enable Poll Mode</label><input type=\"checkbox\" name=\"pM\" value=\"1\"></div><div class=\"itm\"><label for=\"pP\">Poll period</label><input name=\"pP\" type=\"number\" min=\"5\" max=\"240\" step=\"1\" style=\"width: 50px;\"/></div><hr><h4>OTA FW update</h4><div class=\"itm\"><label for=\"uU\">OTA update URL</label><input type=\"text\" name=\"uU\" maxlength=\"80\"></div><div class=\"itm\"><label for=\"uA\">Update now</label><input type=\"checkbox\" name=\"uA\" value=\"1\"></div><div class=\"itm\"><button class=\"btn\" type=\"submit\">Save</button></div></form></div></div></div> \
<script type=\"text/javascript\">var formurl=\"cfg\",formid=\"fcfg\";const handleFormSubmit=a=>{a.preventDefault();const b=formToJSON(form.elements);ajax_request(formurl,\"POST\",b,function(a){process_resp(a)});alert(\"Saving config. Autoreboot in 20 sec...\")},formToJSON=a=>[].reduce.call(a,(a,b)=>{isValidElement(b)&&isValidValue(b)&&(isCheckbox(b)?a[b.name]=(a[b.name]||[]).concat(b.value):isMultiSelect(b)?a[b.name]=getSelectValues(b):a[b.name]=b.value);return a},{}),isValidElement=a=>a.name&&a.value,isValidValue=a=>![\"checkbox\",\"radio\"].includes(a.type)||a.checked,isCheckbox=a=>\"checkbox\"===a.type,isMultiSelect=a=>a.options&&a.multiple,getSelectValues=a=>[].reduce.call(a,(a,b)=>b.selected?a.concat(b.value):a,[]),ajax_request=(a,b,c,d)=>{xhr=new XMLHttpRequest;\"GET\"==b&&(a=a+\"?data=\"+encodeURIComponent(JSON.stringify(c)));xhr.open(b,a,!0);xhr.setRequestHeader(\"Content-type\",\"application/json\");xhr.onreadystatechange=function(){4==xhr.readyState&&200==xhr.status&&d(JSON.parse(xhr.responseText))};\"GET\"==b?xhr.send():xhr.send(JSON.stringify(c))},tickbox=(a,b)=>{if(isCheckbox(a)&&a.value===b)return a.checked=!0,!0;return!1},process_resp=a=>{var b=document.getElementById(formid);b.reset();for(var c in a)if(\"[object Array]\"===Object.prototype.toString.call(a[c]))for(var d in a[c])if(b.elements[c].length)for(var e in b.elements[c])tickbox(b.elements[c][e],a[c][d]);else tickbox(b.elements[c],a[c][d]);else b.elements[c].value=a[c]},form=document.getElementById(formid);form.addEventListener(\"submit\",handleFormSubmit),window.onload=function(){ajax_request(formurl,\"GET\",\"\",function(a){process_resp(a)})};</script></body></html> \
)===";

extern Globals global;
// web-pages

/**
 * pmdata web-page
 * Returns an http-response with Powermeter data from pdata struct
 * If we are not in a poll-mode than make one poll request to update the struct with data
 */
void EspemHttp::wpmdata()
{
    if (!global.poll) //get the data from meter if required
    {
        datapoller();
        global.server().send_P(503, EspemHttp::PGmimetxt, EspemHttp::PGdre);
        return;
    }
    String data;
    global.server().send(200, FPSTR(EspemHttp::PGmimetxt), global.mktxtdata(data));
}

/**
 * OTA update
 * @param url
 */
void otaclient(const String& url)
{
    global.disablePoller(); // disable poller
    Globals::SerialPrintln("Trying OTA Update...");
    t_httpUpdate_return ret = ESPhttpUpdate.update(url, OTA_ver);
    switch (ret) {
        case HTTP_UPDATE_FAILED:
            Globals::SerialPrintln("[update] Update failed");
            break;
        case HTTP_UPDATE_NO_UPDATES:
            Globals::SerialPrintln("[update] No Updates");
            break;
        case HTTP_UPDATE_OK:
            Globals::SerialPrintln("[update] Update OK"); // may reboot the ESP
            break;
    }
}

/**
 * try OTA Update
 */
void EspemHttp::wota()
{
    String otaurl;
    if (global.server().hasArg("url"))  // update via url arg if provided
    {
        otaurl = global.server().arg("url");
    }
    else
    { // otherwise use url from EEPROM config
        Configuration::Load();
        otaurl = Configuration::getConfig().cOTAurl;
    }
    global.server().send_P(200, EspemHttp::PGmimetxt, EspemHttp::PGota);
    otaclient(otaurl);
}

/**
 * Takes config struct and sends it as json via http
 * @param conf
 */
void EspemHttp::convertAndSendCfg()
{
    char buff[sizeof (cfg) + 50]; // let's keep it simple and do some sprintf magic
    Configuration::cfg2json(EspemHttp::PGcfgjson, buff, sizeof (cfg) + 50);
    Globals::SerialPrint("EEPROM cfg:");
    Globals::SerialPrintln(buff); //Debug
    global.server().send(200, FPSTR(EspemHttp::PGmimejson), buff);
}

/**
 *  Webpage: Provide json encoded config data
 *  Get data from EEPROM and return it in via json
 */
void EspemHttp::wcfgget()
{
    Configuration::Load();// Load config from EEPROM
    EspemHttp::convertAndSendCfg();  // send it as json
}

/**
 * Webpage: Update config in EEPROM
 * Use form-posted json object to update data in EEPROM
 */
void EspemHttp::wcfgset()
{
    const size_t bufferSize = 2 * JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(7) + 180;
    StaticJsonBuffer<bufferSize> buff;
    JsonObject& jsoncfg = buff.parseObject(global.server().arg("plain"));

    if (!jsoncfg.success())
    {
        global.server().send_P(500, EspemHttp::PGmimejson, EspemHttp::PGdre); // return http-error if json is unparsable
        return;
    }
    Configuration::Load(); // Load config from EEPROM

    Configuration::initFrom(jsoncfg);
    Configuration::Save(); // Update EEPROM
    EspemHttp::convertAndSendCfg(); // return current config as serialised json

    if (jsoncfg.containsKey("uA"))
    {
        otaclient(jsoncfg["uU"].as<String>()); //Initiate OTA update
    }
    else
    { // there was a config update, so I need either do all the tasks to update setup
        global.restartPoller(); // int restartId = timer.setTimeout(20000, &espreboot); // or reboot it all...
    } // just give it some time to try new WiFi setup if required

}

/**
 * return json-formatted response for in-RAM sampled data
 */
void EspemHttp::wsamples()
{
    if (!POLL_SAMPLES)
    {
        global.server().send_P(503, EspemHttp::PGmimetxt, EspemHttp::PGsmpld);
        return;
    }

    // json response maybe pretty large and needs too much of a precious ram to store it in a temp 'string'
    // So I'm going to generate it on-the-fly and stream to client with chunks
    global.server().setContentLength(CONTENT_LENGTH_UNKNOWN); // I do not know the length in advance
    global.server().sendHeader(FPSTR(EspemHttp::PGacao), "*"); // CORS headers
    global.server().send(200, FPSTR(EspemHttp::PGmimejson), ""); // empty body enables chunked encoding

    unsigned long meter_time = global.getMeterTime(); // find out timestamp for last sample

    // So let's make some pseudo json responder
    global.server().sendContent("["); // open json array
    for (uint16_t i = 0; i != POLL_SAMPLES; ++i)
    {
        String jsn;
        global.mkjsondata(i, meter_time, jsn);
        if (i == POLL_SAMPLES - 1)
        {
            //jsn.remove(jsn.length() - 2); // if it's the last one than chop off ',\n'
            jsn += "]"; //  and close json with ']'
        }
        global.server().sendContent(jsn); // stream chunk to the client
    }
    global.server().sendContent(""); // send empty chunk and notify the client that there will be no more data
    global.server().client().stop(); // Force connection close 'cause the were no content length header
}

/**
 * return json with espem status
 */
void EspemHttp::wver()
{
    char buff[150];
    char* espmver = (char*) malloc(strlen_P(EspemHttp::PGver) + 1);
    strcpy_P(espmver, EspemHttp::PGver);

    snprintf_P(buff, sizeof (buff), EspemHttp::PGverjson,
            ESP.getChipId(),
            ESP.getFlashChipSize(),
#ifndef DISABLE_SYSTEM_GET_SDK_VER
            system_get_sdk_version(),            
#else
            "SDK version is disabled",
#endif  // DISABLE_SYSTEM_GET_SDK_VER
            espmver,
            ESP.getCpuFreqMHz(),
            ESP.getFreeHeap(),
            NTP.getUptime());

    global.server().send(200, FPSTR(EspemHttp::PGmimejson), buff);
}

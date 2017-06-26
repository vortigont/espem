/*  ESPEM - ESP Energy monitor
 *  A code for ESP8266 based boards to interface with PeaceFair PZEM PowerMeters
 *  It can poll/collect PowerMeter data and provide it for futher processing in text/json format
 *
 *  (c) Emil Muratov 2017
 *
 */

#define HWSERIAL_PORT UART0

// Sketch configuration
#include "config.h"
// HTTP related def's
#include "http.h"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
// OTA Updates
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

//#include <SoftwareSerial.h> // Must be a special lib for ESP!	https://github.com/plerup/espsoftwareserial
#include <PZEM004T.h>		// Peacefair PZEM-004T Energy monitor https://github.com/vortigont/PZEM004T
				// a forked version of oleh's lib with a fix for esp platform
				// (should switch back if merged to oleh's master https://github.com/olehs/PZEM004T)

#include <SimpleTimer.h>     	// Simple timers        https://github.com/jfturcot/SimpleTimer

// NTP time
#include <TimeLib.h>
#include <NtpClientLib.h>		// this lib uses internal Espressif SDK SNTP    https://github.com/gmag11/NtpClient
#include <EEPROM.h>
#include <ArduinoJson.h>


// ========= Variables

// This vars are stored in EEPROM
struct cfg {
    char chostname[16];             // WiFi hostname
    byte cWmode;                    // WiFi mode 0: Station/Auto, 1: AccessPoint
    char cWssid[16];                // WiFi SSID
    char cWpwd[16];                 // WiFi passwd
    byte cpoll;                     // Polling enabled
    byte cpoll_period;              // Polling period
    byte cpf_fix;                   // Power Factor correction
    char cOTAurl[80];               // OTA URL
};


// Some global vars
bool poll;                          // Polling enabled
uint8_t poll_period;                // Polling period
bool PF_fix;                        // Power Factor correction

const uint16_t polldelay = 2000;    // meter polling interval delay ms
unsigned long lastpoll = 0;         // meter last poll time
bool pzemrdy = false;               // meter available
int poller_id = 0;                  // scheduler ID for poller task
float meterings[4] = {0.0f};        // array with meter data [UIPW]
uint16_t s_counter=0;               // sample counter

//Reset config to defaults
void cfgReset(cfg &conf) {
    sprintf(conf.chostname, "pmeter-%06X", ESP.getChipId());
    sprintf(conf.cWssid, "ESPEM_%06X", ESP.getChipId());
    snprintf(conf.cWpwd, sizeof conf.cWpwd, "%s", WIFI_PASSWD);
    conf.cWmode = 0;
    conf.cpoll   = POLL_MODE;
    conf.cpoll_period=POLL_PERIOD;
    conf.cpf_fix = PF_CORRECTION;
    snprintf(conf.cOTAurl, sizeof conf.cOTAurl, "%s", OTA_url);
}

// Read/Write structs to EEPROM
template <class T> size_t EEPROM_writeAny(size_t ee, const T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    size_t i;
    for (i = 0; i < sizeof(value); i++)
          EEPROM.write(ee++, *p++);
    EEPROM.commit();
    return i;
}

template <class T> size_t EEPROM_readAny(size_t ee, T& value)
{
    byte* p = (byte*)(void*)&value;
    size_t i;
    for (i = 0; i < sizeof(value); i++)
        *p++ = EEPROM.read(ee++);
    return i;
}

// CRC calc for EEPROM config
byte crc8( byte crc, byte ch )
{
    for (uint8_t i = 8; i; i--) {
      uint8_t mix = crc ^ ch;
      crc >>= 1;
      if (mix & 0x01 ) crc ^= 0x8C;
      ch >>= 1;
  }
  return crc;
}

template <class T> byte CRC_Any( T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    byte _crc=0;
    size_t i;
    for (i = 0; i != sizeof(value); i++)
          _crc = crc8(_crc, *p++);
    return _crc;
}

// Storage array for poll samples (UIP - only)
float samples[POLL_SAMPLES ? POLL_SAMPLES : 1][3] = {0.0};  // at least 1 element array
                                                            // WA to make other functions happy

// structure to hold pointers to metering data (do I still need it?)
struct pmeterData {
  float *voltage, *current, *power, *energy;
};

//declare pointer-to-member function for PZEM class
typedef float( PZEM004T::*PZPTMF) (const IPAddress& );

cfg &cfgload( cfg &conf );         // Load config from EEPROM into a cfg struct

void cfgsave( const cfg &conf );   // Saves config from cfg struct into a EEPROM

bool cfgupdate( const String& json);  // Parse json config data and update EEPROM

void wifibegin(const cfg &conf);   // Initialize WiFi

void wpmdata();                    // Returns an http-response with Powermeter data from pdata struct

void otaclient();                  // try OTA Update

void wsamples();                   // return json-formatted response for in-RAM sampled data

void wcfgget();                    // Webpage: Provide json encoded config data

void wcfgset();                    // Webpage: Update config in EEPROM

bool pzeminit();                   // Initialize powermeter connection by setting ip addr

void datapoller();                 // Poll meter for data and store samples

float pfcalc(const float result[]); // calculate PowerFactor from UIP values

// function to read meterings from PZEM object and fill array with data
bool pollMeter(PZEM004T* meter, float result[] , bool PF=PF_CORRECTION);

//Do some housekeeping tasks while waiting for something
void sleep(unsigned long delaytime=0);

// make json string out of array provided
// bool W - include energy counter in json
// todo: provide vector with flags for each field
String& mkjsondata( const float result[], unsigned long tstamp, String& jsn, const bool W=0  );


// This trick makes all embedded serial stuff go away magically
// http://forum.arduino.cc/index.php?topic=36940.0
#if !DEBUG
class NullSerialClass // a do-nothing class to replace Serial
{
 public:
 void begin(int speed) {}
 void println(int x, int type=DEC) {}
 void println(const char *p=NULL) {}
 void print(int x, int type=DEC) {}
 void print(const char *p) {}
 int available() {return 0;}
 int read() {return -1;}
 void flush() {}
} NullSerial;
#define Serial NullSerial
#endif

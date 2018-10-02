/* 
 *  ESPEM - ESP Energy monitor
 *  A code for ESP8266 based boards to interface with PeaceFair PZEM PowerMeters
 *  It can poll/collect PowerMeter data and provide it for further processing in 
 *  text/JSON format
 *  Author           : Emil Muratow
 * 
 *  This file        : Configuration.h
 *  This file Author : Alexey Shtykov
 *  
 *  Description      : Configuration handler class implementation
 *  Created on 20 February 2018, 15:57
 * 
 */

#ifndef _INSTANCECONFIG_H_
#define _INSTANCECONFIG_H_
#ifndef WIFI_CFG_STR_LEN
#define WIFI_CFG_STR_LEN    16
#endif
#ifndef WIFI_CFG_URL_LEN
#define WIFI_CFG_URL_LEN    80
#endif
// This vars are stored in EEPROM
typedef struct _cfg {
    char chostname[WIFI_CFG_STR_LEN];// WiFi hostname
    byte cWmode;                    // WiFi mode 0: Station/Auto, 1: AccessPoint
    char cWssid[WIFI_CFG_STR_LEN];  // WiFi SSID
    char cWpwd[WIFI_CFG_STR_LEN];   // WiFi passwd
    byte cpoll;                     // Polling enabled
    byte cpoll_period;              // Polling period
    byte cpf_fix;                   // Power Factor correction
    char cOTAurl[WIFI_CFG_URL_LEN]; // OTA URL
}cfg;

class Configuration {
public:
    virtual ~Configuration();
    static  void    Reset(cfg& conf);
    static  void    Reset();
    static  void    Load(); // Load config from EEPROM into a cfg struct
    static  void    Save(const cfg &conf); // Saves config from cfg struct into a EEPROM
    static  void    Save();
    static  bool    Update( const String& json);  // Parse json config data and update EEPROM
    static  byte    crc8( byte crc, byte ch );
    static  const cfg&  getConfig() {return Configuration::m_config;};
    static  void    initFrom(const JsonObject& jsoncfg);
    static  void    cfg2json(const char* fmt, char* buf, size_t bufLen);
private:
    Configuration();
    Configuration(const Configuration& orig);
    static  void    copyTo(cfg& conf);
    static  void    initFrom(const cfg& conf);
    static  cfg m_config;
};

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

template <class T> byte CRC_Any( T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    byte _crc=0;
    size_t i;
    for (i = 0; i != sizeof(value); i++)
          _crc = Configuration::crc8(_crc, *p++);
    return _crc;
}

#endif /* _INSTANCECONFIG_H_ */


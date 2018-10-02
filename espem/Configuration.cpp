/* 
 *  ESPEM - ESP Energy monitor
 *  A code for ESP8266 based boards to interface with PeaceFair PZEM PowerMeters
 *  It can poll/collect PowerMeter data and provide it for further processing in 
 *  text/JSON format
 *  Author           : Emil Muratow
 * 
 *  This file        : Configuration.cpp
 *  This file Author : Alexey Shtykov
 *  
 *  Description      : Configuration handler class implementation
 *  Created on 20 February 2018, 15:57
 * 
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "config.h"
#include "main.h"
#include "Configuration.h"
#include "Globals.h"
cfg Configuration::m_config;

/**
 * default constructor
 **/
Configuration::Configuration()
{
}

/**
 * copy constructor
 **/
Configuration::Configuration(const Configuration& orig)
{
}

/**
 * default destructor
 **/
Configuration::~Configuration()
{
}

/**
 * Reset configuration to default
 * @param conf - configuration to reset
 */
void Configuration::Reset(cfg& conf) 
{
    snprintf(conf.chostname, 16, "pmeter-%06X", ESP.getChipId());
    snprintf(conf.cWssid, WIFI_CFG_STR_LEN, "ESPEM_%06X", ESP.getChipId());
    snprintf(conf.cWpwd, WIFI_CFG_STR_LEN, "%s", WIFI_PASSWD);
    conf.cWmode = 0;
    conf.cpoll = POLL_MODE;
    conf.cpoll_period = POLL_PERIOD;
    conf.cpf_fix = PF_CORRECTION;
    snprintf(conf.cOTAurl, WIFI_CFG_URL_LEN, "%s", OTA_url);
}

/**
 * reset default configuration
 */
void Configuration::Reset()
{
    Reset(m_config);
}
/**
 * copy configuration to holder
 * @param conf - configuration holder
 */
void    Configuration::copyTo(cfg& conf)
{
    if(((cfg*)&conf) != ((cfg*)&m_config))
    {
        memcpy(conf.cOTAurl, Configuration::m_config.cOTAurl, WIFI_CFG_URL_LEN);
        memcpy(conf.chostname, Configuration::m_config.chostname, WIFI_CFG_STR_LEN);
        memcpy(conf.cWssid, Configuration::m_config.cWssid, WIFI_CFG_STR_LEN);
        memcpy(conf.cWpwd, Configuration::m_config.cWpwd, WIFI_CFG_STR_LEN);
        conf.cWmode = Configuration::m_config.cWmode;
        conf.cpoll = Configuration::m_config.cpoll;
        conf.cpoll_period = Configuration::m_config.cpoll_period;
        conf.cpf_fix = Configuration::m_config.cpf_fix;
    }
}

/**
 * initialize configuration with input data
 * @param conf - configuration to set
 */
void    Configuration::initFrom(const cfg& conf)
{
    if(((cfg*)&conf) != ((cfg*)&m_config))
    {
        memcpy(Configuration::m_config.cOTAurl, conf.cOTAurl, WIFI_CFG_URL_LEN);
        memcpy(Configuration::m_config.chostname, conf.chostname, WIFI_CFG_STR_LEN);
        memcpy(Configuration::m_config.cWssid, conf.cWssid, WIFI_CFG_STR_LEN);
        memcpy(Configuration::m_config.cWpwd, conf.cWpwd, WIFI_CFG_STR_LEN);
        Configuration::m_config.cWmode = conf.cWmode;
        Configuration::m_config.cpoll = conf.cpoll;
        Configuration::m_config.cpoll_period = conf.cpoll_period;
        Configuration::m_config.cpf_fix = conf.cpf_fix;
    }
}

/**
 * Load config from EEPROM into a cfg struct
 * If EEPROM data is damaged, than reset to default configuration and update EEPROM
 */
void Configuration::Load()
{
    EEPROM.begin(sizeof (Configuration::m_config) + 4);
    size_t cfgsize = EEPROM_readAny(0, Configuration::m_config); // Load EEPROM data
    byte ee_crc;
    EEPROM_readAny(cfgsize, ee_crc); // read crc from eeprom
    EEPROM.end();

    byte _crc = CRC_Any(Configuration::m_config); // calculate crc for data
    if (_crc != ee_crc) {
        Globals::SerialPrintln("Config CRC error, loading defaults");
        Reset(); // reset config to defaults
        Save(); // save default config to eeprom
    }
}

/**
 * Saves configuration from input into a EEPROM
 * @param conf
 */
void Configuration::Save(const cfg &conf)
{
    byte _crc = CRC_Any(conf);
    size_t cfgsize = sizeof (conf);
    EEPROM.begin(cfgsize + 4); //reserve some extra bytes for crc and...
    EEPROM_writeAny(0, conf);
    EEPROM_writeAny(cfgsize, _crc);
    EEPROM.end();
    initFrom(conf);
}

/**
 * save default configuration
 */
void Configuration::Save()
{
    Save(m_config);
}

/**
 * compute crc8 sum on byte
 * @param crc - crc to update
 * @param ch  - byte to add
 * @return new crc8 value
 */
byte Configuration::crc8(byte crc, byte ch) {
    for (uint8_t i = 8; i; i--) {
        uint8_t mix = crc ^ ch;
        crc >>= 1;
        if (mix & 0x01) crc ^= 0x8C;
        ch >>= 1;
    }
    return crc;
}
/**
 * update configuration from JSON string
 * @param json - JSON input string
 * @return true if configuration set successfully
 */
bool Configuration::Update( const String& json)
{
    return false;
}

/**
 * initialize configuration from JSON object
 * @param jsoncfg - JSON input object
 */
void    Configuration::initFrom(const JsonObject& jsoncfg)
{
    snprintf(Configuration::m_config.chostname, WIFI_CFG_STR_LEN, "%s", jsoncfg["wH"].as<const char*>());
    Configuration::m_config.cpf_fix = jsoncfg.containsKey("pf") ? 1 : 0;
    Configuration::m_config.cpoll = jsoncfg.containsKey("pM") ? 1 : 0;
    Configuration::m_config.cpoll_period = atoi(jsoncfg["pP"].as<const char*>());
    snprintf(Configuration::m_config.cOTAurl, WIFI_CFG_URL_LEN, "%s", jsoncfg["uU"].as<const char*>());

    if (jsoncfg.containsKey("wA"))  //We have new WiFi settings
    {
        Configuration::m_config.cWmode = atoi(jsoncfg["wM"].as<const char*>());
        if (Configuration::m_config.cWmode)    // we have non-station InstanceConfig::m_configig => save SSID/passwd to eeprom
        {
            snprintf(Configuration::m_config.cWssid, WIFI_CFG_STR_LEN, "%s", jsoncfg["wS"].as<String>().c_str());
            // save password only for internal AP-mode, but never for client
            snprintf(Configuration::m_config.cWpwd, WIFI_CFG_STR_LEN, "%s", jsoncfg["wP"].as<String>().c_str()); 
        }
        else
        { // try to connect to the AP with a new settings
            WiFi.mode(WIFI_AP_STA); // Make sure we are in a client mode
            WiFi.begin(jsoncfg["wS"].as<const char*>(), jsoncfg["wP"].as<const char*>()); // try to connect to the AP, event scheduler will
            // take care of disabling internal AP-mode if success
        }
    }
    
}

void Configuration::cfg2json(const char* fmt, char* buf, size_t bufLen)
{
    if(fmt!=NULL && buf!=NULL && bufLen>=(sizeof (Configuration::m_config) + 50))
    {
        sprintf_P(buf, fmt, 
                Configuration::m_config.chostname, 
                Configuration::m_config.cWmode, 
                Configuration::m_config.cWssid, 
                Configuration::m_config.cpf_fix, 
                Configuration::m_config.cpoll, 
                Configuration::m_config.cpoll_period, 
                Configuration::m_config.cOTAurl);
    }
}
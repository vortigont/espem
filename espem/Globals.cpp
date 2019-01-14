/* 
 *  ESPEM - ESP Energy monitor
 *  A code for ESP8266 based boards to interface with PeaceFair PZEM PowerMeters
 *  It can poll/collect PowerMeter data and provide it for further processing in 
 *  text/JSON format
 *  Author           : Emil Muratow
 * 
 *  This file        : Globals.h
 *  This file Author : Alexey Shtykov
 *  
 *  Description      : Global class definition
 *  Created on 20 February 2018, 13:38
 * 
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "main.h"
#include "config.h"
#include "Globals.h"
#include "BoardWitty.h"

const float Globals::cfZero = 0.;
const float Globals::cfPrec = 0.0001;

/**
 * default constructor
 **/
Globals::Globals()
{
    m_Counter   = 0;    // init counter
    lastpoll    = 0;    // meter last poll time
    pzemrdy     = false;// meter available
    poller_id   = 0;    // scheduler ID for poller task
    
    m_nRows = (POLL_SAMPLES ? POLL_SAMPLES : 1);
    m_nCols = 3;
    m_pSamples = new float*[m_nRows];
    for(size_t i=0; i<m_nRows; i++)
    {
        m_pSamples[i] = new float[m_nCols];
    }
    //m_pSamples;//[][3] = {0.0};  // at least 1 element array
    // initialize power data pointers
    ::memset(&m_fMeterings, 0, sizeof(float)*METERING_SIZE);
#if USE_HWSERIAL
    //m_pHWSer = new HardwareSerial(HWSERIAL_PORT);
	HardwareSerial m_pHWSer(HWSERIAL_PORT);
    m_pzem = new PZEM004T(&m_pHWSer);  // Connect to PZEM via HW_serial
#else
    m_pzem = new PZEM004T(PIN_RX, PIN_TX);  // Connect to PZEM via sw_serial pins
#endif
    m_pIp = new IPAddress(192,168,1,1);     // This one is the default
    m_pServer = new ESP8266WebServer(80);   // Create an instance of the web-server
}

/**
 * copy constructor
 **/
Globals::Globals(const Globals& orig)
{
}

/**
 * default destructor
 **/
Globals::~Globals() {
}

/**
 * get sample from circular buffer
 * @param row - row index
 * @param col - column index
 */
float   Globals::getSample(size_t row, size_t col)
{
    if(row<m_nRows && col<m_nCols && m_pSamples!=NULL)
    {
        return m_pSamples[row][col];
    }
    return 0.;
}

/**
 * store sample in circular buffer
 * @param row - row index
 * @param col - column index
 * @param val - value to store
 */
void   Globals::setSample(size_t row, size_t col, float val)
{
    if(row<m_nRows && col<m_nCols && m_pSamples!=NULL)
    {
        m_pSamples[row][col] = val;
    }
}

/**
 * set IP address to hardware
 * @return true if address set successfully
 */
bool Globals::setPzemAddr()
{
    if(m_pzem!=NULL && m_pIp!=NULL)
    {
        return m_pzem->setAddress(*m_pIp);
    }
    return false;
}

/**
 * store current measurement samples in circular buffer
 * @param pSamples - current measurement
 * @param row      - row to store
 * @param count    - count of measurement
 * @return true if measurement samples stored successfully
 */
bool    Globals::copySamples(const float* pSamples, size_t row, size_t count)
{
    if(m_pSamples!=NULL && pSamples!=NULL && 
            row<m_nRows && count<m_nCols)
    {
        if(m_pSamples[row]!=NULL)
        {
            for(size_t i=0; i<count; i++)
            {
                setSample(row, i, pSamples[i]);
            }
            return true;
        }
    }
    return false;
}

/**
 * disable current timer
 */
void    Globals::disablePoller()
{
    timer.disable(poller_id);
}

/**
 * set timer for measurement
 * @param pFunc - callback function
 */
void    Globals::enablePoller(timer_callback pFunc)
{
    if (poll) 
    {
        poller_id = timer.setInterval(poll_period * 1000, pFunc);
    }
}

/**
 * initialize hardware
 * @return true if hardware initialized properly
 */
bool    Globals::initPowerMeter()
{
    if (pzemrdy) 
        return true; // nothing to do
    BoardWitty::triggerLight(BoardWitty::LED_r, true);

    //check for poll delay timeout
    if (millis() - lastpoll < POLL_DELAY) {
        return false;
    };

    // Set PowerMeter address
    Globals::SerialPrintln("Init PZEM...");
    // reconnect to the power meter
    if (! setPzemAddr()) {
        return false;
    }

    lastpoll = millis(); //reset poll timer

    yield();
    Globals::SerialPrintln("OK!");
    BoardWitty::triggerLight(BoardWitty::LED_r, false);
    return true;
}

/**
 * set HTTP server event callback
 * @param eventName - event name
 * @param pFunc     - callback function
 * @return true if callback set
 */
bool    Globals::setHttpServerCallBack(const String &eventName, TWebServerCallBack pFunc)
{
    if(m_pServer!=NULL && pFunc!=NULL)
    {
        std::function<void(void)> f_callback = pFunc;
        m_pServer->on(eventName.c_str(), f_callback);
        return true;
    }
    return false;
}

/**
 * set HTTP server event callback
 * @param eventName - event name
 * @param pFunc     - callback function
 * @return true if callback set
 */
bool    Globals::setHttpServerCallBack(const String &eventName, HTTPMethod method, TWebServerCallBack pFunc)
{
    if(m_pServer!=NULL && pFunc!=NULL)
    {
        std::function<void(void)> f_callback = pFunc;
        m_pServer->on(eventName.c_str(), method, f_callback);
        return true;
    }
    return false;
}

/**
 * dumb function for timer
 */
void espreboot()
{
    Globals::SerialPrintln("Reboot initiated");
    ESP.restart();
}
/**
 * restart timer for reboot
 */
void    Globals::restartPoller()
{
    timer.setTimeout(20000, &espreboot);
}

/**
 * create JSON string for a sample row
 * @param row        - row index
 * @param meter_time - measurement time
 * @param jsn
 */
void   Globals::mkjsondata(size_t row, ESP_TIMESTAMP meter_time, String& jsn)
{
    Globals::makeJsonData(((m_Counter + row) % POLL_SAMPLES), 
                          meter_time - (POLL_SAMPLES - row) * poll_period, 
                          jsn);
}

void    Globals::updateCounter()
{
    m_Counter = (m_Counter+1)%POLL_SAMPLES;
}

/**
 * make a string with last-polled data (cacti poller format)
 * @param txtdata
 * @return 
 */
String& Globals::mktxtdata(String& txtdata)
{
    txtdata = "U:";
    //txtdata += *(pdata.voltage);
    txtdata += m_fMeterings[(int)Meterings_e::emVoltage];
    txtdata += " I:";
    //txtdata += *(pdata.current);
    txtdata += m_fMeterings[(int)Meterings_e::emCurrent];
    txtdata += " P:";
    //txtdata += *(pdata.power);
    txtdata += m_fMeterings[(int)Meterings_e::emPower];
    txtdata += " W:";
    //txtdata += *(pdata.energy);
    txtdata += m_fMeterings[(int)Meterings_e::emEnergy];
    return txtdata;
}

/**
 * makes a pseudo-json from samples array (1 row)
 * @param result
 * @param tstamp
 * @param jsn
 * @param W - include energy counter in json (deprecated)
 * @return 
 */
String& Globals::makeJsonData(size_t row, ESP_TIMESTAMP_C tstamp, String& jsn, const bool W)
{
    jsn = "{\"t\":";
    jsn += tstamp;
    jsn += "000,\"U\":";
    jsn += m_pSamples[row][0]; // Voltage (trailing 000 required for js timestamp in ms)
    jsn += ",\"I\":";
    jsn += m_pSamples[row][1]; // Current
    jsn += ",\"P\":";
    jsn += m_pSamples[row][2]; // Power
    if (W) {
        jsn += ",\"W\":";
        jsn += m_pSamples[row][3];
    } // Energy
    jsn += ",\"pF\":";
    jsn += Globals::pfcalc(m_pSamples[row]); // Add PowerFactor calculation
    jsn += "},\n";
    return jsn;
}

/**
 * calculate PowerFactor from UIP values
 * @param row - row to compute
 * @return power factor
 */
float Globals::pfcalc(const float* pfRow)
{
    if(pfRow==NULL)
        return Globals::cfZero;
    // result[] must be an array of UIP
    // PF = P / UI
    if ( fabs(pfRow[0])<=Globals::cfPrec || 
         fabs(pfRow[1])<=Globals::cfPrec ) 
        return Globals::cfZero;
    return ( pfRow[2] / pfRow[0] / pfRow[1]);
}

// This trick makes all embedded serial stuff go away magically
#if !DEBUG
#define HW_SERIAL_FUCK    0
#else
#include "NullSerial.h"         // http://forum.arduino.cc/index.php?topic=36940.0
static NullSerial   Serial;
#endif  // !DEBUG

/**
 * print debug message to the serial port 
 * @param szToPrint - what to print
 */
void    Globals::SerialPrint(const char* szToPrint)
{
#if !DEBUG
#ifndef NO_GLOBAL_SERIAL
    Serial.print(szToPrint);
#endif  // NO_GLOBAL_SERIAL
#endif
}

/**
 * print debug message with line end to the serial port 
 * @param szToPrint - what to print
 */
void    Globals::SerialPrintln(const char* szToPrint)
{
#if !DEBUG
#ifndef NO_GLOBAL_SERIAL
    Serial.println(szToPrint);
#endif  // NO_GLOBAL_SERIAL
#endif
}

void    Globals::SerialBegin(unsigned long baud)
{
#if !DEBUG
#ifndef NO_GLOBAL_SERIAL
    Serial.begin(baud);
#endif  // NO_GLOBAL_SERIAL
#endif
}
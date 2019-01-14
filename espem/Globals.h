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


#ifndef GLOBALS_H
#define GLOBALS_H

#ifndef POLL_DELAY
#define POLL_DELAY  2000    // meter polling interval delay ms
#endif
#ifndef METERING_SIZE
#define METERING_SIZE 4
#endif
#ifndef ESP_TIMESTAMP
#define ESP_TIMESTAMP unsigned long
#endif
#ifndef ESP_TIMESTAMP_C
#define ESP_TIMESTAMP_C const unsigned long
#endif
// web server call back
typedef std::function<void(void)> TWebServerCallBack;
// to refer power meterings
enum class Meterings_e:int
{
    emVoltage = 0,
    emCurrent,
    emPower,
    emEnergy,
    emCount
};

class Globals {
public:
    // public methods
    Globals();
    Globals(const Globals& orig);
    virtual ~Globals();
    PZEM004T* pzem() {return m_pzem;};
    IPAddress&  ip()    {return *m_pIp;};
    ESP8266WebServer&   server()    {return *m_pServer;};
    float** samples()   {return m_pSamples;};
    float   getSample(size_t x, size_t y);
    void   setSample(size_t x, size_t y, float val);
    // public variables
    SimpleTimer timer;      // timer object
    bool    poll;           // Polling enabled
    uint8_t poll_period;    // Polling period
    bool    PF_fix;         // Power Factor correction
    unsigned long   lastpoll;// meter last poll time
    bool pzemrdy;           // meter available
    int poller_id;          // scheduler ID for poller task
    static const float cfZero;
    static const float cfPrec;
    
    bool setPzemAddr();
    bool copySamples(const float* pSamples, size_t row, size_t count);
    void    disablePoller();
    void    enablePoller(timer_callback pFunc);
    void    restartPoller();
    bool    initPowerMeter();
    bool    setHttpServerCallBack(const String& eventName, TWebServerCallBack pFunc);
    bool    setHttpServerCallBack(const String &eventName, HTTPMethod method, TWebServerCallBack pFunc);
    unsigned long getMeterTime()    {return now() - (millis() - lastpoll) / 1000;};
    void    mkjsondata(size_t row, ESP_TIMESTAMP meter_time, String& jsn);
    void    updateCounter();
    inline  uint16_t getCounter() {return m_Counter;};
    String& mktxtdata(String& txtdata);
    String& makeJsonData(size_t          row, 
                         ESP_TIMESTAMP_C tstamp, 
                         String&         jsn, 
                         const bool      W=true);
    static  float pfcalc(const float* pfRow);
    static  void    SerialPrint(const char* szToPrint);
    static  void    SerialPrintln(const char* szToPrint);
    static  void    SerialBegin(unsigned long baud);

#if USE_HWSERIAL
    HardwareSerial* m_pHWSer;   // hardware serial port
#endif

private:
    PZEM004T* m_pzem;
    IPAddress*  m_pIp;  // IP address

    ESP8266WebServer*   m_pServer;  // WEB server

    // WA to make other functions happy
    float    **m_pSamples;   // Storage array for poll samples (UIP - only)
    size_t   m_nRows;  //  first dimension size;
    size_t   m_nCols;  //  second dimension size;
    uint16_t m_Counter;
    float    m_fMeterings[METERING_SIZE];
};

#endif /* GLOBALS_H */


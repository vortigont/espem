/*  ESPEM - ESP Energy monitor
 *  A code for ESP8266/ESP32 based boards to interface with PeaceFair PZEM PowerMeters
 *  It can poll/collect PowerMeter data and provide it for futher processing in text/json format
 *
 *  (c) Emil Muratov 2018-2021  https://github.com/vortigont/espem
 *
 */

#include <vector>
#include <memory>
#include "main.h"
// Libs
#ifdef USE_PZEMv3
 #include <PZEM004Tv30.h>
#else
 #include <PZEM004T.h>
#endif
// Tasker object from EmbUI
#include "ts.h"

// Defaults
#ifndef ESPEM_POLLRATE
    #define ESPEM_POLLRATE 10       // sec
#endif
#ifndef ESPEM_MAXPOLLRATE
    #define ESPEM_MAXPOLLRATE 1000    // ms
#endif
#ifndef ESPEM_IPADDR
    #define ESPEM_IPADDR {192,168,1,1}
#endif

#ifndef ESPEM_MEMPOOL
    #define ESPEM_MEMPOOL 0                 // KiB (Do not collect metrics by default)
#endif

#define ESPEM_MEMRESERVE    (4*1024U)      // Bytes


// Metrics collector state
enum class mcstate_t{MC_DISABLE=0, MC_RUN, MC_PAUSE};

// TaskScheduler - Let the runner object be a global, single instance shared between object files.
extern Scheduler ts;

struct ESPEM_CFG {

    uint16_t pollrate;           // meter polling interval, s
    uint16_t mempool;            // desired RAM pool to keep metrics, KiB
    IPAddress ip;

    /**
     * Power Factor correction
     * An older PZEM applies some sort of averaging to current and power
     * values that might not always correlate to each other properly resulting in PF being > 1
     * this enables 'current' value correction in favor of 'power' value
     */
    bool PFfix;                        // Power Factor correction

    /**
     * Meter polling active/disabled
     */
    bool poll;

    /**
     * metrics collector state
     */
    mcstate_t collector;

    // struct constructor with default values
    ESPEM_CFG (
        uint16_t _pp = ESPEM_POLLRATE,
        uint16_t _mempool = 0,
        bool _pfix = true,
        bool _poll = true,
        mcstate_t _col = mcstate_t::MC_DISABLE,
        IPAddress _ip = ESPEM_IPADDR
    ) : pollrate(_pp),
        mempool(_mempool),
        ip(_ip),
        PFfix(_pfix),
        poll(_poll),
        collector(_col) {}
};  // end of ESPEM_CFG structure

struct pmeterData {
  union {
    struct {
        float voltage, current, power, energy;
    };
    float meterings[4] = {0.0f};
  };

// calculate PowerFactor from UIP values
float pf() const {
    // result[] must be an array of UIP; PF = P / UI
    if (voltage == 0 || current == 0) return 0.0;
    return( power / voltage / current);
}
};

#ifdef USE_PZEMv3
class PZEM: public PZEM004Tv30 {
    using PZEM004Tv30::PZEM004Tv30;
};
//declare pointer-to-member function for PZEM class
typedef float( PZEM::*PZPTMF)();
#else
class PZEM: public PZEM004T {
    using PZEM004T::PZEM004T;
};
//declare pointer-to-member function for PZEM class
typedef float( PZEM::*PZPTMF) (const IPAddress& );
#endif



/**
 * @brief - PowerMeter instance class
 * Helds an object of one PZEM instance along with it's meterings
 */
class PMETER {

public:
    /**
     * Class constructor
     * uses predefined values of a ESPEM_CFG
     */
    PMETER(IPAddress _ip) : ip(_ip){}

    ~PMETER(){
        #ifdef ESPEM_USE_HWSERIAL
            delete hwser;
        #endif
    }


    bool pffix = true;

    void begin();

    unsigned long getLastPollTime(){ return lastpoll; }

    const pmeterData &getData() const { return pdata; }

    bool poll() { return pollMeter(pzem, pdata, pffix); }

private:

#ifdef ESPEM_USE_HWSERIAL
    HardwareSerial *hwser;
#endif

    // An instance of PZEM lib object
    std::unique_ptr<PZEM> pzem = nullptr;

    IPAddress ip = ESPEM_IPADDR;

    // PZEM meter status - connected and available
    bool pzemrdy = false;

    // meter last poll time (system millis())
    unsigned long lastpoll = 0;

    //pointer array to lib funtions
    PZPTMF pzdatafunc[4] = {&PZEM::voltage, &PZEM::current, &PZEM::power, &PZEM::energy};

    //a struct for meter data
    pmeterData pdata;


// Private Methods
    bool pzeminit();

    bool pollMeter(std::unique_ptr<PZEM> &meter, pmeterData &result, bool fixpf);


};


/**
 * @brief - Power Metrics collector
 * 
 */
class PMETRICS {


public:
    PMETRICS(std::shared_ptr<PMETER> _mtr, uint16_t _psize, uint16_t _rate = ESPEM_POLLRATE, bool _poll = false, bool _collect = false) : meter(_mtr), poolsize(_psize) {

        tPoller.set(_rate * TASK_SECOND, TASK_FOREVER, std::bind(&PMETRICS::datapoller, this));
        ts.addTask(tPoller);

        // enable polling scheduler
        if (_poll)
            tPoller.enableDelayed();

        if (_collect)
            poolAlloc(poolsize);

    };

    ~PMETRICS(){
        ts.deleteTask(tPoller);
        delete samples;
    };

    bool begin();

    inline bool polling(){return tPoller.isEnabled();};

    /**
     * @brief - Control meter polling
     * @param state - enable/disable
     */
    bool polling(bool state){
        state ? tPoller.enableDelayed() : tPoller.disable();
        return polling();
    };

    /**
     * @brief - set polling period
     * @param rate - poll rate interval in seconds
     */
    inline size_t PollInterval(){return (tPoller.getInterval() / 1000);};
    size_t PollInterval(uint16_t rate);


    inline mcstate_t collector(){return mcstate;};

    /**
     * @brief - change collector state to DISABLE/RUN/PAUSE
     * @param newstate - new state
     */
    mcstate_t collector(const mcstate_t newstate);

    /**
     * @brief - resize memory pool for metrics storage
     * @param size - desired new size
     * if new size is less than old - mem pool is shrinked, releasing memory (index pointer might change)
     * if the new size is more than old one - full reallocation is done, all current sampling data is lost!
     */
    size_t poolResize(size_t size);

    uint16_t getMetricsIdx(){return poolidx;}

    const std::vector<pmeterData> *getData() const { return samples; }

    void setPollCallback(callback_function_t callback){
        callback ? _cb_poll = std::move(callback) : _cb_poll = nullptr;
    };

private:
    // meter object
    std::shared_ptr<PMETER> meter;
    std::vector<pmeterData> *samples = nullptr;
    // callback pointers
    callback_function_t _cb_poll = nullptr;

    mcstate_t mcstate = mcstate_t::MC_DISABLE;              // Metrics collector state
    uint16_t poolidx;               // index pointer to next element in a pool
    size_t poolsize;                // desired mem pool size

    Task tPoller;

    // poll meter for data and store sample in RAM ring buffer (if required)
    void datapoller();

    size_t poolAlloc(size_t size);

};

class ESPEM {

public:
    /**
     * Class constructor
     * uses predefined values of a ESPEM_CFG
     */
    ESPEM(){}

    /**
     * Class constructor
     * initialized with customized ESPEM_CFG
     */
    ESPEM(const ESPEM_CFG& _cfg) : ecfg(_cfg){}


    ~ESPEM(){}

    bool begin();

    /** @brief onNetIfUp - коллбек для внешнего события "сеть доступна"
     *
     */
    void onNetIfUp();

    /** @brief onNetIfDown - коллбек для внешнего события "сеть НЕ доступна"
     *
     */
    void onNetIfDown();

    static float pfcalc(const float result[]);

    /**
     * @brief - HTTP request callback with latest polled data (as json)
     * 
     */
    void wdatareply(AsyncWebServerRequest *request);

    void wpmdata(AsyncWebServerRequest *request);

    void wsamples(AsyncWebServerRequest *request);

    /**
     * @brief - set meter poll rate in seconds
     * @param seconds - poll interval rate
     */
    size_t PollRate(uint16_t seconds){
        if (metrics){
          return metrics->PollInterval(seconds);
        } else { return 0;};
    };
    size_t PollRate(){
        if (metrics){
          return metrics->PollInterval();
        } else { return 0;};
    };

    /**
     * @brief - Control meter polling
     * @param active - enable/disable
     * @return - current state
     */
    bool meterPolling(bool active){
        if (metrics){
          return metrics->polling(active);
        } else { return false;};
    };
    bool meterPolling(){
        if (metrics){
          return metrics->polling();
        } else { return false;};
    };

    bool pffix(){
        if (meter){
            return meter->pffix;
        } else
            return false;
    }
    bool pffix(const bool fix){
        if (meter)
            meter->pffix=fix;
        return pffix();
    }

    /**
     * @brief - get metrics storage capacity if any
     * 
     */
    size_t getMetricsCap(){
        if (metrics && metrics->getData())
            return metrics->getData()->capacity();
        return 0;
    }

    size_t poolResize(size_t size){
        return (metrics ? metrics->poolResize(size) : 0);
    };

    mcstate_t  collector(){
        return (metrics ? metrics->collector() : mcstate_t::MC_DISABLE);
    }
    mcstate_t collector(mcstate_t state){
//        if (state > (uint8_t)mcstate_t::MC_PAUSE)
//            state = (uint8_t)mcstate_t::MC_RUN;
        return (metrics ? metrics->collector((mcstate_t)state) : mcstate_t::MC_DISABLE);
    }


    const pmeterData meterData() {
        pmeterData _d;
        if (meter){
            _d = meter->getData();
        }        
        return _d;  // return some dummy struct :)
    }


protected:

private:

    ESPEM_CFG ecfg;

    std::shared_ptr<PMETER> meter = nullptr;        // PZEM object
    std::unique_ptr<PMETRICS> metrics = nullptr;    // Metrics poller/collector

    String& mktxtdata ( String& txtdata);

    // make json string out of array provided
    // bool W - include energy counter in json
    // todo: provide vector with flags for each field
    String& mkjsondata( const float result[], const unsigned long tstamp, String& jsn, const bool W );

    // publish meter data via WebSocket
    void wspublish();

};


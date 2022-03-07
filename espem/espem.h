/*  ESPEM - ESP Energy monitor
 *  A code for ESP8266/ESP32 based boards to interface with PeaceFair PZEM PowerMeters
 *  It can poll/collect PowerMeter data and provide it for futher processing in text/json format
 *
 *  (c) Emil Muratov 2018-2021  https://github.com/vortigont/espem
 *
 */

#pragma once
#include "main.h"
#include "pzem_edl.hpp"
#include "timeseries.hpp"

// Tasker object from EmbUI
#include "ts.h"

// Defaults
#ifndef DEFAULT_WS_UPD_RATE
    #define DEFAULT_WS_UPD_RATE     2     // ws clients update rate, sec
#endif

#ifndef ESPEM_MEMPOOL
    #define ESPEM_MEMPOOL 300             // samples to store in ringbuff by default (5 min)
#endif

#define PZEM_ID     1
#define PORT_1_ID   1

// Metrics collector state
enum class mcstate_t{MC_DISABLE=0, MC_RUN, MC_PAUSE};

// TaskScheduler - Let the runner object be a global, single instance shared between object files.
extern Scheduler ts;

class ESPEM {

public:

    //std::unique_ptr<PZPool> pzpool;        // PZEM object
    PZ004 *pz = nullptr;

    /**
     * Class constructor
     * uses predefined values of a ESPEM_CFG
     */
    ESPEM(){}

    /**
     * Class constructor
     * initialized with customized ESPEM_CFG
     */
    //ESPEM(const ESPEM_CFG& _cfg) : ecfg(_cfg){}

    ~ESPEM(){
        ts.deleteTask(t_uiupdater);
        delete pz;
        pz = nullptr;
        delete qport;
        qport = nullptr;
    }



    bool begin(const uart_port_t p, int rx=UART_PIN_NO_CHANGE, int tx=UART_PIN_NO_CHANGE);

    /** @brief onNetIfUp - коллбек для внешнего события "сеть доступна"
     *
     */
    void onNetIfUp();

    /** @brief onNetIfDown - коллбек для внешнего события "сеть НЕ доступна"
     *
     */
    void onNetIfDown();


    /**
     * @brief - HTTP request callback with latest polled data (as json)
     * 
     */
    void wdatareply(AsyncWebServerRequest *request);

    void wpmdata(AsyncWebServerRequest *request);

    void wsamples(AsyncWebServerRequest *request);

    /**
     * @brief - set webUI refresh rate in seconds
     * @param seconds - webUI interval
     */
    uint8_t set_uirate(uint8_t seconds);

    uint8_t get_uirate();


    /**
     * @brief - Control meter polling
     * @param active - enable/disable
     * @return - current state
     */
    bool meterPolling(bool active){ return pz->autopoll(active); };
    bool meterPolling() const { return pz->autopoll(); };

    /**
     * @brief - get metrics storage capacity, if any
     * 
     */
    int getMetricsCap() const { return tsc.getTScap(); }

    int getMetricsSize() const { return tsc.getTSsize(); }


    /**
     * @brief set TimeSeries Container params
     * 
     * @param size - number of elements to store
     * @param interval - series interval in seconds
     * @return true - on success
     * @return false - on error
     */
    bool tsSet(size_t size = ESPEM_MEMPOOL, uint32_t interval = 1);

    mcstate_t set_collector_state(mcstate_t state);
    mcstate_t get_collector_state() const { return ts_state; };

    /**
     * @brief Set the Energy offset value
     * tis will offset energy value replies from PZEM
     * i.e. to match some other counter, etc...
     * 
     * @param offset 
     */
    inline void setEnergyOffset(float offset){ nrg_offset = offset; };

    /**
     * @brief Get the Energy offset value
     * 
     * @return float 
     */
    inline float getEnergyOffset(){return nrg_offset;};

private:

    UartQ *qport = nullptr;
    TSContainer<pz004::metrics> tsc;
    uint8_t ts_id;
    mcstate_t ts_state = mcstate_t::MC_DISABLE;

    float nrg_offset{0.0};

    String& mktxtdata ( String& txtdata);

    /**
     * @brief publish updates to websocket clients
     * 
     */
    void wspublish();

    // Tasks
    Task t_uiupdater;

    // make json string out of array provided
    // bool W - include energy counter in json
    // todo: provide vector with flags for each field
    //String& mkjsondata( const float result[], const unsigned long tstamp, String& jsn, const bool W );

};


/**
 * @brief callback method to print debug data on PZEM RX
 * 
 * @param id 
 * @param m 
 */
void msgdebug(uint8_t id, const RX_msg* m);
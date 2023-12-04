/*  ESPEM - ESP Energy monitor
 *  A code for ESP32 based boards to interface with PeaceFair PZEM PowerMeters
 *  It can poll/collect PowerMeter data and provide it for futher processing in text/json format
 *
 *  (c) Emil Muratov 2018-2022  https://github.com/vortigont/espem
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

#define PZEM_ID     1
#define PORT_1_ID   1

#define TS_T1_CNT       900              // default Tier 1 TimeSeries count
#define TS_T1_INTERVAL  1                // default Tier 1 TimeSeries interval (1 sec)
#define TS_T2_CNT       1000             // default Tier 2 TimeSeries count
#define TS_T2_INTERVAL  15               // default Tier 2 TimeSeries interval (15 sec)
#define TS_T3_CNT       1000             // default Tier 3 TimeSeries count
#define TS_T3_INTERVAL  300              // default Tier 3 TimeSeries interval (5 min)


// Metrics collector state
enum class mcstate_t{MC_DISABLE=0, MC_RUN, MC_PAUSE};

// TaskScheduler - Let the runner object be a global, single instance shared between object files.
extern Scheduler ts;

class DataStorage {
    // TimeSeries COntainer
    TSContainer<pz004::metrics> tsc;
    std::vector<uint8_t> tsids;

    // energy offset
    int32_t nrg_offset{0};


public:

    /**
     * @brief setup TimeSeries Container based on saved params in EmbUI config
     * 
     */
    void reset();

    /**
     * @brief Set the Energy offset value
     * tis will offset energy value replies from PZEM
     * i.e. to match some other counter, etc...
     * 
     * @param offset 
     */
    void setEnergyOffset(int32_t offset){ nrg_offset = offset; }

    /**
     * @brief Get the Energy offset value
     * 
     * @return float 
     */
    int32_t getEnergyOffset(){ return nrg_offset; }

    const TSContainer<pz004::metrics>& getTSC(){ return tsc; }

    /**
     * @brief - get metrics storage capacity, if any
     * 
     */
    int getMetricsCap() const { return tsc.getTScap(); }

    int getMetricsSize() const { return tsc.getTSsize(); }

    /**
     * @brief push new data to TimeSeries storage
     * 
     * @param m 
     */
    void push(const pz004::metrics *m);

    void wsamples(AsyncWebServerRequest *request);

    // wrappers
    void purge(){ tsc.purge(); }

};

class Espem {

public:

    PZ004 *pz = nullptr;

    // TimeSeries data storage
    DataStorage ds;

    /**
     * Class constructor
     * uses predefined values of a ESPEM_CFG
     */
    Espem(){}

    ~Espem(){
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

    /**
     * @brief - set webUI refresh rate in seconds
     * @param seconds - webUI interval
     */
    uint8_t set_uirate(uint8_t seconds);

    /**
     * @brief Get the ui refresh rate
     * 
     * @return uint8_t 
     */
    uint8_t get_uirate();   // TaskScheduler class does not allow it to declare const'ness


    /**
     * @brief - Control meter polling
     * @param active - enable/disable
     * @return - current state
     */
    bool meterPolling(bool active){ return pz->autopoll(active); };
    bool meterPolling() const { return pz->autopoll(); };

    mcstate_t set_collector_state(mcstate_t state);
    mcstate_t get_collector_state() const { return ts_state; };

private:

    UartQ *qport = nullptr;
    mcstate_t ts_state = mcstate_t::MC_DISABLE;
    // Tasks
    Task t_uiupdater;

    // mqtt feeder id
    int _mqtt_feed_id{0};

    String& mktxtdata ( String& txtdata);

    /**
     * @brief publish updates to websocket clients
     * 
     */
    void wspublish();


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


/*  ESPEM - ESP Energy monitor
 *  A code for ESP8266/ESP32 based boards to interface with PeaceFair PZEM PowerMeters
 *  It can poll/collect PowerMeter data and provide it for futher processing in text/json format
 *
 *  (c) Emil Muratov 2018-2021  https://github.com/vortigont/espem
 *
 */

#include "espem.h"
#include "EmbUI.h"      // EmbUI framework

#ifdef ESP32
 #define MAX_FREE_MEM_BLK ESP.getMaxAllocHeap()
#else
 #define MAX_FREE_MEM_BLK ESP.getMaxFreeBlockSize()
#endif

// sprintf template for json sampling data
#define JSON_SMPL_LEN 80    // {"t":1615496537000,"U":229.50,"I":1.47,"P":1216,"W":5811338,"pF":0.64},
static const char PGsmpljsontpl[] PROGMEM = "{\"t\":%lu000,\"U\":%.1f,\"I\":%.2f,\"P\":%.0f,\"W\":%.0f,\"pF\":%.2f},";
static const char PGdatajsontpl[] PROGMEM = "{\"age\":%lu,\"U\":%.1f,\"I\":%.2f,\"P\":%.0f,\"W\":%.0f,\"pF\":%.2f}";

// HTTP responce messages
static const char PROGMEM PGsmpld[] = "Metrics collector disabled";
static const char PROGMEM PGdre[] = "Data read error";
static const char PROGMEM PGacao[] = "Access-Control-Allow-Origin";


bool ESPEM::begin(){

  meter = std::make_shared<PMETER>(ecfg.ip);     // Connect to PZEM via HW_serial

  meter->pffix = ecfg.PFfix;

  meter->begin();

//  if (ecfg.mempool && ecfg.collector){
    metrics = std::unique_ptr<PMETRICS>(new PMETRICS(meter, ecfg.mempool, ecfg.pollrate, ecfg.poll, (bool)ecfg.collector));
//  }
  metrics->setPollCallback(std::bind(&ESPEM::wspublish, this));

  embui.server.on(PSTR("/getdata"), HTTP_GET, [this](AsyncWebServerRequest *request){
    wdatareply(request);
  });

  // compat method for v 1.x cacti scripts
  embui.server.on(PSTR("/getpmdata"), HTTP_GET, [this](AsyncWebServerRequest *request){
    wpmdata(request);
  });

	// generate json with sampled meter data
  embui.server.on(PSTR("/samples"), HTTP_GET, std::bind(&ESPEM::wsamples, this, std::placeholders::_1));
  embui.server.on(PSTR("/samples.json"), HTTP_GET, std::bind(&ESPEM::wsamples, this, std::placeholders::_1));

/*
  embui.server.on(PSTR("/getdata"), HTTP_GET, [this](AsyncWebServerRequest *request){
    //AsyncWebServerResponse *response = request->beginResponse(307, FPSTR(PGmimetxt), "Hello World!");
    AsyncWebServerResponse *response = request->beginResponse(307);
    response->addHeader(F("Retry-After"),F("5"));
    response->addHeader(F("Location"),F("/getpmdata"));
    request->send(response);

    //request->send(200, FPSTR(PGmimetxt), mktxtdata(data) );
  });
*/

  return true;
}


// make a string with last-polled data (cacti poller format)
// this is the 'compat' version for an old pzem w/o pf/HZ values
String& ESPEM::mktxtdata ( String& txtdata) {

    //pmeterData pdata = meter->getData();
    txtdata = "U:";
    txtdata += meter->getData().voltage;
    txtdata += " I:";
    txtdata += meter->getData().current;
    txtdata += " P:";
    txtdata += meter->getData().power;
    txtdata += " W:";
    txtdata += meter->getData().energy;
//    txtdata += " pf:";
//    txtdata += pfcalc(meter->getData().meterings);
    return txtdata;
}

// calculate PowerFactor from UIP values
float ESPEM::pfcalc(const float result[]) {
  // result[] must be an array of UIP
  // PF = P / UI
  if (result[0] == 0 || result[1] == 0) return 0.0;
  return( result[2] / result[0] / result[1]);
}

// pmdata web-page
// Returns an http-response with Powermeter data from pdata struct
// compat method for v 1.x cacti scripts
void ESPEM::wpmdata(AsyncWebServerRequest *request) {
  if ( !meter ) {
    request->send(503, FPSTR(PGmimetxt), FPSTR(PGdre) );
    return;
  }

  String data;
  request->send(200, FPSTR(PGmimetxt), mktxtdata(data) );
}


void ESPEM::wdatareply(AsyncWebServerRequest *request){
  if ( !meter ) {
    request->send_P(503, PGmimetxt, PGdre );
    return;
  }

  char buffer[JSON_SMPL_LEN];
  sprintf_P(buffer, PGdatajsontpl,
            millis() - meter->getLastPollTime(),
            meter->getData().voltage,
            meter->getData().current,
            meter->getData().power,
            meter->getData().energy,
            meter->getData().pf()
  );
  request->send(200, FPSTR(PGmimejson), buffer );
}


// return json-formatted response for in-RAM sampled data
void ESPEM::wsamples(AsyncWebServerRequest *request) {

  // check if there is any sampled data
  if ( !getMetricsCap() ) {
    request->send_P(503, PGmimetxt, PGsmpld);
    return;
  }

  // json response maybe pretty large and needs too much of a precious ram to store it in a temp 'string'
  // So I'm going to generate it on-the-fly and stream to client with chunks

  size_t sampleIndex = 0;   // index pointer to keep track of chuncked data in replies
  size_t  cnt = 0;    // cnt - return last 'cnt' samples, 0 - all samples

  if (request->hasParam(F("scntr"))){
    AsyncWebParameter* p = request->getParam(F("scntr"));
    if (!p->value().isEmpty())
      cnt = p->value().toInt();
  }

  AsyncWebServerResponse* response = request->beginChunkedResponse(FPSTR(PGmimejson),
                                  [this, sampleIndex, cnt](uint8_t* buffer, size_t buffsize, size_t index) mutable -> size_t {

      // If provided bufer is not large enough to fit 1 sample chunk, than I'm just sending
      // an empty space char (allowed json symbol) and wait for the next buffer
      if (buffsize < JSON_SMPL_LEN){
        buffer[0] = 0x20; // ASCII 'Space'
        return 1;
      }

      size_t len = 0;

      const std::vector<pmeterData> *samples = metrics->getData();

      // set number of samples to send in responce
      if (!cnt || cnt > samples->capacity())
        cnt = samples->capacity();

      if (!sampleIndex){
        buffer[0] = 0x5b;   // Open json with ASCII '['
        ++len;
        sampleIndex = samples->capacity() - cnt;  // shift index to the last n'th sample as requested
      }

      time_t meter_time = embui.timeProcessor.getUnixTime() - (millis() - meter->getLastPollTime())/1000;  // find out timestamp for last sample
      // todo: need to handle variable interval between samples

      // prepare a chunk of sampled data wrapped in json
      while (len < (buffsize - JSON_SMPL_LEN) && sampleIndex != samples->capacity()){
        size_t idx = (metrics->getMetricsIdx() + sampleIndex) % samples->capacity();
        len += sprintf_P((char *)buffer + len, PGsmpljsontpl,
                    meter_time - ecfg.pollrate * (samples->capacity() - sampleIndex),
                    samples->at(idx).voltage,
                    samples->at(idx).current,
                    samples->at(idx).power,
                    samples->at(idx).energy,
                    pfcalc(samples->at(idx).meterings)
                    );

        ++sampleIndex;

        if (sampleIndex == samples->capacity()){
          buffer[len-1] = 0x5d;   // ASCII ']' implaced over last comma
        }
      }

      //LOG(printf, "JSON: Sending %d items, buffer %d/%d, idx: %d\n", cnt, len, buffsize, sampleIndex);
      return len;
  });

  response->addHeader(FPSTR(PGacao),"*");   // CORS header
  request->send(response);
}

// publish meter data via WebSocket
void ESPEM::wspublish(){
  if (!embui.ws.count())  // if there are no clients connected
      return;

  Interface *interf = new Interface(&embui, &embui.ws, 512);

  interf->json_frame_custom(F("rawdata"));
  interf->value(F("U"), meter->getData().voltage);
  interf->value(F("I"), meter->getData().current);
  interf->value(F("P"), meter->getData().power);
#ifdef USE_PZEMv3
  interf->value(F("W"), meter->getData().energy);
#else
  interf->value(F("W"), meter->getData().energy/1000.0);
#endif
  interf->value(F("Pf"), meter->getData().pf());
  interf->json_frame_flush();
  delete interf;
}



/**********************   PMETRICS Methods **************/

/**
 * @brief - allocates memory pool for metrics data
 * @param size - pool size in KiB
 */
size_t PMETRICS::poolAlloc(size_t size){
  if (!size)
    return 0;

  LOG(printf_P, PSTR("PMETRICS: FreeHeap: %d, MaxFreeBlockSize: %d\n"), ESP.getFreeHeap(), MAX_FREE_MEM_BLK);

  delete samples; // make sure it is free
  if (size > MAX_FREE_MEM_BLK/1024 - ESPEM_MEMRESERVE)
      size = MAX_FREE_MEM_BLK/1024 - ESPEM_MEMRESERVE;

  samples = new std::vector<pmeterData>(size*1024 / sizeof(pmeterData));

  if (samples){
    LOG(printf_P, PSTR("Memory pool allocated for %d samples\n"), samples->capacity());
    mcstate = mcstate_t::MC_RUN;
    poolidx = 0;
    return samples->capacity();
  } else {
    LOG(println, F("Memory pool allocation failed\n"));
    mcstate = mcstate_t::MC_DISABLE;
  }
  return 0;
}


/**
 * @brief - change collector state to DISABLE/RUN/PAUSE
 * @param newstate - new state
 */
mcstate_t PMETRICS::collector(mcstate_t newstate){
  switch(newstate){
    case mcstate_t::MC_DISABLE : {
      delete samples;
      samples = nullptr;
      mcstate = mcstate_t::MC_DISABLE;
      break;
    }
    case mcstate_t::MC_PAUSE :
      mcstate = newstate;
      break;
    case mcstate_t::MC_RUN :
    default: {
      if (mcstate_t::MC_DISABLE == mcstate){
        poolAlloc(poolsize);
      } else {
        mcstate = mcstate_t::MC_RUN;
      }
      break;
    }
  }
  LOG(printf_P, PSTR("Collector state: %d\n"), (int)mcstate);
  return mcstate;
}


/**
 * @brief - resize memory pool for metrics storage
 * @param size - desired new size
 * if new size is less than old - mem pool is shrinked, releasing memory (index pointer might change)
 * if the new size is more than old one - full reallocation is done, all current sampling data is lost!
 */
size_t PMETRICS::poolResize(size_t size){

  LOG(printf_P, PSTR("Requested metrics pool change to %d KiB\n"), size);
  poolsize = size;

  if (!size){
    collector(mcstate_t::MC_DISABLE);
    return 0;
  }

  if ( samples && (size*1024 < samples->capacity()*sizeof(pmeterData)) ){
    LOG(printf_P, PSTR("Resizing metrics pool to fit %d samples\n"), size*1024 / sizeof(pmeterData));
    samples->resize(size*1024 / sizeof(pmeterData));
    samples->shrink_to_fit();
    if (poolidx >= samples->capacity())
      poolidx = 0;
    LOG(printf_P, PSTR("New pool size: %d samples\n"), samples->capacity());
    return samples->capacity();
  } else {
    return poolAlloc(size);
  }
}


// poll meter for data and store sample in RAM ring buffer (if required)
void PMETRICS::datapoller(){

  if (!meter->poll())
    return;
  LOG(printf_P, PSTR("Meter poll: U:%.1f I:%.2f P:%.0f W:%.0f pf:%.2f\n"), meter->getData().voltage, meter->getData().current, meter->getData().power, meter->getData().energy, meter->getData().pf());

  if ( mcstate == mcstate_t::MC_RUN && samples && samples->capacity() ){
      samples->at(poolidx) = meter->getData();

/*
//exception handling disabled, use -fexceptions to enable
      //std::copy(&pdata.meterings[0], &pdata.meterings[3], samples->at(poolidx).meterings);
    try {}
    catch (std::out_of_range o){
      LOG(printf_P, PSTR("Exception: Vector out of range - %s"), o.what());
    }
*/
    LOG(printf_P, PSTR("SMPL: %d out of %d\n"), poolidx+1, samples->capacity());
    if ( ++poolidx == samples->capacity() )
      poolidx=0;
  };

  if (_cb_poll)
    _cb_poll();
}

/**
 * @brief - set meter poll rate in seconds
 * @param seconds - poll interval rate
 */
size_t PMETRICS::PollInterval(uint16_t rate){
  tPoller.setInterval(rate * TASK_SECOND);
  return tPoller.getInterval() / 1000;
}

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
#define JSON_SMPL_LEN 85    // {"t":1615496537000,"U":229.50,"I":1.47,"P":1216,"W":5811338,"hz":50.0,"pF":0.64},
static const char PGsmpljsontpl[] PROGMEM = "{\"t\":%u000,\"U\":%.2f,\"I\":%.2f,\"P\":%.0f,\"W\":%.0f,\"hz\":%.1f,\"pF\":%.2f},";
static const char PGdatajsontpl[] PROGMEM = "{\"age\":%llu,\"U\":%.1f,\"I\":%.2f,\"P\":%.0f,\"W\":%.0f,\"hz\":%.1f,\"pF\":%.2f}";

// HTTP responce messages
static const char PROGMEM PGsmpld[] = "Metrics collector disabled";
static const char PROGMEM PGdre[] = "Data read error";
static const char PROGMEM PGacao[] = "Access-Control-Allow-Origin";

using namespace pzmbus;     // use general pzem abstractions


bool ESPEM::begin(const uart_port_t p, int rx, int tx){
  LOG(printf, "espem.begin: port: %d, rx_pin: %d, tx_pin:%d\n", p, rx, tx);

  // let's make our begin idempotent )
  if (qport){
    if (pz)
      pz->detachMsgQ();

    delete qport;
    qport = nullptr;
  }

  qport = new UartQ(p, rx, tx);
  if (!qport) return false; // failed to create qport

  if (pz){                  // obj already exist
    pz->attachMsgQ(qport);
    qport->startQueues();
    return true;
  }

  // first run
  pz = new PZ004(PZEM_ID, ADDR_ANY);
  if (!pz) return false;    // failed to create obj

  pz->attachMsgQ(qport);
  qport->startQueues();

  // WebUI updater task
  t_uiupdater.set( DEFAULT_WS_UPD_RATE * TASK_SECOND, TASK_FOREVER, std::bind(&ESPEM::wspublish, this) );
  ts.addTask(t_uiupdater);

  if (pz->autopoll(true)){
    t_uiupdater.restartDelayed();
    LOG(println, "Autopolling enabled");
  } else {
      LOG(println, "Sorry, can't autopoll somehow :(");
  }

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

  return true;
}


// make a string with last-polled data (cacti poller format)
// this is the 'compat' version for an old pzem w/o pf/HZ values
String& ESPEM::mktxtdata ( String& txtdata) {
    if (!pz)
      return txtdata;

    //pmeterData pdata = meter->getData();
    const auto m = pz->getMetricsPZ004();

    txtdata = "U:";
    txtdata += m->voltage/10;
    txtdata += " I:";
    txtdata += m->current/1000;
    txtdata += " P:";
    txtdata += m->asFloat(meter_t::pwr) + nrg_offset;
    txtdata += " W:";
    txtdata += m->asFloat(meter_t::enrg);
//    txtdata += " pf:";
//    txtdata += pfcalc(meter->getData().meterings);
    return txtdata;
}

// compat method for v 1.x cacti scripts
void ESPEM::wpmdata(AsyncWebServerRequest *request) {
  if ( !tsc.getTScnt() ) {
    request->send(503, FPSTR(PGmimetxt), FPSTR(PGdre) );
    return;
  }

  String data;
  request->send(200, FPSTR(PGmimetxt), mktxtdata(data) );
}


void ESPEM::wdatareply(AsyncWebServerRequest *request){
    if (!pz)
      return;

  const auto m = pz->getMetricsPZ004();
  char buffer[JSON_SMPL_LEN];
  sprintf_P(buffer, PGdatajsontpl,
            pz->getState()->dataAge(),
            m->asFloat(meter_t::vol),
            m->asFloat(meter_t::cur),
            m->asFloat(meter_t::pwr),
            m->asFloat(meter_t::enrg) + nrg_offset,
            m->asFloat(meter_t::frq),
            m->asFloat(meter_t::pf)
  );
  request->send(200, FPSTR(PGmimejson), buffer );
}


// return json-formatted response for in-RAM sampled data
void ESPEM::wsamples(AsyncWebServerRequest *request) {

  // check if there is any sampled data
  if ( !tsc.getTScnt() ) {
    request->send_P(503, PGmimejson, "[]");
    return;
  }

  // json response maybe pretty large and needs too much of a precious ram to store it in a temp 'string'
  // So I'm going to generate it on-the-fly and stream to client in chunks

  size_t cnt = 0;           // cnt - return last 'cnt' samples, 0 - all samples

  if (request->hasParam("scntr")){
    AsyncWebParameter* p = request->getParam("scntr");
    if (!p->value().isEmpty())
      cnt = p->value().toInt();
  }


  const auto ts = tsc.getTS(ts_id);
  auto iter = ts->cbegin();   // get const iterator

  // set number of samples to send in responce
  if (cnt > 0 && cnt < ts->getSize())
    iter += ts->getSize() - cnt;                    // offset iterator to the last cnt elements

  LOG(printf, "TimeSeries buffer has %d items, scntr: %d\n", ts->getSize(), cnt);

  AsyncWebServerResponse* response = request->beginChunkedResponse(FPSTR(PGmimejson),
                                  [this, iter, ts](uint8_t* buffer, size_t buffsize, size_t index) mutable -> size_t {

      // If provided bufer is not large enough to fit 1 sample chunk, than I'm just sending
      // an empty white space char (allowed json symbol) and wait for the next buffer
      if (buffsize < JSON_SMPL_LEN){
        buffer[0] = 0x20; // ASCII 'white space'
        return 1;
      }

      size_t len = 0;

      if (!index){
        buffer[0] = 0x5b;   // Open json with ASCII '['
        ++len;
      }

      // prepare a chunk of sampled data wrapped in json
      while (len < (buffsize - JSON_SMPL_LEN) && iter != ts->cend()){

        if (iter.operator->() != nullptr){
          // obtain a copy of a struct (.asFloat() member method crashes for dereferenced obj - TODO: investigate)
          pz004::metrics m = *iter.operator->();

          len += sprintf((char *)buffer + len, PGsmpljsontpl,
                    ts->getTstamp() - (ts->cend() - iter) * ts->getInterval(),  // timestamp
                    m.asFloat(meter_t::vol),
                    m.asFloat(meter_t::cur),
                    m.asFloat(meter_t::pwr),
                    m.asFloat(meter_t::enrg) + nrg_offset,
                    m.asFloat(meter_t::frq),
                    m.asFloat(meter_t::pf)
          );
        } else {
            LOG(println, "SMLP pointer is null");
        }

        if (++iter == ts->cend())
          buffer[len-1] = 0x5d;   // ASCII ']' implaced over last comma
      }

      LOG(printf, "Sending timeseries JSON, buffer %d/%d, items left: %d\n", len, buffsize, ts->cend() - iter);
      return len;
  });

  response->addHeader(FPSTR(PGacao),"*");   // CORS header
  request->send(response);
}

// publish meter data via WebSocket (a periodic Task)
void ESPEM::wspublish(){
  if (!embui.ws.count() || !pz)  // exit, if there are no clients connected
      return;

  Interface *interf = new Interface(&embui, &embui.ws, 512);

  const auto m = pz->getMetricsPZ004();

  interf->json_frame_custom(F("rawdata"));

  interf->value(F("stale"), pz->getState()->dataStale(), false);
  interf->value(F("age"), pz->getState()->dataAge());

  interf->value(F("U"), m->voltage/10);
  interf->value(F("I"), m->asFloat(meter_t::cur));
  interf->value(F("P"), m->asFloat(meter_t::pwr));
  interf->value(F("W"), (m->asFloat(meter_t::enrg) + nrg_offset) / 1000);
  interf->value(F("Pf"), m->asFloat(meter_t::pf));
  interf->json_frame_flush();
  delete interf;
}

uint8_t ESPEM::set_uirate(uint8_t seconds){
  if (seconds){
    t_uiupdater.setInterval(seconds * TASK_SECOND);
    t_uiupdater.restartDelayed();
  } else
    t_uiupdater.disable();

  return seconds;
}

uint8_t ESPEM::get_uirate(){
  if (t_uiupdater.isEnabled())
    return (t_uiupdater.getInterval() / TASK_SECOND);

  return 0;
}

bool ESPEM::tsSet(size_t size, uint32_t interval){
  if (!size || !interval)
    return false;

  tsc.purge();

  ts_id = tsc.addTS(size, TimeProcessor::getInstance().getUnixTime(), interval, "TS_1");
  //LOG.printf("Add TS: %d\n", sec);
  //tsc.addTS(300, esp_timer_get_time() >> 20, 10, "per10sec", 2);
  //tsc.addTS(300, esp_timer_get_time() >> 20, 60, "permin", 2);

  LOG(printf, "SRAM: heap %d, free %d\n", ESP.getHeapSize(), ESP.getFreeHeap());
  LOG(printf, "SPI-RAM: heap %d, free %d\n", ESP.getPsramSize(), ESP.getFreePsram());

  return (bool)tsc.getTScap();
}

mcstate_t ESPEM::set_collector_state(mcstate_t state){
  if (!pz){
    ts_state = mcstate_t::MC_DISABLE;
      return ts_state;
  }

  switch (state) {
    case mcstate_t::MC_RUN : {
      if (ts_state == mcstate_t::MC_RUN) return mcstate_t::MC_RUN;
      if (!getMetricsCap()) tsSet();   // reinitialize TS Container if empty

        // attach collector's callback
        auto ref = &tsc;
        pz->attach_rx_callback([this, ref](uint8_t id, const RX_msg* m){
          // collect time-series data
          if (!pz->getState()->dataStale()){
            auto data = pz->getMetricsPZ004();
            ref->push(*data, TimeProcessor::getInstance().getUnixTime());
          }
          #ifdef ESPEM_DEBUG
            msgdebug(id, m);          // it will print every data packet coming from PZEM
          #endif
        });
        ts_state = mcstate_t::MC_RUN;
      break;
    }
    case mcstate_t::MC_PAUSE : {
      pz->detach_rx_callback();
      if (!getMetricsCap()) tsSet();   // reinitialize TS Container if empty
      ts_state = mcstate_t::MC_PAUSE;
      break;
    }
    default: {
      pz->detach_rx_callback();
      tsc.purge();
      ts_state = mcstate_t::MC_DISABLE;
    }
  }
  return ts_state;
}


void msgdebug(uint8_t id, const RX_msg* m){
    Serial.printf("\nCallback triggered for PZEM ID: %d\n", id);

/*
    It is also possible to work directly on a raw data from PZEM
    let's call for a little help here and use a pretty_printer() function
    that parses and prints RX_msg to the stdout
*/
    pz004::rx_msg_prettyp(m);
}   

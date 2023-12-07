/*  ESPEM - ESP Energy monitor
 *  A code for ESP32 based boards to interface with PeaceFair PZEM PowerMeters
 *  It can poll/collect PowerMeter data and provide it for futher processing in text/json format
 *
 *  (c) Emil Muratov 2018-2022  https://github.com/vortigont/espem
 *
 */

#include "espem.h"
#include "EmbUI.h"      // EmbUI framework

#define MAX_FREE_MEM_BLK ESP.getMaxAllocHeap()
#define PUB_JSSIZE  1024
// sprintf template for json sampling data
#define JSON_SMPL_LEN 85    // {"t":1615496537000,"U":229.50,"I":1.47,"P":1216,"W":5811338,"hz":50.0,"pF":0.64},
static const char PGsmpljsontpl[] PROGMEM = "{\"t\":%u000,\"U\":%.2f,\"I\":%.2f,\"P\":%.0f,\"W\":%.0f,\"hz\":%.1f,\"pF\":%.2f},";
static const char PGdatajsontpl[] PROGMEM = "{\"age\":%llu,\"U\":%.1f,\"I\":%.2f,\"P\":%.0f,\"W\":%.0f,\"hz\":%.1f,\"pF\":%.2f}";

// HTTP responce messages
static const char PGsmpld[] = "Metrics collector disabled";
static const char PGdre[] = "Data read error";
static const char PGacao[] = "Access-Control-Allow-Origin";
static const char* PGmimetxt = "text/plain";
//static const char* PGmimehtml = "text/html; charset=utf-8";

using namespace pzmbus;     // use general pzem abstractions


class FrameSendMQTTRaw : public FrameSendMQTT {
public:
  FrameSendMQTTRaw(EmbUI *emb) : FrameSendMQTT(emb){}
  void send(const JsonVariantConst& data) override {
    if (data[P_pkg] == C_espem){
      _eu->publish(C_mqtt_pzem_jmetrics, data[P_block]);
    }
  };
};

bool Espem::begin(const uart_port_t p, int rx, int tx){
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
#ifdef ESPEM_DUMMY
  pz = new DummyPZ004(PZEM_ID, ADDR_ANY);
#else
  pz = new PZ004(PZEM_ID, ADDR_ANY);
#endif
  if (!pz) return false;    // failed to create obj

  pz->attachMsgQ(qport);
  qport->startQueues();

  // WebUI updater task
  t_uiupdater.set( DEFAULT_WS_UPD_RATE * TASK_SECOND, TASK_FOREVER, std::bind(&Espem::wspublish, this) );
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
  embui.server.on("/samples.json", HTTP_GET, [this](AsyncWebServerRequest *r){ ds.wsamples(r); } );

  // create MQTT rawdata feeder and add into the chain
  _mqtt_feed_id = embui.feeders.add( std::make_unique<FrameSendMQTTRaw>(&embui) );

  return true;
}


// make a string with last-polled data (cacti poller format)
// this is the 'compat' version for an old pzem w/o pf/HZ values
String& Espem::mktxtdata ( String& txtdata) {
    if (!pz)
      return txtdata;

    //pmeterData pdata = meter->getData();
    const auto m = pz->getMetricsPZ004();

    txtdata = "U:";
    txtdata += m->voltage/10;
    txtdata += " I:";
    txtdata += m->current/1000;
    txtdata += " P:";
    txtdata += m->asFloat(meter_t::pwr) + ds.getEnergyOffset();
    txtdata += " W:";
    txtdata += m->asFloat(meter_t::enrg);
//    txtdata += " pf:";
//    txtdata += pfcalc(meter->getData().meterings);
    return txtdata;
}

// compat method for v 1.x cacti scripts
void Espem::wpmdata(AsyncWebServerRequest *request) {
  if ( !ds.getTSsize(1) ) {
    request->send(503, PGmimetxt, PGdre );
    return;
  }

  String data;
  request->send(200, PGmimetxt, mktxtdata(data) );
}


void Espem::wdatareply(AsyncWebServerRequest *request){
    if (!pz)
      return;

  const auto m = pz->getMetricsPZ004();
  char buffer[JSON_SMPL_LEN];
  sprintf_P(buffer, PGdatajsontpl,
            pz->getState()->dataAge(),
            m->asFloat(meter_t::vol),
            m->asFloat(meter_t::cur),
            m->asFloat(meter_t::pwr),
            m->asFloat(meter_t::enrg) + ds.getEnergyOffset(),
            m->asFloat(meter_t::frq),
            m->asFloat(meter_t::pf)
  );
  request->send(200, FPSTR(PGmimejson), buffer );
}


// return json-formatted response for in-RAM sampled data
void DataStorage::wsamples(AsyncWebServerRequest *request) {
  uint8_t id = 1;           // default ts id

  if (request->hasParam("tsid")) {
    AsyncWebParameter* p = request->getParam("tsid");
    id = p->value().toInt();
  }

  // check if there is any sampled data
  if ( !getTSsize(id) ) {
    request->send_P(503, PGmimejson, "[]");
    return;
  }

  // json response maybe pretty large and needs too much of a precious ram to store it in a temp 'string'
  // So I'm going to generate it on-the-fly and stream to client in chunks

  size_t cnt = 0;           // cnt - return last 'cnt' samples, 0 - all samples

  if (request->hasParam(C_scnt)){
    AsyncWebParameter* p = request->getParam(C_scnt);
    if (!p->value().isEmpty())
      cnt = p->value().toInt();
  }


  const auto ts = getTS(id);
  if (!ts)
    request->send_P(503, PGmimejson, "[]");

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
        buffer[0] = 0x5b;   // Open json array with ASCII '['
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

  response->addHeader(PGacao, "*");   // CORS header
  request->send(response);
}

// publish meter data via availbale EmbUI feeders (a periodic Task)
void Espem::wspublish(){
  if (!embui.feeders.available() || !pz)  // exit, if there are no clients connected
    return;

  const auto m = pz->getMetricsPZ004();

  DynamicJsonDocument doc(PUB_JSSIZE);
  JsonObject obj = doc.to<JsonObject>();
  doc["stale"] = pz->getState()->dataStale();
  doc["age"] = pz->getState()->dataAge();
  doc["U"] = m->voltage;
  doc["I"] = m->current;
  doc["P"] = m->power;
  doc["W"] = m->energy + ds.getEnergyOffset();
  doc["Pf"] = m->pf;
  doc["freq"] = m->freq;

  Interface interf(&embui.feeders, 128);
  interf.json_frame(C_espem);
  interf.jobject(doc, true);
  interf.json_frame_flush();
}

uint8_t Espem::set_uirate(uint8_t seconds){
  if (seconds){
    t_uiupdater.setInterval(seconds * TASK_SECOND);
    t_uiupdater.restartDelayed();
  } else
    t_uiupdater.disable();

  return seconds;
}

uint8_t Espem::get_uirate(){
  if (t_uiupdater.isEnabled())
    return (t_uiupdater.getInterval() / TASK_SECOND);

  return 0;
}

void DataStorage::reset(){
  purge();
  tsids.clear();

  uint8_t a;
  a = addTS(embui.paramVariant(V_TS_T1_CNT), time(nullptr), embui.paramVariant(V_TS_T1_INT), "Tier 1", 1);
  tsids.push_back(a);
  //LOG(printf, "Add TS: %d\n", a);

  a = addTS(embui.paramVariant(V_TS_T2_CNT), time(nullptr), embui.paramVariant(V_TS_T2_INT), "Tier 2", 2);
  tsids.push_back(a);
  //LOG(printf, "Add TS: %d\n", a);

  a = addTS(embui.paramVariant(V_TS_T3_CNT), time(nullptr), embui.paramVariant(V_TS_T3_INT), "Tier 3", 3);
  tsids.push_back(a);
  //LOG(printf, "Add TS: %d\n", a);

  LOG(println, "Setup TimeSeries DB:");
  LOG_CALL(
    for ( auto i : tsids ){
      auto t = getTS(i);
      if (t){
        LOG(printf, "%s: size:%d, interval:%u, mem:%u\n", t->getDescr(), t->capacity, t->getInterval(), t->capacity * sizeof(pz004::metrics));
      }
    }
  )

  LOG(printf, "SRAM: heap %u, free %u\n", ESP.getHeapSize(), ESP.getFreeHeap());
  LOG(printf, "SPI-RAM: size %u, free %u\n", ESP.getPsramSize(), ESP.getFreePsram());
}

mcstate_t Espem::set_collector_state(mcstate_t state){
  if (!pz){
    ts_state = mcstate_t::MC_DISABLE;
      return ts_state;
  }

  switch (state) {
    case mcstate_t::MC_RUN : {
      if (ts_state == mcstate_t::MC_RUN) return mcstate_t::MC_RUN;
      if (!ds.getTScap()) ds.reset();        // reinitialize TS Container if empty

        // attach collector's callback
        pz->attach_rx_callback([this](uint8_t id, const RX_msg* m){
          // collect time-series data
          if (!pz->getState()->dataStale()){
            ds.push(*(pz->getMetricsPZ004()), time(nullptr));
          }
          #ifdef ESPEM_DEBUG
            if (m) msgdebug(id, m);          // it will print every data packet coming from PZEM
          #endif
        });
        ts_state = mcstate_t::MC_RUN;
      break;
    }
    case mcstate_t::MC_PAUSE : {
      pz->detach_rx_callback();
      ts_state = mcstate_t::MC_PAUSE;
      break;
    }
    default: {
      pz->detach_rx_callback();
      ds.purge();
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

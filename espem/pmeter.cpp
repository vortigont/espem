/*  ESPEM - ESP Energy monitor
 *  A code for ESP8266/ESP32 based boards to interface with PeaceFair PZEM PowerMeters
 *  It can poll/collect PowerMeter data and provide it for futher processing in text/json format
 *
 *  (c) Emil Muratov 2018-2021  https://github.com/vortigont/espem
 *
 */

#include "espem.h"


void PMETER::begin(){
  #ifdef ESPEM_USE_HWSERIAL
      hwser = new HardwareSerial(HWSERIAL_PORT);

    #if defined(ESP32)
      #if defined (PIN_RX) && defined (PIN_TX)
        pzem = std::unique_ptr<PZEM>(new PZEM(hwser, PIN_RX, PIN_TX));  // Connect to PZEM via HW_serial
      #else
        pzem = std::unique_ptr<PZEM>(new PZEM(hwser));  // Connect to PZEM via HW_serial
      #endif
    #else // ESP8266
      pzem = std::unique_ptr<PZEM>(new PZEM(hwser));  // Connect to PZEM via HW_serial
    #endif

    #if defined ESPEM_HWSERIAL_SWAP && defined ESP8266
        hwser->swap();      //swap hw_serial pins to gpio13(rx),15(tx) for esp8266
    #endif
  #elif defined ESP8266 // softwareserial
    pzem = std::unique_ptr<PZEM>(new PZEM(PIN_RX, PIN_TX));  // Connect to PZEM via sw_serial pins
  #endif

  pzeminit();
}

// Initialize powermeter connection by setting ip addr
bool PMETER::pzeminit() {

  if (pzemrdy) return true;	// nothing to do

  LOG(print, F("Init PZEM..."));

#ifndef USE_PZEMv3
  pzemrdy = pzem->setAddress(ip);   // Set PowerMeter address  for an "old" PZEM
#else
  pzemrdy = true;   // just a stub for PZEM-004T-v30, TODO: implement some kind of a real link check
#endif

  LOG(println, pzemrdy ? F("OK!") : F("FAILED!"));
  return pzemrdy;
}

// function to read meterings from PZEM object and fill array with data
// read from object 'meter', write to array 'result', correct PF if 'PF_fix'
bool PMETER::pollMeter(std::unique_ptr<PZEM> &meter, pmeterData &result, bool fixpf) {

  if (!pzem)    // no object
    return false;

  if ( not pzemrdy ) {
      LOG(println, F("PZEM no link, reconnecting..."));
      if(!pzeminit())
        return false;
  }

  if (millis()-lastpoll < ESPEM_MAXPOLLRATE)    // won't poll with rate higher than MAX
    return true;

  bool isdata = false; // non-zero data from meter

  //fill the array with new meter data
  for ( uint8_t i = 0; i != 4; ++i) {
    #ifdef USE_PZEMv3
      result.meterings[i] = (*(meter).*pzdatafunc[i])();
    #else
      result.meterings[i] = (*(meter).*pzdatafunc[i])(ip);
    #endif

      if ( result.meterings[i] < 0) { pzemrdy = false; lastpoll = 0; return false; }         // return on error reading meter
      isdata = result.meterings[i] || isdata;
  }

  // consider it as an error if all meter values are equal to zero
  if (!isdata){
    lastpoll = 0;
    return false;
  }

  // correct pF value if required   I = P/U
  if (fixpf && (result.pf() > 1.0) )
    result.current = result.power / result.voltage;

/*
  String _t;
  mktxtdata(_t);
  LOG(printf, "Poll: %s\n", _t.c_str());
*/

  lastpoll=millis();	//reset poll timer
  return true;
}

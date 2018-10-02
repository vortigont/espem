/* 
 *  ESPEM - ESP Energy monitor
 *  A code for ESP8266 based boards to interface with PeaceFair PZEM PowerMeters
 *  It can poll/collect PowerMeter data and provide it for further processing in 
 *  text/JSON format
 *  Author           : Emil Muratow
 * 
 *  This file        : BoardWitty.cpp
 *  This file Author : Alexey Shtykov
 *  
 *  Description      : BoardWitty class implementation
 *  Created on 22 February 2018, 16:18
 * 
 */
#include "config.h"
#include "main.h"
#include "BoardWitty.h"

BoardWitty::BoardWitty() {
}

BoardWitty::BoardWitty(const BoardWitty& orig) {
}

BoardWitty::~BoardWitty() {
}

/**
 * initialize debug LEDs
 */
void BoardWitty::initialize()
{
#ifdef BOARD_WITTY
    pinMode((uint8_t)LED_r, (uint8_t)OUTPUT);
    pinMode((uint8_t)LED_g, (uint8_t)OUTPUT);
    pinMode((uint8_t)LED_b, (uint8_t)OUTPUT);
#endif  // BOARD_WITTY
}

/**
 * trigger light
 * @param light - light to change
 * @param bOn   - level (true=HIGH, false=LOW)
 */
void BoardWitty::triggerLight(BoardWitty::Lights light, bool bOn)
{
#ifdef BOARD_WITTY
    if(bOn)
        digitalWrite((uint8_t)light, (uint8_t)LOW);
    else
        digitalWrite((uint8_t)light, (uint8_t)HIGH);
#endif  // BOARD_WITTY
}
/* 
 *  ESPEM - ESP Energy monitor
 *  A code for ESP8266 based boards to interface with PeaceFair PZEM PowerMeters
 *  It can poll/collect PowerMeter data and provide it for further processing in 
 *  text/JSON format
 *  Author           : Emil Muratow
 * 
 *  This file        : NullSerial.cpp
 *  This file Author : Emil Muratow
 *  
 *  Description      : Null serial black hole to consume debug output
 *  Created on 19 February 2018, 17:34
 * 
 */
#include <cstdio>
#include "NullSerial.h"

void NullSerialClass::begin(int speed)
{
}

void NullSerialClass::println(int x, int type)
{
}

void NullSerialClass::println(const char *p)
{
}

void NullSerialClass::print(int x, int type)
{
}

void NullSerialClass::print(const char *p)
{
}

int NullSerialClass::available()
{
    return 0;
}

int NullSerialClass::read()
{
    return -1;
}

void NullSerialClass::flush()
{
}

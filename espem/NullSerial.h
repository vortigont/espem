/* 
 *  ESPEM - ESP Energy monitor
 *  A code for ESP8266 based boards to interface with PeaceFair PZEM PowerMeters
 *  It can poll/collect PowerMeter data and provide it for further processing in 
 *  text/JSON format
 *  Author           : Emil Muratow
 * 
 *  This file        : NullSerial.h
 *  This file Author : Emil Muratow
 *  
 *  Description      : Null serial black hole to consume debug output
 *  Created on 19 February 2018, 17:34
 * 
 */

#ifndef NULLSERIAL_H
#define NULLSERIAL_H
#ifndef DEC
#define DEC -1
#endif

class NullSerialClass // a do-nothing class to replace Serial
{
public:
    void begin(int speed);
    void println(int x, int type = DEC);
    void println(const char *p = 0);
    void print(int x, int type = DEC);
    void print(const char *p);
    int available();
    int read();
    void flush();
};
typedef NullSerialClass NullSerial;
#endif /* NULLSERIAL_H */


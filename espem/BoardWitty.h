/* 
 *  ESPEM - ESP Energy monitor
 *  A code for ESP8266 based boards to interface with PeaceFair PZEM PowerMeters
 *  It can poll/collect PowerMeter data and provide it for further processing in 
 *  text/JSON format
 *  Author           : Emil Muratow
 * 
 *  This file        : BoardWitty.h
 *  This file Author : Alexey Shtykov
 *  
 *  Description      : BoardWitty class definition
 *  Created on 22 February 2018, 16:18
 * 
 */
#ifndef BOARDWITTY_H
#define BOARDWITTY_H

class BoardWitty {
public:
    enum  Lights  {
        LED_g = 12,
        LED_r = 15,
        LED_b = 13
    };
    virtual ~BoardWitty();
    static void initialize();
    static void triggerLight(BoardWitty::Lights light=LED_g, bool bOn=false);
private:
    BoardWitty();
    BoardWitty(const BoardWitty& orig);
};

#endif /* BOARDWITTY_H */


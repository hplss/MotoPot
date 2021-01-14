/*
 * led_class.h
 *
 * Created: 12/31/2015 10:35:16 AM
 *  Author: Andrew
 */ 
#include <Arduino.h>
#include <Shifter.h>

#ifndef LED_CLASS_H_
#define LED_CLASS_H_

#define TIMEOUT 3000//msec
#define BLINK_INTERVAL 250 //msec

class Led
{
public:
Led(uint8_t x){ l_Pin = x; l_nextBlink = l_prevMillis = millis(); l_State = LOW; }
void loop ( void );
void SetState( uint8_t state );
void SetBlinks( uint8_t x, int interval = 250 ){ l_NumBlinks = x; l_blinkInterval = interval; }
void HandleBlinks( void );

private:
bool l_State;
uint8_t l_Pin; //Physical pin for LED on 74hc595
uint8_t l_NumBlinks = 0;
uint8_t l_TempBlinks = 0;
int l_blinkInterval;
long l_prevMillis;
long l_nextBlink;
};

extern Shifter shifter;


#endif /* LED_CLASS_H_ */
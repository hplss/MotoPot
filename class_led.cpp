/*
 * led_class.cpp
 *
 * Created: 12/31/2015 10:35:05 AM
 *  Author: Andrew
 */ 
#include <class_led.h>

void Led::loop( void ) 
{
	HandleBlinks();		
}
void Led::SetState( uint8_t state ) //Note: DO NOT CALL clear() HERE.
{
	if ( state == l_State )
		return; //Just stop here to prevent unnecessary write()
		
	shifter.setPin(l_Pin, state);
	shifter.write();
	l_State = state;
}
void Led::HandleBlinks( void )
{
	if ( l_NumBlinks > 0)
	{
		if ( millis() > l_nextBlink )
		{
			if (!l_State) //If false..
			{
				SetState( !l_State ); //Set opposite
				l_TempBlinks++;
			}
			else
				SetState( !l_State ); //Set false
				
			if ( l_NumBlinks == l_TempBlinks && !l_State ) //Reset
				l_NumBlinks = l_TempBlinks = 0;
				
			l_nextBlink = millis() + BLINK_INTERVAL;
		}
	}
}
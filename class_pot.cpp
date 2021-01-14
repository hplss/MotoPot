/*
 * class_pot.cpp
 *
 * Created: 12/31/2015 7:20:31 AM
 *  Author: Andrew
 */ 

#include <class_pot.h>

void Potentiometer::loop( void )
{
	p_LED->loop(); //Also call our LED's loop function :>
	
	if ( millis() > prevMillis )
	{
		if (!GetPosition())
		{
			prevMillis = millis() + IDLE_FREQ; //Update less often when known to be idle to save cycles.	
			p_Online = false;
		}	
	}
}
bool Potentiometer::GetPosition()
{
	int raw = analogRead(p_ReadPin); //Read the pin for a value.
	
	if (raw)
	{
		float buffer= raw * BASE_VOLTAGE;
		float Vout= (buffer)/1024.0;
		buffer= (BASE_VOLTAGE/Vout) -1;
		float R2 = DIVIDER_RESISTOR * buffer;
		if ( R2 < READ_IGNORE ) //Check to see if out-of-bounds
		{
			float temp = (12 + (R2 * .0078)); //Some numbers that have tested well. //.0078
			if ( g_DoUpdates && !IsUpdating() && (int(R2) < (p_prevReading - temp) || int(R2) > (p_prevReading + temp) ) && g_beeps == 0 && millis() > p_nextStatusUpdate )
			{
				Serial.print("P"); //
				Serial.print(p_motorNum);
				Serial.print(" ");
				Serial.println( int(R2) );
				p_nextStatusUpdate = millis() + 40;
				p_prevReading = int(R2);
			}
			UpdatePosition( R2 );
			p_Online = true;
			return true;
		}
		else
			return false;
	}
	else
		return false;
}

void Potentiometer::SetValue( int value )
{
	if ( value < KILL_HIGH && value > KILL_LOW ) //Over or under our limit to start the motor.
		p_gotoValue = value;
}
void Potentiometer::UpdatePosition( float pos )
{	
	if (!p_gotoValue || !g_DoUpdates )
		return;	 //Just stop here
		
	//See if other motors are updating before us.
	for ( uint8_t x = 0; x < MAX_POTS; x++ )
	{
		if ( v_Pots[x].p_motorNum != p_motorNum && v_Pots[x].IsUpdating() ) //Make sure we're not checking ourselves
			return;
	}
	//
	
	float distance = fabs(p_gotoValue - pos);

	if ( !p_TempPos )
	{
		p_TempPos = pos;
		p_timeoutMillis = millis() + POT_TIMEOUT;
	}
	else if ( millis() > p_timeoutMillis )
	{
		if ( fabs( p_TempPos - pos ) < POT_TIMEOUT_AMOUNT || fabs( p_TempPos + pos ) < POT_TIMEOUT_AMOUNT ) 
		{
			Reset(); //Kill the motor here
			Buzzer( 3, 500 ); //Buzzer
			p_LED->SetBlinks( 10, 500 );
			
			//error here
			Serial.print("Error! Motor ");
			Serial.print( p_motorNum );
			Serial.println(" Not Responding.");
			//
			return; //End it here
		}
		else //update our position and time
		{
			p_TempPos = pos;
			p_timeoutMillis = millis() + POT_TIMEOUT;
		}
	}
		
	bool pwm = ( distance > PWM_THRESHHOLD ) ? false : true;
	int threshhold = (p_gotoValue < ACCURACY_THRESHHOLD ) ? VALUE_THRESHHOLD_LOW : VALUE_THRESHHOLD_HIGH; //Variable accuracy rating
	
	if( ( p_gotoValue > ( pos + threshhold ) || p_gotoValue < ( pos - threshhold) ) )
	{
		p_Updating = true;
		
		if ( p_gotoValue  < pos ) //New value is LESS than existing
			SetPin( DECREASE, pwm );
			
		else if ( p_gotoValue > pos ) //New value is MORE than existing
			SetPin( INCREASE, pwm );
			
		dialinMillis = millis() + DIALIN_PERIOD;
		
		p_LED->SetState(true); //Turn motor indicator LED on
	}
	else //Can't do anything else, clear value
	{
		if ( p_Dialin && millis() < dialinMillis )
			return;
			
		Reset();	
	}
	//
}
void Potentiometer::Reset( void )
{
	p_Updating = false;
	SetPin( BOTH_LOW ); //Just in case
	p_LED->SetState(false); //Turn indicator LED back off
	p_timeoutMillis = p_TempPos = p_gotoValue = 0; //Reset so we don't process anymore
}
void Potentiometer::SetPin( uint8_t pin_type, bool pwm  ) 
{	
	//We'll use this to make absolutely sure that the opposing gates are closed while switching power.
	switch ( pin_type )
	{
		case INCREASE:
			shifter.setPin( p_DecPin, LOW ); //Make sure decrease is disabled before we increase values.
			if ( pwm && millis() > pwmMillis )
			{
				shifter.setPin( p_IncPin, LOW );
				pwmMillis = millis() + PWM_MSEC;
			}
			else
				shifter.setPin( p_IncPin, HIGH );
				
			break;
			
		case DECREASE:
			shifter.setPin( p_IncPin, LOW ); //Make sure increase is disabled before we decrease values.
			if ( pwm && millis() > pwmMillis )//better..
			{
				shifter.setPin( p_DecPin, LOW );
				pwmMillis = millis() + PWM_MSEC;
			}
			else
				shifter.setPin( p_DecPin, HIGH );
				
			break;
			
		case BOTH_LOW: //Set both pins to LOW - This was implemented this way to prevent redundant write()'s being called.
			shifter.setPin( p_DecPin, LOW );
			shifter.setPin( p_IncPin, LOW );
			break;
			
		default: //Just in case?
			shifter.setPin( p_DecPin, LOW );
			shifter.setPin( p_IncPin, LOW );
			break;
	}

	shifter.write(); //Fire away our changes
}

void Potentiometer::setup( uint8_t pin ) //Primary Setup function. TOUCH THESE AND I CUT YOU
{
	p_motorNum = pin;
	p_ReadPin = analogInputToDigitalPin( (MAX_POTS - 1) - pin ); //Yeah.. hacked up because I wired the inputs backwards.
	//Power pins for pot motor -- May need tweaks
	p_IncPin = pin * 2;
	p_DecPin = p_IncPin + 1;
	//
	p_LED = new Led((MAX_POTS * 2) + pin);
	pinMode( p_ReadPin, INPUT ); //Just to make sure. Technically, AnalogRead() does not require this.
	
	GetPosition(); //test
}

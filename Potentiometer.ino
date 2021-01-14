#include <Shifter.h>
#include <class_pot.h>
#include <class_led.h>
#include <StandardCplusplus.h>
#include <vector>

using namespace std;

#define SER_Pin 7 //SER_IN
#define RCLK_Pin 8 //L_CLOCK
#define SRCLK_Pin 9 //CLOCK

vector<Potentiometer> v_Pots;

//initaize shifter using the Shifter library
Shifter shifter(SER_Pin, RCLK_Pin, SRCLK_Pin, NUM_REGISTERS);

#define MOSFET_PIN 5 //Pin that is used by our power mosfet
#define POWER_SENSOR_PIN A5 //Analog pin used to detect our primary power source.
#define ARDUINO_TIMEOUT_INTERVAL 60000//msec

//Global Stuff for buzzer
uint8_t g_tempBeeps = 0;
uint8_t g_beeps = 0;
bool g_BState = false;
bool g_DoUpdates = true;
int g_tempinterval = 0;
long nextBeep = 0;
//Aux power related
bool b_PrimaryPower = false;
bool b_HasWarned = false;
long g_nextPowerMillis = 0;
long g_arduinoTimeout = 0;
//
char *c_PowerWarning = "Error! Power is disconnected.";

void Buzzer( uint8_t beeps, int interval )
{
	if ( g_beeps > 0 )//Already beeping.
		return; 
		
	g_beeps = beeps;
	g_tempinterval = interval;
}
void HandleBuzzer( void )
{
	if ( g_beeps > 0)
	{
		if ( millis() > nextBeep )
		{
			if (!g_BState) //If false..
			{
				analogWrite( BUZZER_PIN, 60 );
				g_tempBeeps++;
			}
			else
				analogWrite( BUZZER_PIN, 0 );
			
			g_BState = !g_BState;
			
			if ( g_beeps == g_tempBeeps && !g_BState ) //Reset
				g_beeps = g_tempBeeps = 0;
			
			nextBeep = millis() + g_tempinterval;
		}
	}
}
bool CheckPrimaryPower( bool force = false )
{
	if ( millis() < g_nextPowerMillis && !force ) //Don't check so often.
		return b_PrimaryPower;
			
	if ( analogRead( POWER_SENSOR_PIN ) > 1000 && ( millis() < g_arduinoTimeout || force ) )//Meh. Over 1000 is pretty solid. 
	{
		b_PrimaryPower = true;
		b_HasWarned = false;
	}
		
	else 
	{
		if ( !b_HasWarned && millis() < g_arduinoTimeout ) //Do not display the error for timeout
		{
			Serial.println( c_PowerWarning );
			b_HasWarned = true;
		}
		b_PrimaryPower = false;
	}

	digitalWrite( MOSFET_PIN, b_PrimaryPower ? HIGH : LOW);
	g_nextPowerMillis = millis() + 350; //Meh
	
	
	return b_PrimaryPower;
}
void setup()
{
	shifter.clear(); //Just to make sure. Default to 0.
	Serial.begin(57600);
	
	pinMode(MOSFET_PIN, OUTPUT);
	
	Serial.println("/r"); //Ready signal
	delay(15); //hacked up bullshit, the serial is trying to dump too much data at once, causing issues. slow it down here.
	
	g_arduinoTimeout = millis() + ARDUINO_TIMEOUT_INTERVAL; //Begin the countdown.
	
	for ( uint8_t x = 0; x < MAX_POTS; x++) //Set up pots here
	{
		v_Pots.push_back( *new Potentiometer( x ) );
		delay( 15 ); //hacked up bullshit, the serial is trying to dump too much data at once, causing issues. slow it down here.
	}
}

void (* ResetFunc) ( void ) = 0;

void loop()
{
	HandleBuzzer();
	
	CheckPrimaryPower();
	
	for ( uint8_t x = 0; x < MAX_POTS; x++ )
		v_Pots[x].loop(); //Call the loop of each pot
	
	
	if (Serial.available() > 0) //So theproblem with parseint is that it seems to cause a lag spike. :/
	{
		g_DoUpdates = false; //Till we're done gathering data.
		uint8_t motor = Serial.parseInt();
		
		g_arduinoTimeout = millis() + ARDUINO_TIMEOUT_INTERVAL; //Now that we've received some serial input, reset the timeout counter.
		CheckPrimaryPower(true); //Force power on, just in case
		
		if ( motor == 10 ) //HACK RESET
			ResetFunc();
		
		if (Serial.available() > 0)
		{
			if ( !b_PrimaryPower ) //Is the auxiliary power on?
			{
				Serial.println(c_PowerWarning);
				Buzzer( 1, 1000 );
				return; //Stop here if no power is supplied.
			}
			
			int value = Serial.parseInt(); //do this first -- because.... reasons
			
			if (motor > ( MAX_POTS - 1)) //0 - 4 are valid v_Pots.size()? whatever.
			{
				Serial.println("Error! Invalid Motor.");
				Buzzer( 2, 100 );
				return;
			}	
			Led & pLed = v_Pots[motor].GetLED(); //LED pointer
			
			if ( !v_Pots[motor].CheckOnline() )
			{
				Serial.print("Error! Motor ");
				Serial.print(v_Pots[motor].p_motorNum);
				Serial.println(" Offline.");
				pLed.SetBlinks( 2, 100 );
				Buzzer( 2, 100 );
				return;
			}
			else
				v_Pots[motor].SetValue( value );
				
		}
	}
	else
		g_DoUpdates = true;
}
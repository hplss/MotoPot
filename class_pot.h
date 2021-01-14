/*
 * class_pot.h
 *
 * Created: 12/31/2015 7:18:40 AM
 *  Author: Andrew
 */ 

#include <Arduino.h>
#include <Shifter.h>
#include <class_led.h>
#include <StandardCplusplus.h>
#include <vector>

using namespace std;

#ifndef CLASS_POT_H_
#define CLASS_POT_H_

//Defines for Shift Register switching in SetPin() DO NOT ALTER----
#define INCREASE 0
#define DECREASE 1
#define BOTH_LOW 2
//

#define BUZZER_PIN 10 //Error Buzzer

//CONST DEFINES HERE
#define MAX_POTS 5 //Total number of pots being controlled. Should only be changed for testing purposes.
#define IDLE_FREQ 350 // Time between checks that determine whether or not the motor is active. This should only occur in error.
#define NUM_REGISTERS 2 //how many registers are in use on the circuit .. DO NOT CHANGE
//
// Error Defines
#define POT_TIMEOUT 500 // Msec before motor generates an error.
#define POT_TIMEOUT_AMOUNT 500 //Necessary change in value before we count down to the timeout.
#define READ_IGNORE 10500 //Ignore any value over this amount.
//

//This controls the max readings allowed between motor movements. Used for safety CAREFUL WITH THESE
#define KILL_HIGH 9910 //never allow any values above this
#define KILL_LOW 10 //never allow values below this
//
//These values control the acuracy of our pots' adjustments.
#define PWM_THRESHHOLD 3000 // Amount in ohms in difference between adjustments when motor is controlled by PWM.
#define PWM_MSEC 3 //Frequency in which the shift register is pulsed. Used for throttling to ensure better accuracy below PWM_THRESHHOLD
#define ACCURACY_THRESHHOLD 2500 // Level that determines which VALUE_THRESHHOLD should be used, based on a specific Goto_Value
#define VALUE_THRESHHOLD_LOW 35 // margin for lower values
#define VALUE_THRESHHOLD_HIGH 75 // margin for higher values
#define DIALIN_PERIOD 25 //msec Doesn't need to be long at all. Just long ehough to compensate for the overshoot.
//

//Formula vars below ------
#define DIVIDER_RESISTOR 5120 //This is fairly well dialed in, despite the rating of 5.1k on the board. DO NOT TOUCH
#define BASE_VOLTAGE 5
//

class Potentiometer
{
public:
Potentiometer(uint8_t pin){ setup(pin); prevMillis = millis(); p_Online = false; }
void loop(void);
void setup(uint8_t pin);
//void SetPosition( int value );
void UpdatePosition( float pos ); //This function is what does all of he math related to motor positioning
void SetPin( uint8_t pin_type, bool pwm = false );
bool CheckOnline(){ return p_Online; } //Is our motor active?
bool IsUpdating(){ return p_Updating; } //Are we currently updating/moving?
void SetDialin( bool dial ){ p_Dialin = dial; } //used to disable/enable "Dial-in" feature
Led &GetLED( void ){ return *p_LED; }
void SetValue( int value );
void Reset( void );
bool GetPosition( void );
uint8_t GetReadPin(){ return p_ReadPin; }

uint8_t p_motorNum;

private:
//Physical pin numbers -- DO NOT TOUCH
uint8_t p_ReadPin;
uint8_t p_IndicPin; //pin used for indicator LED designated to pot
uint8_t p_IncPin; //we'll see
uint8_t p_DecPin;
//
Led *p_LED; //LED object

bool p_Online;
bool p_Dialin = true; //Default to on.
bool p_Updating = false;

int p_gotoValue = 0;
int p_prevReading = 0;

float p_TempPos; //Used to save off previously recorded value. 

long p_timeoutMillis = 0;
long prevMillis;
long pwmMillis = 0;
long dialinMillis = 0;
long p_nextStatusUpdate = 0;
};

void Buzzer( uint8_t beeps, int interval = 200 );

extern Shifter shifter; //Say hello to the shifter object
extern vector<Potentiometer> v_Pots; //Yeah.. for hax
extern bool g_DoUpdates;
extern uint8_t g_beeps;

#endif /* CLASS_POT_H_ */
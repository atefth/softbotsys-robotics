/******************************************************************************************
 * Rotary-encoder decoder for Arduino
 ******************************************************************************************
 * Copyright (c) Robert Bakker 2013
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 ******************************************************************************************
 * This library handles polling and decoding a rotary encoder.
 * It is non-blocking code that executes in the background while your sketch
 * does it's thing.
 *****************************************************************************************/

#include "Encoder.h"
#include "Arduino.h"	// Has just about everything in it


/*Private #defines************************************************************************/

// Used by state machines
#define STANDBY 0
#define WAITING 2
#define IDLE 4


/*Declare Variables***********************************************************************/

struct encoderVars
{
	volatile uint8_t pin_A;
	volatile uint8_t pin_B;

	volatile uint8_t state;
	volatile int8_t data;

	volatile uint8_t pinState_A;
	volatile uint8_t pinStateOld_A;
	volatile uint8_t pinState_B;
};

// Initialize variables (start all state machines in STANDBY state)
encoderVars encoder_1 = { 0, 0,  STANDBY, 0,  0, 0, 0 };
encoderVars encoder_2 = { 0, 0,  STANDBY, 0,  0, 0, 0 };


/*Start of Functions**********************************************************************/

// This function sets up timer2 to trigger an ISR every 300 us.
// It also sets up the input pins.
void encoder_1_begin(uint8_t pin_A, uint8_t pin_B)
{
	encoder_1.pin_A = pin_A;
	encoder_1.pin_B = pin_B;

	pinMode(encoder_1.pin_A, INPUT);
	pinMode(encoder_1.pin_B, INPUT);

	// Configure timer 2
	cli();					// Disable global interrupts

	TCCR2A = 0;				// Clear timer2's control registers
	TCCR2B = 0;
	TIMSK2 = 0;				// ...and interrupt mask register (just in case)
	TCNT2 = 0;				// Pre-load the timer to 0
	OCR2A = 149;			// Set output compare register to 149
	TCCR2A |= _BV(WGM21);	// Turn on CTC mode (Clear Timer on Compare match)
	TCCR2B |= 0b011;		// Set prescaler to 32 (starts timer) 
	TIMSK2 |= _BV(OCIE2A);	// Enable timer compare interrupt 

	sei();					// Re-enable global interrupts
}

void encoder_2_begin(uint8_t pin_A, uint8_t pin_B)
{
	encoder_2.pin_A = pin_A;
	encoder_2.pin_B = pin_B;

	pinMode(encoder_2.pin_A, INPUT);
	pinMode(encoder_2.pin_B, INPUT);

	// Configure timer 2
	cli();					// Disable global interrupts

	TCCR2A = 0;				// Clear timer2's control registers
	TCCR2B = 0;
	TIMSK2 = 0;				// ...and interrupt mask register (just in case)
	TCNT2 = 0;				// Pre-load the timer to 0
	OCR2A = 149;			// Set output compare register to 149
	TCCR2A |= _BV(WGM21);	// Turn on CTC mode (Clear Timer on Compare match)
	TCCR2B |= 0b011;		// Set prescaler to 32 (starts timer) 
	TIMSK2 |= _BV(OCIE2A);	// Enable timer compare interrupt 

	sei();					// Re-enable global interrupts
}

int8_t encoder_1_data(void)
{
	if(encoder_1.state == IDLE)
	{
		int8_t encoder_1_Temp = encoder_1.data;
		encoder_1.data = 0;

		// This must always be done last (if it isn't, the state machine could restart 
		// with the other vairable(s) not initialized (ie. data, counter, etc.)).
		encoder_1.state = STANDBY;

		return encoder_1_Temp;
	}

	else
	{
		return 0;
	}
}

int8_t encoder_2_data(void)
{
	if(encoder_2.state == IDLE)
	{
		int8_t encoder_2_Temp = encoder_2.data;
		encoder_2.data = 0;

		// This must always be done last (if it isn't, the state machine could restart 
		// with the other vairable(s) not initialized (ie. data, counter, etc.)).
		encoder_2.state = STANDBY;

		return encoder_2_Temp;
	}

	else
	{
		return 0;
	}
}


/*ISR*************************************************************************************/

// This is the interrupt itself
// It is only active if encoder_begin() is called
// This polls the input pins and decodes the incoming signals.
// "Decoding" is done using a state machine.
ISR(TIMER2_COMPA_vect)
{
	// State machine for encoder
	switch(encoder_1.state)
	{
		case STANDBY:
			// First read encoder_1Pin_A's current state into pinStateOld_A.
			encoder_1.pinStateOld_A = digitalRead(encoder_1.pin_A);
			//Next step
			encoder_1.state = WAITING;
			break;

		case WAITING:
			// Read encoder_1Pin_A into different variable
			encoder_1.pinState_A = digitalRead(encoder_1.pin_A);
			// If it has changed since the STANDBY state... (ie. encoder_1 is turning)
			if(encoder_1.pinState_A != encoder_1.pinStateOld_A)
			{
				// ...read encoder_1Pin_B's state...
				encoder_1.pinState_B = digitalRead(encoder_1.pin_B);
				// ...and use it to decide what direction the encoder_1's turning
				if(encoder_1.pinState_A == encoder_1.pinState_B)
				{
					// Set data
					encoder_1.data = 1;
				}

				else
				{
					// Set data
					encoder_1.data = -1;
				}

				// Next step (do nothing until further notice)
				encoder_1.state = IDLE;
			}
			break;
	}
}

ISR(TIMER0_COMPA_vect)
{
	// State machine for encoder
	switch(encoder_2.state)
	{
		case STANDBY:
			// First read encoder_1Pin_A's current state into pinStateOld_A.
			encoder_2.pinStateOld_A = digitalRead(encoder_2.pin_A);
			//Next step
			encoder_2.state = WAITING;
			break;

		case WAITING:
			// Read encoder_1Pin_A into different variable
			encoder_2.pinState_A = digitalRead(encoder_2.pin_A);
			// If it has changed since the STANDBY state... (ie. encoder_1 is turning)
			if(encoder_2.pinState_A != encoder_2.pinStateOld_A)
			{
				// ...read encoder_1Pin_B's state...
				encoder_2.pinState_B = digitalRead(encoder_2.pin_B);
				// ...and use it to decide what direction the encoder_1's turning
				if(encoder_2.pinState_A == encoder_2.pinState_B)
				{
					// Set data
					encoder_2.data = 1;
				}

				else
				{
					// Set data
					encoder_2.data = -1;
				}

				// Next step (do nothing until further notice)
				encoder_2.state = IDLE;
			}
			break;
	}
}

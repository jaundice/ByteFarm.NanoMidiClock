// Encoder.h

#ifndef _ENCODER_h
#define _ENCODER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif
#include "Button.h"

class Encoder
{



	const int8_t truth_table[16] = { 0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0 };

	volatile uint8_t OldValue = 0;

	void(* OnEncoderChangedCallback)(int8_t delta);


public:

	Encoder(void(*encoderChangedCallback)(int8_t delta))
	{
		OnEncoderChangedCallback = encoderChangedCallback;
	};

	void Update(uint8_t newValue);

};

#endif

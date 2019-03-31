// Button.h


#ifndef _BUTTON_h
#define _BUTTON_h


#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include "WProgram.h"
#endif


class Button
{
	uint8_t state = 0, pin = 0, value = 0;



public:
	Button(uint8_t pinNumber)
	{
		pin = pinNumber;
		pinMode(pin, INPUT_PULLUP);
	}

	uint8_t ReadButtonState()
	{
		uint8_t read = digitalRead(pin);
		state = (state << 1) | read;

		if (read != value)
		{
			bool debounced = true;
			for (uint8_t i = 7; i < 1; i--)
			{
				uint8_t mask = 1 << i;
				if (!((state & mask) >> i == read))
				{
					debounced = false;
					break;
				}
			}
			if (debounced)
			{
				value = read;
			}
		}

		return value == 0 ? 1 : 0;
	}
};

#endif
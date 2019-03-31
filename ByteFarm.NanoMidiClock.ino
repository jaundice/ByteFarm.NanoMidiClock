/*
	Name:       ByteFarm.NanoMidiClock.ino
	Created:	28/03/2019 23:58:43
	Author:     RALPH\John
*/

#include "Encoder.h"
#include "UIButtonStates.h"
#include <Arduino.h>
#include <uClock.h>
#include <LiquidCrystal_I2C.h>
#include "Button.h"
#include <avr/interrupt.h>
//#include <interrupt.h>


using namespace umodular::clock;

// MIDI clock, start and stop byte definitions - based on MIDI 1.0 Standards.
#define MIDI_CLOCK 0xF8
#define MIDI_START 0xFA
#define MIDI_STOP  0xFC

#define EncoderSwitchPin 8
#define StopSwitchPin  9
#define PlaySwitchPin 10
#define EncoderCLKPin 11
#define EncoderDTPin 12

bool IsPlaying = false;

long long nextUIUpdate = 0;

#define UIUpdateMillis 50

volatile uint16_t Tempo = 126;


// The callback function which will be called by Clock each Pulse of 96PPQN clock resolution.
void ClockOut96PPQN(uint32_t * tick)
{
	// Send MIDI_CLOCK to external gears
	Serial.write(MIDI_CLOCK);
}

// The callback function which will be called when clock starts by using Clock.start() method.
void onClockStart()
{
	Serial.write(MIDI_START);
}

// The callback function which will be called when clock stops by using Clock.stop() method.
void onClockStop()
{
	Serial.write(MIDI_STOP);
}





LiquidCrystal_I2C lcd(0x20, 16, 2);

Button PlayButton(PlaySwitchPin);
Button StopButton(StopSwitchPin);
Button EncoderButton(EncoderSwitchPin);

void TempoChange(int8_t delta)
{
	Tempo += delta;
	uClock.setTempo(Tempo);
}

Encoder ENC(TempoChange);



void setup()
{
	pinMode(EncoderCLKPin, INPUT_PULLUP);
	pinMode(EncoderDTPin, INPUT_PULLUP);

	PCICR |= bit(PCIE0);     // set PCIE0 to enable PCMSK0 scan
	PCMSK0 |= bit(PCINT3);   // set PCINT0 to trigger an interrupt on state change 
	PCMSK0 |= bit(PCINT4);

	sei();  // turn on interrupts



	lcd.init();                      // initialize the lcd 
  // Print a message to the LCD.
	lcd.backlight();
	lcd.setCursor(0, 0);
	lcd.print("Midi Clock v0.1a");

	lcd.clear();
	// Initialize serial communication at 31250 bits per second, the default MIDI serial speed communication:
	Serial.begin(31250);

	// Inits the clock
	uClock.init();
	// Set the callback function for the clock output to send MIDI Sync message.
	uClock.setClock96PPQNOutput(ClockOut96PPQN);
	// Set the callback function for MIDI Start and Stop messages.
	uClock.setOnClockStartOutput(onClockStart);
	uClock.setOnClockStopOutput(onClockStop);
	// Set the clock BPM to 126 BPM
	uClock.setTempo(Tempo);

	// Starts the clock, tick-tac-tick-tac...
	//uClock.start();
}

UIButtonStates ReadButtonStates()
{
	const UIButtonStates states = {
		PlayButton.ReadButtonState(),
		StopButton.ReadButtonState(),
		EncoderButton.ReadButtonState(), 0 };
	return states;
}



void DoUI(const UIButtonStates buttons)
{
	lcd.setCursor(0, 0);


	uint8_t display = buttons.PlayButton | buttons.StopButton << 1 | buttons.EncoderButton << 2;

	lcd.print(display);

	lcd.print(" ");

	lcd.print(IsPlaying ? "    Running" : "Not Running");

	lcd.setCursor(0, 1);

	lcd.print("BPM ");

	lcd.print(Tempo);



}

void loop()
{
	const UIButtonStates buttons = ReadButtonStates();

	if (buttons.PlayButton && buttons.StopButton)
	{
		//todo settings menu
	}
	else if (buttons.PlayButton)
	{
		if (!IsPlaying)
		{
			uClock.start();
			IsPlaying = true;
		}
	}
	else if (buttons.StopButton)
	{
		if (IsPlaying)
		{
			uClock.stop();
			IsPlaying = false;
		}
	}

	long long mi = millis();
	if (mi > nextUIUpdate)
	{
		DoUI(buttons);
		nextUIUpdate = mi + UIUpdateMillis;
	}

}



ISR(PCINT0_vect)
{
	cli();
	const uint8_t pinMask = 0b00011000;
	const uint8_t pins = (PINB & pinMask) >> 3;
	ENC.Update(pins);
	sei();
}

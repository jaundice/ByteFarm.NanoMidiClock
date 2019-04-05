/*
	Name:       ByteFarm.NanoMidiClock.ino
	Created:	28/03/2019 23:58:43
	Author:     RALPH\John
*/

#include <splash.h>
#include <Adafruit_SSD1306.h>
#include <gfxfont.h>
#include <Adafruit_SPITFT_Macros.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_GFX.h>
#include <fixedWidthFont.h>
#include "Encoder.h"
//#include "UIButtonStates.h"
#include <Arduino.h>
#include <uClock.h>
//#include "Button.h"
#include <avr/interrupt.h>


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

volatile bool IsPlaying = false;

long long nextUIUpdate = 0;

#define UIUpdateMillis 30

volatile uint16_t Tempo = 126;


// The callback function which will be called by Clock each Pulse of 96PPQN clock resolution.
void ClockOut96PPQN(uint32_t* tick)
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


//LiquidCrystal_I2C lcd(0x20, 16, 2);

#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(-1);


//Button PlayButton(PlaySwitchPin);
//Button StopButton(StopSwitchPin);
//Button EncoderButton(EncoderSwitchPin);

volatile int8_t EncoderDelta = 0;

void TempoChange(int8_t delta)
{
	EncoderDelta += delta;

	int8_t mod = digitalRead(EncoderSwitchPin) == LOW ? 16 : 4;

	if (EncoderDelta % mod == 0)
	{
		uClock.setTempo(Tempo += EncoderDelta / mod);
		EncoderDelta = 0;
	}
}

Encoder ENC(TempoChange);


void InitOLED()
{
	display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
	display.clearDisplay();
	display.setTextSize(2);
	display.setTextColor(WHITE);
}

void setup()
{
	pinMode(EncoderCLKPin, INPUT_PULLUP);
	pinMode(EncoderDTPin, INPUT_PULLUP);
	pinMode(EncoderSwitchPin, INPUT_PULLUP);
	pinMode(PlaySwitchPin, INPUT_PULLUP);
	pinMode(StopSwitchPin, INPUT_PULLUP);


	PCICR |= bit(PCIE0); // set PCIE0 to enable PCMSK0 scan
	//PCMSK0 |= bit(PCINT0); // set PCINT0 to trigger an interrupt on state change 
	PCMSK0 |= bit(PCINT1); // set PCINT0 to trigger an interrupt on state change 
	PCMSK0 |= bit(PCINT2); // set PCINT0 to trigger an interrupt on state change 
	PCMSK0 |= bit(PCINT3); // set PCINT0 to trigger an interrupt on state change 
	PCMSK0 |= bit(PCINT4);

	sei(); // turn on interrupts


	//InitLCD();

	InitOLED();

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
}

//UIButtonStates ReadButtonStates()
//{
//	const UIButtonStates states = {
//		PlayButton.ReadButtonState(),
//		StopButton.ReadButtonState(),
//		EncoderButton.ReadButtonState() };
//	return states;
//}


void DoUI()
{
	display.clearDisplay();

	display.setCursor(0, 0);

	display.print(IsPlaying ? "Running" : "Stopped");

	display.setCursor(0, 32);
	display.print("BPM ");

	display.print(Tempo);

	display.display();
}


void Play()
{
	if (!IsPlaying)
	{
		uClock.start();
		IsPlaying = true;
	}
}

void Stop()
{
	if (IsPlaying)
	{
		uClock.stop();
		IsPlaying = false;
	}
}

void loop()
{
	long long mi = millis();
	if (mi > nextUIUpdate)
	{
		DoUI();
		nextUIUpdate = mi + UIUpdateMillis;
		delay(UIUpdateMillis);
	}
	
}

volatile uint8_t OldPINB = 0;

ISR(PCINT0_vect)
{
	const uint8_t changedPins = (PINB ^ OldPINB) & 0b00011111;

	OldPINB = PINB;


	const bool encoderChanged = (changedPins & 0b00011000) > 0;

	const uint8_t start = (changedPins & 0b00000100) > 0 ? PINB & 0b00000100 : 0;

	const uint8_t stop = (changedPins & 0b00000010) > 0 ? PINB & 0b00000010 : 0;


	if (encoderChanged)
	{
		const uint8_t pinMask = 0b00011000;
		const uint8_t pins = (PINB & pinMask) >> 3;
		ENC.Update(pins);
	}
	//else if ((start | stop) > 0 && (PINB & 0b00000100) > 0 && (PINB & 0b00000010) > 0)
	//{
	//	//todo change mode
	//}
	else if (start > 0)
	{
		Play();
	}
	else if (stop > 0)
	{
		Stop();
	}
}

// UIButtonStates.h

#ifndef _UIBUTTONSTATES_h
#define _UIBUTTONSTATES_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include "Encoder.h"
typedef struct {
public:
	uint8_t PlayButton;
	uint8_t StopButton;
	uint8_t EncoderButton;
	int8_t EncoderState;
} UIButtonStates;

#endif


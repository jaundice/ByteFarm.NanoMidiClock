// 
// 
// 

#include "Encoder.h"

void Encoder::Update(uint8_t newValue)
{
	uint8_t changed = newValue ^ OldValue;

	if (changed == 0)
		return;

	uint8_t key = (OldValue << 2) | newValue;

	int8_t ret = truth_table[key & 0b111];

	OldValue = newValue;

	OnEncoderChangedCallback(ret);
}



#ifndef GBX_KEYS_HPP_
#define GBX_KEYS_HPP_
#include <Utix/Ints.h>


namespace gbx {

struct Keys
{
	union
	{
		struct 
		{
			uint8_t right : 1;
			uint8_t left : 1;
			uint8_t up : 1;
			uint8_t down : 1;

			uint8_t a : 1;
			uint8_t b : 1;
			uint8_t select : 1;
			uint8_t start : 1;
		}bit;

		uint8_t all;
	}pad;

	uint8_t value;
};










}

#endif

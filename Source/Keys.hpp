#ifndef GBX_KEYS_HPP_
#define GBX_KEYS_HPP_
#include <Utix/Ints.h>


namespace gbx {

union Keys
{
	struct
	{
		uint8_t a : 1;
		uint8_t b : 1;
		uint8_t select : 1;
		uint8_t start : 1;
	}keys1;

	struct
	{
		uint8_t right : 1;
		uint8_t left : 1;
		uint8_t up : 1;
		uint8_t down : 1;
	}keys2;

	uint8_t value;
};










}

#endif
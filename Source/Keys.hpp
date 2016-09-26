#ifndef GBX_KEYS_HPP_
#define GBX_KEYS_HPP_
#include <stdint.h>


namespace gbx {


struct Keys 
{
	enum State : uint8_t {
		Up = 0x01,
		Down = 0x00
	};

	uint8_t value;

	union {
		struct {
			uint8_t right  : 1;
			uint8_t left   : 1;
			uint8_t up     : 1;
			uint8_t down   : 1;
			uint8_t a      : 1;
			uint8_t b      : 1;
			uint8_t select : 1;
			uint8_t start  : 1;
		};

		uint8_t value;
	}pad;
};




} // namespace gbx
#endif


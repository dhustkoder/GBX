#ifndef GBX_JOYPAD_HPP_
#define GBX_JOYPAD_HPP_
#include <stdint.h>
#include "hwstate.hpp"

namespace gbx {


struct Joypad 
{
	enum KeyState : uint8_t {
		Up = 0x01,
		Down = 0x00
	};
	enum Mode : uint8_t {
		Both = 0x00,
		Buttons = 0x01,
		Directions = 0x02,
	};

	union {
		uint8_t value;
		struct {
			uint8_t keys  : 4;
			uint8_t mode  : 2;
			uint8_t dummy : 2;
		};
	} reg;

	union {
		uint8_t value : 4;
		struct {
			uint8_t a      : 1;
			uint8_t b      : 1;
			uint8_t select : 1;
			uint8_t start  : 1;
		};
	} buttons;

	union {
		uint8_t value : 4;
		struct {
			uint8_t right : 1;
			uint8_t left  : 1;
			uint8_t up    : 1;
			uint8_t down  : 1;
		};
	} directions;
};

 
inline void update_joypad(const uint32_t(&keycodes)[8], const uint32_t keycode,
       const Joypad::KeyState state, HWState* const /*hwstate*/, Joypad* const pad)
{
	int keycode_index = 0;
	for (const auto key : keycodes) {
		if (key != keycode) {
			++keycode_index;
			continue;
		}
		break;
	}

	switch (keycode_index) {
	case 0: pad->buttons.a = state; break;
	case 1: pad->buttons.b = state; break;
	case 2: pad->buttons.select = state; break;
	case 3: pad->buttons.start = state; break;
	case 4: pad->directions.right = state; break;
	case 5: pad->directions.left = state; break;
	case 6: pad->directions.up = state; break;
	case 7: pad->directions.down = state; break;
	default: break;
	}
}


} // namespace gbx
#endif


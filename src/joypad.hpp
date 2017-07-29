#ifndef GBX_JOYPAD_HPP_
#define GBX_JOYPAD_HPP_
#include <stdint.h>
#include "input.hpp"
#include "hwstate.hpp"

namespace gbx {


enum class KeyState : uint8_t {
	Up = 0x01,
	Down = 0x00
};

enum class JoypadMode : uint8_t {
	Both = 0x00,
	Buttons = 0x01,
	Directions = 0x02,
};

struct Joypad {
	union {
		uint8_t value;
		struct {
			uint8_t keys : 4;
			uint8_t mode : 2;
		};
	} reg;

	union {
		uint8_t both : 8;
		struct {
			uint8_t buttons : 4;
			uint8_t directions : 4;
		};
	} keys;
};


inline void update_joypad(const uint32_t(&keycodes)[8],
                          const uint32_t keycode, const KeyState state,
			  HWState* const /*hwstate*/, Joypad* const pad)
{
	for (int i = 0; i < 8; ++i) {
		if (keycodes[i] == keycode) {
			if (state == KeyState::Up)
				pad->keys.both = set_bit(i, pad->keys.both);
			else
				pad->keys.both = res_bit(i, pad->keys.both);
			break;
		}
	}
}


} // namespace gbx
#endif


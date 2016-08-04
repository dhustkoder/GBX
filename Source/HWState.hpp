#ifndef GBX_HWSTATE_HPP_
#define GBX_HWSTATE_HPP_
#include <Utix/Ints.h>

namespace gbx {


enum Interrupts : uint8_t
{
	INTERRUPT_VBLANK = 0x01,
	INTERRUPT_LCD_STAT = 0x02,
	INTERRUPT_TIMER = 0x04,
	INTERRUPT_SERIAL = 0x08,
	INTERRUPT_JOYPAD = 0x10
};


struct HWState
{
	enum HWFlags : uint8_t
	{
		INTERRUPT_MASTER_ENABLED = 0x01,
		INTERRUPT_MASTER_ACTIVE = 0x02
	};

	uint8_t hwflags;
	uint8_t interrupt_enable;
	uint8_t interrupt_flags;
};



}
#endif
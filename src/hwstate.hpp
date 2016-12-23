#ifndef GBX_HWSTATE_HPP_
#define GBX_HWSTATE_HPP_
#include <stdint.h>

namespace gbx {

struct Interrupt {
	const uint8_t mask;
	const uint16_t addr;
};

constexpr const struct {
	const Interrupt vblank  { 0x01, 0x40 };
	const Interrupt lcd     { 0x02, 0x48 };
	const Interrupt timer   { 0x04, 0x50 };
	const Interrupt serial  { 0x08, 0x58 };
	const Interrupt joypad  { 0x10, 0x60 };
	const Interrupt array[5] { vblank, lcd, timer, serial, joypad };

	constexpr const Interrupt* begin() const
	{
		return &array[0]; 
	}

	constexpr const Interrupt* end() const
	{
		return begin() + 5;
	}
} kInterrupts;

constexpr const int16_t kTimaClockLimits[] { 1024, 16, 64, 256 };

struct HWState {
	int16_t tima_clock;
	int16_t tima_clock_limit;
	int16_t div_clock;
	
	struct {
		uint8_t ime : 2;
		bool cpu_halt : 1;
	} flags;

	uint8_t div;
	uint8_t tima;
	uint8_t tma;
	uint8_t tac;
	uint8_t int_enable;
	uint8_t int_flags;
};


inline uint8_t get_pendent_interrupts(const HWState& hwstate)
{
	return hwstate.int_enable & hwstate.int_flags;
}


inline void enable_interrupt(const Interrupt inter, HWState* const hwstate)
{
	hwstate->int_enable |= inter.mask;
}


inline void disable_interrupt(const Interrupt inter, HWState* const hwstate)
{
	hwstate->int_enable &= ~inter.mask;
}


inline void request_interrupt(const Interrupt inter, HWState* const hwstate)
{
	hwstate->int_flags |= inter.mask;
}


inline void clear_interrupt(const Interrupt inter, HWState* const hwstate)
{
	hwstate->int_flags &= ~inter.mask;
}

inline void inc_tima(HWState* const hwstate)
{
	if (++hwstate->tima == 0) {
		hwstate->tima = hwstate->tma;
		request_interrupt(kInterrupts.timer, hwstate);
	}
}


} // namespace gbx
#endif


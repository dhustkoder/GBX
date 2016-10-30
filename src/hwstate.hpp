#ifndef GBX_HWSTATE_HPP_
#define GBX_HWSTATE_HPP_
#include <stdint.h>

namespace gbx {

struct Interrupt {
	const uint8_t mask;
	const uint16_t addr;
};

namespace interrupts {
	constexpr const Interrupt vblank  { 0x01, 0x40 };
	constexpr const Interrupt lcd     { 0x02, 0x48 };
	constexpr const Interrupt timer   { 0x04, 0x50 };
	constexpr const Interrupt serial  { 0x08, 0x58 };
	constexpr const Interrupt joypad  { 0x10, 0x60 };
	constexpr const Interrupt array[] { vblank, lcd, timer, serial, joypad };
}


struct HWState 
{
	enum Flags : uint8_t {
		IntMasterEnable = 0x01,
		IntMasterActive = 0x02,
		CpuHalt = 0x04
	};

	int16_t tima_clock;
	int16_t tima_clock_limit;
	int16_t div_clock;
	uint8_t flags;
	uint8_t div;
	uint8_t tima;
	uint8_t tma;
	uint8_t tac;
	uint8_t int_enable;
	uint8_t int_flags;
};


constexpr HWState::Flags operator|(const HWState::Flags f1, const HWState::Flags f2)
{
	return static_cast<HWState::Flags>
		(static_cast<uint8_t>(f1) | static_cast<uint8_t>(f2));
}


constexpr HWState::Flags operator&(const HWState::Flags f1, const HWState::Flags f2)
{
	return static_cast<HWState::Flags>
		(static_cast<uint8_t>(f1) & static_cast<uint8_t>(f2));
}



inline uint8_t get_pendent_interrupts(const HWState& hwstate)
{
	return 0x1f & hwstate.int_enable & hwstate.int_flags;
}


inline HWState::Flags get_flags(const HWState& hwstate, const HWState::Flags hwflags)
{
	return static_cast<HWState::Flags>(hwstate.flags & hwflags);
}


inline void set_flags(const HWState::Flags hwflags, HWState* const hwstate)
{
	hwstate->flags |= hwflags;
}


inline void clear_flags(const HWState::Flags hwflags, HWState* const hwstate)
{
	hwstate->flags &= ~hwflags;
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



} // namespace gbx
#endif


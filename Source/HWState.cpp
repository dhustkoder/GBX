#include "HWState.hpp"
#include "Gameboy.hpp"

namespace gbx {


void Gameboy::UpdateHWState(const uint8_t cycles)
{
	if ((hwstate.div_clock + cycles) >= 0x100)
		++hwstate.div;
	hwstate.div_clock += cycles;

	if (!hwstate.GetFlags(HWState::TIMER_STOP)) {
		hwstate.tima_clock += cycles;
		if (hwstate.tima_clock >= hwstate.tima_clock_limit) {
			if (hwstate.tima < 0xff) {
				++hwstate.tima;
			} else {
				hwstate.tima = hwstate.tma;
				hwstate.RequestInt(INT_TIMER);
			}

			hwstate.tima_clock -= hwstate.tima_clock_limit;
		}
	}
}







void Gameboy::UpdateInterrupts()
{
	if (hwstate.GetFlags(HWState::CPU_HALT) && (hwstate.int_flags&0x1f)) {
		++cpu.pc;
		hwstate.ClearFlags(HWState::CPU_HALT);
	}

	if (!hwstate.GetIntMaster()) {
		return;
	} else if (!hwstate.GetIntActive()) {
		hwstate.EnableIntActive();
		return;
	}

	const uint8_t pendents = hwstate.GetPendentInts();

	if (!pendents)
		return;

	hwstate.DisableIntMaster();

	const auto trigger = [this](const IntMask inter, const uint16_t addr) {
		hwstate.ClearInt(inter);
		PushStack16(cpu.pc);
		cpu.pc = addr;
		cpu.clock += 12;
	};

	if (pendents & INT_VBLANK)
		trigger(INT_VBLANK, 0x40);

	if (pendents & INT_LCD_STAT)
		trigger(INT_LCD_STAT, 0x48);

	if (pendents & INT_TIMER)
		trigger(INT_TIMER, 0x50);

	if (pendents & INT_SERIAL)
		trigger(INT_SERIAL, 0x58);

	if (pendents & INT_JOYPAD)
		trigger(INT_JOYPAD, 0x60);
}






















}

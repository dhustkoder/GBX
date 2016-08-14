#include "HWState.hpp"
#include "Gameboy.hpp"

namespace gbx {


void Gameboy::UpdateHWState(const uint8_t cycles)
{
	hwstate.div_clock += cycles;
	if (hwstate.div_clock >= 0x100) {
		++hwstate.div;
		hwstate.div_clock -= 0x100;
	}

	if (!hwstate.GetFlags(HWState::TIMER_STOP)) {
		hwstate.tima_clock += cycles;
		if (hwstate.tima_clock >= hwstate.tima_clock_limit) {
			if (hwstate.tima < 0xff) {
				++hwstate.tima;
			}
			else {
				hwstate.tima = hwstate.tma;
				hwstate.RequestInt(INTERRUPT_TIMER);
			}

			hwstate.tima_clock -= hwstate.tima_clock_limit;
		}
	}
}







void Gameboy::UpdateInterrupts()

{
	if (!hwstate.GetIntMaster()) {
		return;
	}
	else if(!hwstate.GetIntActive()) {
		hwstate.EnableIntActive();
		return;
	}

	const uint8_t pendents = hwstate.GetPendentInts();

	if (!pendents)
		return;

	hwstate.DisableIntMaster();

	const auto push_pc = [this] {
		if (hwstate.GetFlags(HWState::CPU_HALT)) {
			PushStack16(cpu.GetPC() + 1);
			hwstate.ClearFlags(HWState::CPU_HALT);
		}
		else {
			PushStack16(cpu.GetPC());
		}
	};

	if (pendents & INTERRUPT_VBLANK) {
		hwstate.ClearInt(INTERRUPT_VBLANK);
		push_pc();
		cpu.SetPC(INTERRUPT_VBLANK_ADDR);
		cpu.AddCycles(12);
	}

	if (pendents & INTERRUPT_LCD_STAT) {
		hwstate.ClearInt(INTERRUPT_LCD_STAT);
		push_pc();
		cpu.SetPC(INTERRUPT_LCD_STAT_ADDR);
		cpu.AddCycles(12);
	}

	if (pendents & INTERRUPT_TIMER) {
		hwstate.ClearInt(INTERRUPT_TIMER);
		push_pc();
		cpu.SetPC(INTERRUPT_TIMER_ADDR);
		cpu.AddCycles(12);
	}

	if (pendents & INTERRUPT_SERIAL) {
		hwstate.ClearInt(INTERRUPT_SERIAL);
		push_pc();
		cpu.SetPC(INTERRUPT_SERIAL_ADDR);
		cpu.AddCycles(12);
	}

	if (pendents & INTERRUPT_JOYPAD) {
		hwstate.ClearInt(INTERRUPT_JOYPAD);
		push_pc();
		cpu.SetPC(INTERRUPT_JOYPAD_ADDR);
		cpu.AddCycles(12);
	}
}






















}
#include "HWState.hpp"
#include "Gameboy.hpp"

namespace gbx {

void Gameboy::UpdateTimers(const uint8_t cycles)
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
				hwstate.RequestInt(IntTimer);
			}

			hwstate.tima_clock -= hwstate.tima_clock_limit;
		}
	}
}


void Gameboy::UpdateInterrupts()
{
	const uint8_t pendents = hwstate.GetPendentInts();

	if (pendents && hwstate.GetFlags(HWState::CPU_HALT))
		hwstate.ClearFlags(HWState::CPU_HALT);

	if (!hwstate.GetIntMaster()) {
		return;
	} else if (!hwstate.GetIntActive()) {
		hwstate.EnableIntActive();
		return;
	}

	if (!pendents)
		return;

	hwstate.DisableIntMaster();

	const auto trigger = [this](const IntMask inter, const uint16_t addr) {
		hwstate.ClearInt(inter);
		PushStack16(cpu.pc);
		cpu.pc = addr;
		cpu.clock += 12;
	};

	if (pendents & IntVBlank)
		trigger(IntVBlank, 0x40);

	if (pendents & IntLcdStat)
		trigger(IntLcdStat, 0x48);

	if (pendents & IntTimer)
		trigger(IntTimer, 0x50);

	if (pendents & IntSerial)
		trigger(IntSerial, 0x58);

	if (pendents & IntJoypad)
		trigger(IntJoypad, 0x60);
}



} // namespace gbx

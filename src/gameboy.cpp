#include <string.h>
#include "instructions.hpp"
#include "gameboy.hpp"

namespace gbx {

extern void update_gpu(int16_t cycles, const Memory& mem, HWState* hwstate, Gpu* gpu);
static void update_apu(int16_t cycles, HWState* hwstate, Apu* apu);
static void update_timers(int16_t cycles, HWState* hwstate);
static void update_interrupts(Gameboy* gb);

void reset(Gameboy* const gb)
{
	memset(gb, 0, sizeof(*gb));

	// init the system
	// up to now only Gameboy (DMG) mode is supported
	gb->cpu.pc = 0x0100;
	gb->cpu.sp = 0xFFFE;
	gb->cpu.af = 0x01B0;
	gb->cpu.bc = 0x0013;
	gb->cpu.de = 0x00D8;
	gb->cpu.hl = 0x014D;

	gb->gpu.lcdc.value = 0x91;
	gb->gpu.stat.value = 0x85;
	write_pallete(0xFC, &gb->gpu.bgp);
	write_pallete(0xFF, &gb->gpu.obp0);
	write_pallete(0xFF, &gb->gpu.obp1);

	gb->apu.ch1.nr10.value = 0x80;
	write_nr11(0xBF, &gb->apu);
	gb->apu.ch1.nr12.value = 0xF3;
	gb->apu.ch1.nr14.value = 0xBF;
	write_nr21(0x3F, &gb->apu);
	gb->apu.ch2.nr24.value = 0xBF;
	gb->apu.ch3.nr30.value = 0x7F;
	write_nr31(0xFF, &gb->apu);
	gb->apu.ch3.nr32.value = 0x9F;
	gb->apu.ch3.nr33.value = 0xBF;
	write_nr41(0xFF, &gb->apu);
	gb->apu.ch4.nr44.value = 0xBF;
	gb->apu.ctl.nr50.value = 0x77;
	gb->apu.ctl.nr51.value = 0xF3;
	gb->apu.ctl.nr52.value = 0xF1;

	gb->hwstate.tac = 0xF8;

	gb->joypad.reg.value = 0xFF;
	gb->joypad.keys.both = 0xFF;
}


void run_for(const int32_t clock_limit, Gameboy* const gb)
{
	do {
		const auto before = gb->cpu.clock;
		update_interrupts(gb);

		if (!gb->hwstate.flags.cpu_halt) {
			const uint8_t opcode = mem_read8(*gb, gb->cpu.pc++);
			main_instructions[opcode](gb);
			gb->cpu.clock += clock_table[opcode];
		} else {
			gb->cpu.clock += 4;
		}

		const auto step_cycles =
		  static_cast<int16_t>(gb->cpu.clock - before);

		update_gpu(step_cycles, gb->memory, &gb->hwstate, &gb->gpu);
		update_apu(step_cycles, &gb->hwstate, &gb->apu);
		update_timers(step_cycles, &gb->hwstate);
	} while (gb->cpu.clock < clock_limit ||
	         (gb->gpu.ly > 0 && gb->gpu.ly < 144));

	gb->cpu.clock -= clock_limit;
}


void update_timers(const int16_t cycles, HWState* const hwstate)
{
	hwstate->div_clock += cycles;
	if (hwstate->div_clock >= 256) {
		++hwstate->div;
		hwstate->div_clock -= 256;
	}

	const auto tac = hwstate->tac;
	if (test_bit(2, tac)) {
		hwstate->tima_clock += cycles;
		while (hwstate->tima_clock >= hwstate->tima_clock_limit) {
			inc_tima(hwstate);
			hwstate->tima_clock -= hwstate->tima_clock_limit;
			hwstate->tima_clock_limit = kTimaClockLimits[tac&3];
		}
	}
}


void update_apu(const int16_t cycles, HWState* const /*hwstate*/, Apu* const apu)
{
	if (apu->ctl.nr52.master == 0)
		return;

	constexpr const int16_t timer_limit = 16384;

	const auto steptimer = 
	[timer_limit, cycles] (const int length, int16_t* const timer) {
		if (length != 0 && (*timer += cycles) >= timer_limit) {
			*timer -= timer_limit;
			return true;
		}
		return false;
	};

	if (steptimer(apu->ch1.nr11.length, &apu->ch1.timer)) {
		if (--apu->ch1.nr11.length == 0)
			apu->ctl.nr52.ch1_on = 0;
	}

	if (steptimer(apu->ch2.nr21.length, &apu->ch2.timer)) {
		if (--apu->ch2.nr21.length == 0)
			apu->ctl.nr52.ch2_on = 0;
	}

	if (steptimer(apu->ch3.nr31.length, &apu->ch3.timer)) {
		if (--apu->ch3.nr31.length == 0)
			apu->ctl.nr52.ch3_on = 0;
	}

	if (steptimer(apu->ch4.nr41.length, &apu->ch4.timer)) {
		if (--apu->ch4.nr41.length == 0)
			apu->ctl.nr52.ch4_on = 0;
	}
}

void update_interrupts(Gameboy* const gb)
{
	const uint8_t pendents = get_pendent_interrupts(gb->hwstate);
	const auto flags = gb->hwstate.flags;
	if (pendents && flags.cpu_halt) {
		gb->hwstate.flags.cpu_halt = false;
		gb->cpu.clock += 4;
	}

	if (flags.ime == 0) {
		return;
	} else if (flags.ime == 1) {
		gb->hwstate.flags.ime = 2;
		return;
	} else if (pendents == 0) {
		return;
	}

	gb->hwstate.flags.ime = 0;
	for (const auto interrupt : kInterrupts) {
		if (pendents & interrupt.mask) {
			clear_interrupt(interrupt, &gb->hwstate);
			stack_push16(gb->cpu.pc, gb);
			gb->cpu.pc = interrupt.addr;
			gb->cpu.clock += 20;
			break;
		}
	}
}





} // namespace gbx


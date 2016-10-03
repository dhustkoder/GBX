#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Instructions.hpp"
#include "Cartridge.hpp"
#include "Gameboy.hpp"

namespace gbx {

extern void update_gpu(uint8_t cycles, const Memory& mem, HWState* hwstate, Gpu* gpu);
static void update_timers(uint8_t cycles, HWState* hwstate);
static void update_interrupts(Gameboy* gb);
static uint8_t step_machine(Gameboy* gb);


owner<Gameboy*> create_gameboy()
{
	const auto gb = malloc(sizeof(Gameboy));
	if (gb != nullptr) {
		memset(gb, 0, sizeof(Gameboy));
		return static_cast<Gameboy*>(gb);
	} else {
		perror("Couldn't allocate memory");
		return nullptr;
	}
}


void destroy_gameboy(const owner<Gameboy*> gb)
{
	assert(gb != nullptr);
	free(gb);
}


bool Gameboy::Reset()
{
	assert(Cartridge::info.loaded);
	const auto& cart_info = Cartridge::info;

	printf("Cartridge information\n"
	       "internal name: %s\n"
	       "internal size: %zu\n"
	       "type code: %u\n"
	       "system code: %u\n",
	       cart_info.internal_name, cart_info.size, 
	       static_cast<unsigned>(cart_info.type), 
	       static_cast<unsigned>(cart_info.system));
	/*
	if (cart_info.system != Cartridge::System::Gameboy 
	     || cart_info.type != Cartridge::Type::RomOnly) {
		fprintf(stderr, "cartridge system not supported!\n");
		return false;
	}
	*/

	// init the system, Gameboy mode
	cpu.pc = 0x0100;
	cpu.sp = 0xFFFE;
	cpu.af = 0x01B0;
	cpu.bc = 0x0013;
	cpu.de = 0x00D8;
	cpu.hl = 0x014D;

	gpu.lcdc.value = 0x91;
	gpu.stat.value = 0x85;
	gpu.bgp = 0xFC;
	gpu.obp0 = 0xFF;
	gpu.obp1 = 0xFF;

	hwstate.tima_clock_limit = 0x400;
	keys.value = 0xCF;
	keys.pad.value = 0xFF;

	// inital values for hardware registers
	// Write8(0xFF05, 0x00); TIMA, in HWState
	// Write8(0xFF06, 0x00); TMA, in HWState
	// Write8(0xFF07, 0x00); TAC, in HWState
	// Write8(0xFF10, 0x80); NR10
	// Write8(0xFF11, 0xBF); NR11
	// Write8(0xFF12, 0xF3); NR12
	// Write8(0xFF14, 0xBF); NR14
	// Write8(0xFF16, 0x3F); NR21
	// Write8(0xFF17, 0x00); NR22
	// Write8(0xFF19, 0xBF); NR24
	// Write8(0xFF1A, 0x7F); NR30
	// Write8(0xFF1B, 0xFF); NR31
	// Write8(0xFF1C, 0x9F); NR32
	// Write8(0xFF1E, 0xBF); NR33
	// Write8(0xFF20, 0xFF); NR41
	// Write8(0xFF21, 0x00); NR42
	// Write8(0xFF22, 0x00); NR43
	// Write8(0xFF23, 0xBF); NR30
	// Write8(0xFF24, 0x77); NR50
	// Write8(0xFF25, 0xF3); NR51
	// Write8(0xFF26, 0xF1); NR52
	// Write8(0xFF40, 0x91); LCDC, in GPU
	// Write8(0xFF42, 0x00); SCY, in GPU
	// Write8(0xFF43, 0x00); SCX, in GPU
	// Write8(0xFF45, 0x00); LYC, in GPU
	// Write8(0xFF47, 0xFC); BGP, in GPU
	// Write8(0xFF48, 0xFF); OBP0, in GPU
	// Write8(0xFF49, 0xFF); OBP1, in GPU
	// Write8(0xFF4A, 0x00); WY, in GPU
	// Write8(0xFF4B, 0x00); WX, in GPU

	return true;
}


void Gameboy::Run(const uint32_t cycles)
{
	assert(Cartridge::info.loaded);

	do {
		const uint8_t step_cycles = step_machine(this);
		cpu.clock += step_cycles;
		update_gpu(step_cycles, memory, &hwstate, &gpu);
		update_timers(step_cycles, &hwstate);
		update_interrupts(this);
	} while (cpu.clock < cycles);

	cpu.clock = 0;
}


uint8_t step_machine(Gameboy* const gb)
{
	if (!gb->hwstate.GetFlags(HWState::CpuHalt)) {
		const uint8_t opcode = gb->Read8(gb->cpu.pc++);
		main_instructions[opcode](gb);
		return clock_table[opcode];
	}
	
	return 4;
}


void update_timers(const uint8_t cycles, HWState* const hwstate)
{
	hwstate->div_clock += cycles;

	if (hwstate->div_clock >= 0x100) {
		++hwstate->div;
		hwstate->div_clock = 0;
	}

	if (!hwstate->GetFlags(HWState::TimerStop)) {
		
		hwstate->tima_clock += cycles;
		
		if (hwstate->tima_clock >= hwstate->tima_clock_limit) {
			
			if (hwstate->tima < 0xff) {
				++hwstate->tima;
			} else {
				hwstate->tima = hwstate->tma;
				hwstate->RequestInt(Interrupts::timer);
			}

			hwstate->tima_clock -= hwstate->tima_clock_limit;
		}
	}
}


void update_interrupts(Gameboy* const gb)
{
	auto& hwstate = gb->hwstate;
	const uint8_t pendents = hwstate.GetPendentInts();

	if (pendents && hwstate.GetFlags(HWState::CpuHalt))
		hwstate.ClearFlags(HWState::CpuHalt);

	if (!hwstate.GetIntMaster()) {
		return;
	} else if (!hwstate.GetIntActive()) {
		hwstate.EnableIntActive();
		return;
	}

	if (!pendents)
		return;

	hwstate.DisableIntMaster();
	
	for (const auto inter : Interrupts::array) {
		if (pendents & inter.mask) {
			hwstate.ClearInt(inter);
			gb->PushStack16(gb->cpu.pc);
			gb->cpu.pc = inter.addr;
			gb->cpu.clock += 12;
		}
	}
}





} // namespace gbx


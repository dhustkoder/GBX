#include <stdio.h>
#include <string.h>
#include <Utix/Malloc.h>
#include "Instructions.hpp"
#include "Cartridge.hpp"
#include "Gameboy.hpp"

namespace gbx {



Gameboy* create_gameboy() 
{
	Gameboy* const gb = utix::malloc_t<Gameboy>();
	if (!gb) {
		perror("Couldn't allocate memory");
		return nullptr;
	}
	memset(gb, 0, sizeof(Gameboy));
	return gb;
}

void destroy_gameboy(Gameboy* const gb) 
{
	free(gb);
}



bool Gameboy::Reset() 
{
	const auto& cart_info = memory.cart.info;

	printf("Cartridge information\n"
	       "internal name: %s\n"
	       "internal size: %zu\n"
	       "type code: %u\n"
	       "system code: %u\n",
	       cart_info.internal_name, cart_info.size, 
	       static_cast<unsigned>(cart_info.type), 
	       static_cast<unsigned>(cart_info.system));

	/*
	if (cart_info.system != System::GAMEBOY) {
		fprintf(stderr, "cartridge system not supported!");
		return false;
	} else if (cart_info.type != CartridgeType::ROM_ONLY) {
		fprintf(stderr, "cartridge type not suppoerted!");
		return false;
	}
	*/

	// init the system, Gameboy mode
	cpu.pc = 0x0100;
	cpu.sp = 0xFFFE;
	cpu.af.pair = 0x01B0;
	cpu.bc.pair = 0x0013;
	cpu.de.pair = 0x00D8;
	cpu.hl.pair = 0x014D;

	gpu.lcdc = 0x91;
	gpu.stat = 0x85;
	gpu.bgp = 0xFC;
	gpu.obp0 = 0xFF;
	gpu.obp1 = 0xFF;

	hwstate.tima_clock_limit = 0x400;
	keys.value = 0xCF;
	keys.pad.all = 0xFF;

	// Write8(0xFF05, 0x00); // TIMA, in HWState
	// Write8(0xFF06, 0x00); // TMA, in HWState
	// Write8(0xFF07, 0x00); // TAC, in HWState
	Write8(0xFF10, 0x80); // NR10
	Write8(0xFF11, 0xBF); // NR11
	Write8(0xFF12, 0xF3); // NR12
	Write8(0xFF14, 0xBF); // NR14
	Write8(0xFF16, 0x3F); // NR21
	Write8(0xFF17, 0x00); // NR22
	Write8(0xFF19, 0xBF); // NR24
	Write8(0xFF1A, 0x7F); // NR30
	Write8(0xFF1B, 0xFF); // NR31
	Write8(0xFF1C, 0x9F); // NR32
	Write8(0xFF1E, 0xBF); // NR33
	Write8(0xFF20, 0xFF); // NR41
	Write8(0xFF21, 0x00); // NR42
	Write8(0xFF22, 0x00); // NR43
	Write8(0xFF23, 0xBF); // NR30
	Write8(0xFF24, 0x77); // NR50
	Write8(0xFF25, 0xF3); // NR51
	Write8(0xFF26, 0xF1); // NR52
	// Write8(0xFF40, 0x91); // LCDC, in GPU
	// Write8(0xFF42, 0x00); // SCY, in GPU
	// Write8(0xFF43, 0x00); // SCX, in GPU
	// Write8(0xFF45, 0x00); // LYC, in GPU
	// Write8(0xFF47, 0xFC); // BGP, in GPU
	// Write8(0xFF48, 0xFF); // OBP0, in GPU
	// Write8(0xFF49, 0xFF); // OBP1, in GPU
	// Write8(0xFF4A, 0x00); // WY, in GPU
	// Write8(0xFF4B, 0x00); // WX, in GPU

	return true;
}





uint8_t Gameboy::Step()
{
	if (!hwstate.GetFlags(HWState::CPU_HALT)) {
		const uint8_t opcode = Read8(cpu.pc++);
		main_instructions[opcode](this);
		const uint8_t cycles = clock_table[opcode];
		cpu.clock += cycles;
		return cycles;
	} else {
		cpu.clock += 4;
		return 4;
	}
}




void Gameboy::Run(const uint32_t cycles)
{
	do {
		const uint8_t step_cycles = Step();
		UpdateGPU(step_cycles);
		UpdateHWState(step_cycles);
		UpdateInterrupts();
	} while (cpu.clock < cycles);
	cpu.clock = 0;
}













}


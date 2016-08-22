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
	// load cartridge data into memory
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
	} 
	else if (cart_info.type != CartridgeType::ROM_ONLY) {
		fprintf(stderr, "cartridge type not suppoerted!");
		return false;
	}
	*/

	// init the system, Gameboy mode
	cpu.pc = CARTRIDGE_ENTRY_ADDR;
	cpu.sp = 0xFFFE;
	cpu.af.pair = 0x01B0;
	cpu.bc.pair = 0x0013;
	cpu.de.pair = 0x00D8;
	cpu.hl.pair = 0x014D;

	gpu.lcdc = 0x91;
	gpu.stat = 0x85;
	gpu.bgp = 0xfc;
	gpu.obp0 = 0xff;
	gpu.obp1 = 0xff;

	hwstate.tima_clock_limit = 0x400;
	keys.value = 0xcf;
	keys.pad.all = 0xff;

	// WriteU8(0xFF05, 0x00); // TIMA, in HWState
	// WriteU8(0xFF06, 0x00); // TMA, in HWState
	// WriteU8(0xFF07, 0x00); // TAC, in HWState
	WriteU8(0xFF10, 0x80); // NR10
	WriteU8(0xFF11, 0xBF); // NR11
	WriteU8(0xFF12, 0xF3); // NR12
	WriteU8(0xFF14, 0xBF); // NR14
	WriteU8(0xFF16, 0x3F); // NR21
	WriteU8(0xFF17, 0x00); // NR22
	WriteU8(0xFF19, 0xBF); // NR24
	WriteU8(0xFF1A, 0x7F); // NR30
	WriteU8(0xFF1B, 0xFF); // NR31
	WriteU8(0xFF1C, 0x9F); // NR32
	WriteU8(0xFF1E, 0xBF); // NR33
	WriteU8(0xFF20, 0xFF); // NR41
	WriteU8(0xFF21, 0x00); // NR42
	WriteU8(0xFF22, 0x00); // NR43
	WriteU8(0xFF23, 0xBF); // NR30
	WriteU8(0xFF24, 0x77); // NR50
	WriteU8(0xFF25, 0xF3); // NR51
	WriteU8(0xFF26, 0xF1); // NR52
	// WriteU8(0xFF40, 0x91); // LCDC, in GPU
	// WriteU8(0xFF42, 0x00); // SCY, in GPU
	// WriteU8(0xFF43, 0x00); // SCX, in GPU
	// WriteU8(0xFF45, 0x00); // LYC, in GPU
	// WriteU8(0xFF47, 0xFC); // BGP, in GPU
	// WriteU8(0xFF48, 0xFF); // OBP0, in GPU
	// WriteU8(0xFF49, 0xFF); // OBP1, in GPU
	// WriteU8(0xFF4A, 0x00); // WY, in GPU
	// WriteU8(0xFF4B, 0x00); // WX, in GPU

	return true;
}





uint8_t Gameboy::Step()
{
	const uint8_t opcode = ReadU8(cpu.pc++);
	main_instructions[opcode](this);
	const uint8_t cycles = clock_table[opcode];
	cpu.clock += cycles;
	return cycles;
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


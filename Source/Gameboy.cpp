#include <stdio.h>
#include <string.h>
#include <Utix/Alloc_t.h>
#include "Debug.hpp"
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

	return gb;
}

void destroy_gameboy(Gameboy* const gb) 
{
	free(gb);
}








bool Gameboy::Reset() 
{
	// load cartridge data into memory
	const auto cartridge_info = get_cartridge_info(memory);

	printf("Cartridge information\n"
	       "internal name: %s\n"
	       "internal size: %zu\n"
	       "type code: %u\n"
	       "system code: %u\n",
	       cartridge_info.internal_name, cartridge_info.size, 
	       static_cast<unsigned>(cartridge_info.type), 
	       static_cast<unsigned>(cartridge_info.system));


	if (cartridge_info.system != System::GAMEBOY) {
		fprintf(stderr, "cartridge system not supported!");
		return false;
	} 
	else if (cartridge_info.type != CartridgeType::ROM_ONLY) {
		fprintf(stderr, "cartridge type not suppoerted!");
		return false;
	}

	// init the system, Gameboy mode
	cpu.SetPC(CARTRIDGE_ENTRY_ADDR);
	cpu.SetSP(0xfffe);
	cpu.SetAF(0x01B0);
	cpu.SetBC(0x0013);
	cpu.SetDE(0x00D8);
	cpu.SetHL(0x014D);
	cpu.SetClock(0);

	gpu.clock = 0;
	gpu.control = 0x91;
	gpu.status = 0x00;
	gpu.scanline = 0x00;

	keys.value = 0xCF;

	hwstate.hwflags = 0x00;
	hwstate.interrupt_enable = 0x00;
	hwstate.interrupt_flags = 0x00;
	

	WriteU8(0xFF05, 0x00); // TIMA
	WriteU8(0xFF06, 0x00); // TMA
	WriteU8(0xFF07, 0x00); // TAC
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
	WriteU8(0xFF42, 0x00); // SCY
	WriteU8(0xFF43, 0x00); // SCX
	WriteU8(0xFF45, 0x00); // LYC
	WriteU8(0xFF47, 0xFC); // BGP
	WriteU8(0xFF48, 0xFF); // OBP0
	WriteU8(0xFF49, 0xFF); // OBP1
	WriteU8(0xFF4A, 0x00); // WY
	WriteU8(0xFF4B, 0x00); // WX

	return true;
}






void Gameboy::Step()
{
	const uint16_t pc = cpu.GetPC();
	const uint8_t opcode = ReadU8(pc);
	cpu.AddPC(1);

	debug_printf("PC: %4x | OP: %4x\n", pc, opcode);

	main_table[opcode](this);
	const uint8_t cycles = clock_table[opcode];
	cpu.AddCycles(cycles);
	gpu.clock += cycles;
}





void Gameboy::UpdateGPU()
{
	if (!(gpu.control & GPU::LCD_ON_OFF)) {
		gpu.clock = 0;
		gpu.scanline = 0;
		gpu.SetMode(GPU::Mode::VBLANK);
		return;
	}

	switch (gpu.GetMode()) {
	case GPU::Mode::HBLANK:
		if (gpu.clock >= 204) {
			++gpu.scanline;

			if (gpu.scanline == 143) {
				hwstate.interrupt_flags |= INTERRUPT_VBLANK;
				gpu.SetMode(GPU::Mode::VBLANK);
			}
			else {
				gpu.SetMode(GPU::Mode::OAM);
			}

			gpu.clock -= 204;
		}

		break;

	case GPU::Mode::VBLANK:
		if (gpu.clock >= 456) {
			++gpu.scanline;

			if (gpu.scanline > 153) {
				gpu.scanline = 0;
				gpu.SetMode(GPU::Mode::OAM);
			}

			gpu.clock -= 456;
		}

		break;

	case GPU::Mode::OAM:
		if (gpu.clock >= 80) {
			gpu.SetMode(GPU::Mode::TRANSFER);
			gpu.clock -= 80;
		}

		break;

	case GPU::Mode::TRANSFER:
		if (gpu.clock >= 172) {
			gpu.SetMode(GPU::Mode::HBLANK);
			gpu.clock -= 172;
		}

		break;
	}
}







void Gameboy::UpdateInterrupts()
{
	if (!(hwstate.hwflags & HWState::INTERRUPT_MASTER_ENABLED)) {
		return;
	} 
	else if(!(hwstate.hwflags & HWState::INTERRUPT_MASTER_ACTIVE)) {
		hwstate.hwflags |= HWState::INTERRUPT_MASTER_ACTIVE;
		return;
	}

	const uint8_t requests = 0x1f & hwstate.interrupt_enable & hwstate.interrupt_flags;

	if(!requests)
		return;

	hwstate.hwflags &= ~HWState::INTERRUPT_MASTER_ENABLED;

	if (requests & INTERRUPT_VBLANK) {
		hwstate.interrupt_flags &= ~INTERRUPT_VBLANK;
		PushStack16(cpu.GetPC());
		cpu.SetPC(INTERRUPT_VBLANK_ADDR);
		cpu.AddCycles(12);
	}

	if (requests & INTERRUPT_LCDC) {
		hwstate.interrupt_flags &= ~INTERRUPT_LCDC;
		PushStack16(cpu.GetPC());
		cpu.SetPC(INTERRUPT_LCDC_ADDR);
		cpu.AddCycles(12);
	}

	if (requests & INTERRUPT_TIMER) {
		hwstate.interrupt_flags &= ~INTERRUPT_TIMER;
		PushStack16(cpu.GetPC());
		cpu.SetPC(INTERRUPT_TIMER_ADDR);
		cpu.AddCycles(12);
	}

	if (requests & INTERRUPT_SERIAL) {
		hwstate.interrupt_flags &= ~INTERRUPT_SERIAL;
		PushStack16(cpu.GetPC());
		cpu.SetPC(INTERRUPT_SERIAL_ADDR);
		cpu.AddCycles(12);
	}

	if (requests & INTERRUPT_JOYPAD) {
		hwstate.interrupt_flags &= ~INTERRUPT_JOYPAD;
		PushStack16(cpu.GetPC());
		cpu.SetPC(INTERRUPT_JOYPAD_ADDR);
		cpu.AddCycles(12);
	}

}


















}


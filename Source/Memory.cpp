#include <stdio.h>
#include <Utix/Assert.h>
#include "Gameboy.hpp"
#include "Memory.hpp"

namespace gbx {
static const uint8_t* solve_address(const uint16_t address, const Gameboy& gb);
static const uint8_t* solve_hardware_io_address(const uint16_t address, const Gameboy& gb);




// TODO: this might not be totally portable 
int8_t Gameboy::ReadS8(const uint16_t address) const {
	return static_cast<int8_t>(ReadU8(address));
}




uint8_t Gameboy::ReadU8(const uint16_t address) const {
	const uint8_t* const addr = solve_address(address, *this);
	return addr ? *addr : 0;
}



uint16_t Gameboy::ReadU16(const uint16_t address) const {
	return ConcatBytes(ReadU8(address + 1), ReadU8(address));
}





// TODO: ready only hardware addresses must be checked before calling solve_address
void Gameboy::WriteU8(const uint16_t address, const uint8_t value) 
{
	if (address > 0x7fff) {
		uint8_t* const addr = const_cast<uint8_t*>(solve_address(address, *this));
		if (addr)
			*addr = value;
	} else {
		// attempt to write to ROM
		fprintf(stderr, "ROM write attempt at: %4x\n", address);
	}
}



void Gameboy::WriteU16(const uint16_t address, const uint16_t value) 
{
	WriteU8(address, GetLowByte(value));
	WriteU8(address + 1, GetHighByte(value));
}





void Gameboy::PushStack8(const uint8_t value) 
{
	const uint16_t sp = cpu.GetSP() - 1;
	WriteU8(sp, value);
	cpu.SetSP(sp);
}


void Gameboy::PushStack16(const uint16_t value) 
{
	const uint16_t sp = cpu.GetSP() - 2;
	WriteU16(sp, value);
	cpu.SetSP(sp);
}



uint8_t Gameboy::PopStack8() 
{
	const uint16_t sp = cpu.GetSP();
	const uint8_t val = ReadU8(sp);
	cpu.SetSP(sp + 1);
	return val;
}




uint16_t Gameboy::PopStack16() 
{
	const uint16_t sp = cpu.GetSP();
	const uint16_t val = ReadU16(sp);
	cpu.SetSP(sp + 2);
	return val;
}








static const uint8_t* solve_address(const uint16_t address, const Gameboy& gb)
{
	if (address >= 0xFF80) {
		return address < 0xFFFF ? &gb.memory.zero_page[address - 0xFF80] 
		                        : &gb.hwstate.interrupt_enable;
	}
	else if (address >= 0xFF00) {
		return solve_hardware_io_address(address, gb);
	}
	else if (address >= 0xFE00) {
		if (address < 0xFEA0)
			return &gb.memory.oam[address - 0xFE00];
	}
	else if (address >= 0xC000) {
		if (address < 0xE000)
			return &gb.memory.ram[address - 0xC000];
	}
	else if (address >= 0xA000) {
		ASSERT_MSG(false, "cartridge ram required");
	}
	else if (address >= 0x8000) {
		return &gb.memory.vram[address - 0x8000];
	}
	else if (address < 0x8000) {
		return &gb.memory.home[address];
	}

	fprintf(stderr, "required address: %4x\n", address);
	return nullptr;
}



static const uint8_t* solve_hardware_io_address(const uint16_t address, const Gameboy& gb)
{
	switch (address) {
	case 0xFF0F: return &gb.hwstate.interrupt_flags;
	default:
		fprintf(stderr, "required hardware io address: %4x\n", address);
		break;
	};

	return nullptr;
}









}

#include <stdio.h>
#include <stdlib.h>
#include <Utix/Assert.h>
#include "Debug.hpp"
#include "Gameboy.hpp"
#include "Memory.hpp"

namespace gbx {

static void dma_transfer(const uint8_t value, Gameboy* const gb);









// TODO: this might not be totally portable 
int8_t Gameboy::ReadS8(const uint16_t address) const 
{
	return static_cast<int8_t>(ReadU8(address));
}




uint8_t Gameboy::ReadU8(const uint16_t address) const 
{
	if (address < 0x8000) {
		return memory.home[address];
	}
	else if (address >= 0xFF80) {
		return address != 0xFFFF ? memory.hram[address - 0xFF80]
		                         : hwstate.interrupt_enable;
	}
	else if (address >= 0xFF00) {
		switch (address) {
		case 0xFF00: return keys.value;
		case 0xFF0F: return hwstate.interrupt_flags;
		case 0xFF40: return gpu.control;
		case 0xFF41: return gpu.status;
		case 0xFF42: return gpu.scy;
		case 0xFF43: return gpu.scx;
		case 0xFF44: return gpu.ly;
		case 0xFF45: return gpu.lyc;
		case 0xFF47: return gpu.bgp;
		case 0xFF4A: return gpu.wy;
		case 0xFF4B: return gpu.wx;
		default:
			debug_printf("required hardware io address: %4x\n", address);
			break;
		};
	}
	else if (address >= 0xFE00) {
		if (address < 0xFEA0)
			return memory.oam[address - 0xFE00];
	}
	else if (address >= 0xC000) {
		if (address < 0xE000)
			return memory.wram[address - 0xC000];
	}
	else if (address >= 0xA000) {
		ASSERT_MSG(false, "cartridge ram required");
	}
	else {
		return memory.vram[address - 0x8000];
	}


	return 0;
}




void Gameboy::WriteU8(const uint16_t address, const uint8_t value) 
{

	if (address >= 0xFF80) {
		if (address != 0xFFFF) 
			memory.hram[address - 0xFF80] = value;
		else 
			hwstate.interrupt_enable = value;
	}
	else if (address >= 0xFF00) {
		switch (address) {
		case 0xFF00:
			if ((value & 0x30) == 0x10)
				keys.value = 0xD0 | (keys.buttons >> 4);
			else if ((value & 0x30) == 0x20)
				keys.value = 0xE0 | (keys.buttons & 0x0f);
			else
				keys.value = 0xFF;
			break;
		case 0xFF0F:
			hwstate.interrupt_flags = value; 
			break;
		case 0xFF40:
			gpu.control = value; 
			break;
		case 0xFF41:
			gpu.status = (value & 0xF8) | (gpu.status & 0x07);
			break;
		case 0xFF42:
			gpu.scy = value;
			break;
		case 0xFF43:
			gpu.scx = value;
			break;
		case 0xFF44: 
			gpu.ly = 0;
			break;
		case 0xFF45:
			gpu.lyc = value;
			break;
		case 0xFF46:
			dma_transfer(value, this);
			break;
		case 0xFF47:
			gpu.bgp = value;
			break;
		case 0xFF4A:
			gpu.wy = value;
			break;
		case 0xFF4B:
			gpu.wx = value;
			break;
		default:
			debug_printf("required hardware io address: %4x\n", address);
			break;
		};
	}
	else if (address >= 0xFE00) {
		if (address < 0xFEA0) {
			memory.oam[address - 0xFE00] = value;
		}
	}
	else if (address >= 0xC000) {
		if (address < 0xE000)
			memory.wram[address - 0xC000] = value;
	}
	else if (address >= 0xA000) {
		ASSERT_MSG(false, "cartridge ram required");
	}
	else if (address >= 0x8000) {
		memory.vram[address - 0x8000] = value;
	}
	else {
		debug_printf("required hardware io address: %4x\n", address);
	}

}







uint16_t Gameboy::ReadU16(const uint16_t address) const 
{
	return ConcatBytes(ReadU8(address + 1), ReadU8(address));
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
















static void dma_transfer(const uint8_t value, Gameboy* const gb)
{
	uint16_t source_addrs = value * 0x100;
	for (size_t i = 0; i < 0xA0; ++i, ++source_addrs)
		gb->memory.oam[i] = gb->ReadU8(source_addrs);
}








}

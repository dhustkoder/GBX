#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Utix/Assert.h>
#include "Debug.hpp"
#include "Gameboy.hpp"

namespace gbx {


static uint8_t read_io(const uint16_t address, const Gameboy& gb);
static void write_io(const uint16_t address, const uint8_t value, Gameboy* const gb);
static uint8_t read_rom(const uint16_t address, const Cartridge& cart);
static void write_rom(const uint16_t address, const uint8_t value, Cartridge* const);
static void write_keys(const uint8_t value, Keys* const keys);
static void write_tac(const uint8_t value, HWState* const hwstate);
static void dma_transfer(const uint8_t value, Gameboy* const gb);






// TODO: this might not be totally portable 
int8_t Gameboy::ReadS8(const uint16_t address) const 
{
	return static_cast<int8_t>(ReadU8(address));
}




uint8_t Gameboy::ReadU8(const uint16_t address) const 
{
	if (address < 0x8000) {
		return read_rom(address, memory.cart);
	}
	else if (address >= 0xFF80) {
		return address != 0xFFFF ? memory.hram[address - 0xFF80]
		                         : hwstate.interrupt_enable;
	}
	else if (address >= 0xFF00) {
		return read_io(address, *this);
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
		write_io(address, value, this);
	}
	else if (address >= 0xFE00) {
		if (address < 0xFEA0)
			memory.oam[address - 0xFE00] = value;
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
		write_rom(address, value, &memory.cart);
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






















static uint8_t read_io(const uint16_t address, const Gameboy& gb)
{
	switch (address) {
	case 0xFF00: return gb.keys.value;
	case 0xFF04: return gb.hwstate.div;
	case 0xFF05: return gb.hwstate.tima;
	case 0xFF06: return gb.hwstate.tma;
	case 0xFF07: return gb.hwstate.tac;
	case 0xFF0F: return gb.hwstate.interrupt_flags;
	case 0xFF40: return gb.gpu.lcdc;
	case 0xFF41: return gb.gpu.stat;
	case 0xFF42: return gb.gpu.scy;
	case 0xFF43: return gb.gpu.scx;
	case 0xFF44: return gb.gpu.ly;
	case 0xFF45: return gb.gpu.lyc;
	case 0xFF47: return gb.gpu.bgp;
	case 0xFF48: return gb.gpu.obp0;
	case 0xFF49: return gb.gpu.obp1;
	case 0xFF4A: return gb.gpu.wy;
	case 0xFF4B: return gb.gpu.wx;
	default:
		     debug_printf("required hardware io address: %4x\n", address);
		     break;
	};

	return 0;
}


static void write_io(const uint16_t address, const uint8_t value, Gameboy* const gb)
{
	switch (address) {
	case 0xFF00:
		write_keys(value, &gb->keys);
		break;
	case 0xFF04: 
		gb->hwstate.div = 0x00;
		break;
	case 0xFF05:
		gb->hwstate.tima = value;
		break;
	case 0xFF06:
		gb->hwstate.tma = value;
		break;
	case 0xFF07:
		write_tac(value, &gb->hwstate);
		break;
	case 0xFF0F:
		gb->hwstate.interrupt_flags = value; 
		break;
	case 0xFF40:
		gb->gpu.lcdc = value;
		break;
	case 0xFF41:
		gb->gpu.stat = (value & 0xF8) | (gb->gpu.stat & 0x07);
		break;
	case 0xFF42:
		gb->gpu.scy = value;
		break;
	case 0xFF43:
		gb->gpu.scx = value;
		break;
	case 0xFF44: 
		gb->gpu.ly = 0;
		break;
	case 0xFF45:
		gb->gpu.lyc = value;
		break;
	case 0xFF46:
		dma_transfer(value, gb);
		break;
	case 0xFF47:
		gb->gpu.bgp = value;
		break;
	case 0xFF48:
		gb->gpu.obp0 = value;
		break;
	case 0xFF49:
		gb->gpu.obp1 = value;
		break;
	case 0xFF4A:
		gb->gpu.wy = value;
		break;
	case 0xFF4B:
		gb->gpu.wx = value;
		break;
	default:
		debug_printf("required hardware io address: %4x\n", address);
		break;
	};
}





static uint8_t read_rom(const uint16_t address, const Cartridge& cart)
{
	return cart.rom_banks[address];
}



static void write_rom(const uint16_t address, const uint8_t value, Cartridge* const)
{
	debug_printf("cartridge write value $%2x at $%4x\n", value, address);
}





static void write_keys(const uint8_t value, Keys* const keys)
{
	const uint8_t mask = ( value & 0x30 );

	if (mask == 0x10)
		keys->value = 0xD0 | (keys->pad.all >> 4);
	else if (mask == 0x20)
		keys->value = 0xE0 | (keys->pad.all & 0x0f);
	else
		keys->value = 0xFF;
}


static void write_tac(const uint8_t value, HWState* const hwstate)
{
		hwstate->tac = value;
		const uint8_t hz_code = value & 0x03;
		if (hz_code == 0x00)
			hwstate->tima_clock_limit = 0x400;
		else if (hz_code == 0x01)
			hwstate->tima_clock_limit = 0x10;
		else if (hz_code == 0x02)
			hwstate->tima_clock_limit = 0x40;
		else if (hz_code == 0x03)
			hwstate->tima_clock_limit = 0x100;

		const uint8_t stop_code = value & 0x04;
		if (stop_code == 0x04) {
			if (hwstate->GetFlags(HWState::TIMER_STOP)) {
				hwstate->ClearFlags(HWState::TIMER_STOP);
				hwstate->tima = hwstate->tma;
			}
		}
		else {
			hwstate->SetFlags(HWState::TIMER_STOP);
		}
}



static void dma_transfer(const uint8_t value, Gameboy* const gb)
{
	uint16_t source_addr = value * 0x100;
	const uint8_t nbytes = 0xA0;
	if (source_addr == 0xC000) {
		memcpy(gb->memory.oam, gb->memory.wram, sizeof(uint8_t) * nbytes);
	}
	else if (source_addr == 0x8000) {
		memcpy(gb->memory.oam, gb->memory.vram, sizeof(uint8_t) * nbytes);
	}
	else {
		for (size_t i = 0; i < nbytes; ++i)
			gb->memory.oam[i] = gb->ReadU8(source_addr++);
	}
}




}

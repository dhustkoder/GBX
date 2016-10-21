#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Debug.hpp"
#include "Gameboy.hpp"

namespace gbx {

static uint8_t read_cart(uint16_t address, const Cartridge& cart);
static uint8_t read_hram(uint16_t address, const Gameboy& gb);
static uint8_t read_oam(uint16_t address, const Memory& mem);
static uint8_t read_wram(uint16_t address, const Memory& mem);
static uint8_t read_vram(uint16_t address, const Memory& mem);
static uint8_t read_cart_ram(uint16_t address, const Cartridge& cart);
static uint8_t read_io(uint16_t address, const Gameboy& gb);

static void write_cart(uint16_t address, uint8_t value, Cartridge* cart);
static void write_hram(uint16_t address, uint8_t value, Gameboy* gb);
static void write_oam(uint16_t address, uint8_t value, Memory* mem);
static void write_wram(uint16_t address, uint8_t value, Memory* mem);
static void write_vram(uint16_t address, uint8_t value, Memory* mem);
static void write_cart_ram(uint16_t address, uint8_t value, Cartridge* cart);
static void write_io(uint16_t address, uint8_t value, Gameboy* gb);

static void write_stat(uint8_t value, Gpu* gpu);
static void write_keys(uint8_t value, Keys* keys);
static void write_div(uint8_t value, HWState* hwstate);
static void write_tac(uint8_t value, HWState* hwstate);
static void dma_transfer(uint8_t value, Gameboy* gb);





uint8_t Gameboy::Read8(const uint16_t address) const 
{
	if (address < 0x8000)
		return read_cart(address, cart);
	else if (address >= 0xFF80)
		return read_hram(address, *this);
	else if (address >= 0xFF00)
		return read_io(address, *this);
	else if (address >= 0xFE00)
		return read_oam(address, memory);
	else if (address >= 0xC000)
		return read_wram(address, memory);
	else if (address >= 0xA000)
		return read_cart_ram(address, cart);
	else
		return read_vram(address, memory);
}


void Gameboy::Write8(const uint16_t address, const uint8_t value) 
{
	if (address >= 0xFF80)
		write_hram(address, value, this);
	else if (address >= 0xFF00)
		write_io(address, value, this);
	else if (address >= 0xFE00)
		write_oam(address, value, &memory);
	else if (address >= 0xC000)
		write_wram(address, value, &memory);
	else if (address >= 0xA000)
		write_cart_ram(address, value, &cart);
	else if (address >= 0x8000)
		write_vram(address, value, &memory);
	else
		write_cart(address, value, &cart);
}


uint16_t Gameboy::Read16(const uint16_t address) const 
{
	return concat_bytes(Read8(address + 1), Read8(address));
}


void Gameboy::Write16(const uint16_t address, const uint16_t value) 
{
	Write8(address, get_low_byte(value));
	Write8(address + 1, get_high_byte(value));
}


void Gameboy::PushStack8(const uint8_t value) 
{
	Write8(--cpu.sp, value);
}


void Gameboy::PushStack16(const uint16_t value) 
{
	cpu.sp -= 2;
	Write16(cpu.sp, value);
}


uint8_t Gameboy::PopStack8() 
{
	return Read8(cpu.sp++);
}


uint16_t Gameboy::PopStack16() 
{
	const uint16_t val = Read16(cpu.sp);
	cpu.sp += 2;
	return val;
}





uint8_t read_cart(const uint16_t address, const Cartridge& cart)
{
	if (address >= 0x4000)
		return cart.banks[cart.rom_bank_offset + address];

	return cart.banks[address];
}


uint8_t read_hram(const uint16_t address, const Gameboy& gb)
{
	if (address != 0xFFFF)
		return gb.memory.hram[address - 0xFF80];
	else
		return gb.hwstate.int_enable;
}


uint8_t read_oam(const uint16_t address, const Memory& mem)
{
	return address < 0xFEA0 ? mem.oam[address - 0xFE00] : 0;
}


uint8_t read_wram(const uint16_t address, const Memory& mem)
{
	const uint16_t offset = address < 0xE000 ? address - 0xC000 : address - 0xE000;
	return mem.wram[offset];
}


uint8_t read_vram(const uint16_t address, const Memory& mem)
{
	return mem.vram[address - 0x8000];
}


uint8_t read_cart_ram(const uint16_t address, const Cartridge& cart)
{
	debug_printf("Cartridge RAM read required at $%X\n", address);
	if (cart.ram_bank_offset != 0x00) {
		const auto offset = cart.ram_bank_offset + address;
		return cart.banks[offset];
	}
	return 0x00;
}


void write_cart(const uint16_t address, const uint8_t value, Cartridge* const cart)
{
	debug_printf("Cartridge write request at $%X value $%X\n", address, value);
	if (cart->info.type == Cartridge::Type::RomOnly)
		return;

	const auto eval_banks_offset = [cart] {
		const auto banking_mode = cart->banking_mode;
		const auto rom_bank_num = banking_mode
			? cart->banks_num_lower_bits 
			: cart->banks_num;

		if (rom_bank_num < 0x02)
			cart->rom_bank_offset = 0x00;
		else if (banking_mode || (rom_bank_num != 0x20 && 
		          rom_bank_num != 0x40 && rom_bank_num != 0x60))
			cart->rom_bank_offset = 0x4000 * (rom_bank_num - 1);
		else
			cart->rom_bank_offset = 0x4000 * rom_bank_num;

		if (cart->info.type >= Cartridge::Type::RomMBC1Ram) {
			if (banking_mode && cart->banks_num_upper_bits) {
				cart->ram_bank_offset = 
					(cart->info.rom_size - 0xA000) + 
					0x2000 * cart->banks_num_upper_bits;
			} else {
				cart->ram_bank_offset =
					cart->info.rom_size - 0xA000;
			}
		}
	};

	if (address >= 0x6000) {
		const uint8_t new_mode = value ? 1 : 0;
		if (cart->banking_mode != new_mode) {
			cart->banking_mode = new_mode;
			eval_banks_offset();
		}
	} else if (address >= 0x4000) {
		const uint8_t new_val = value & 0x03;
		if (cart->banks_num_upper_bits != new_val) {
			cart->banks_num_upper_bits = new_val;
			eval_banks_offset();
		}
	} else if (address >= 0x2000) {
		const uint8_t new_val = value & 0x1F;
		if (cart->banks_num_lower_bits != new_val) {
			cart->banks_num_lower_bits = new_val;
			eval_banks_offset();
		}
	}
}


void write_hram(const uint16_t address, const uint8_t value, Gameboy* const gb)
{
	if (address != 0xFFFF)
		gb->memory.hram[address - 0xFF80] = value;
	else
		gb->hwstate.int_enable = value;
}


void write_oam(const uint16_t address, const uint8_t value, Memory* const mem)
{
	if (address < 0xFEA0)
		mem->oam[address - 0xFE00] = value;
}


void write_wram(const uint16_t address, const uint8_t value, Memory* const mem)
{
	const uint16_t offset = address < 0xE000 ? address - 0xC000 : address - 0xE000;
	mem->wram[offset] = value;
}


void write_vram(const uint16_t address, const uint8_t value, Memory* const mem)
{
	mem->vram[address - 0x8000] = value;
}


void write_cart_ram(const uint16_t address, const uint8_t value, Cartridge* const cart)
{
	debug_printf("Cartridge RAM write value $%X required at $%X\n", value, address);
	if (cart->ram_bank_offset != 0) {
		const auto offset = cart->ram_bank_offset + address;
		cart->banks[offset] = value;
	}
}



uint8_t read_io(const uint16_t address, const Gameboy& gb)
{
	switch (address) {
	case 0xFF00: return gb.keys.value;
	case 0xFF04: return gb.hwstate.div;
	case 0xFF05: return gb.hwstate.tima;
	case 0xFF06: return gb.hwstate.tma;
	case 0xFF07: return gb.hwstate.tac;
	case 0xFF0F: return gb.hwstate.int_flags;
	case 0xFF40: return gb.gpu.lcdc.value;
	case 0xFF41: return gb.gpu.stat.value;
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
		debug_printf("required read hardware io address: %4x\n", address);
		break;
	}

	return 0;
}


void write_io(const uint16_t address, const uint8_t value, Gameboy* const gb)
{
	switch (address) {
	case 0xFF00: write_keys(value, &gb->keys); break;
	case 0xFF04: write_div(value, &gb->hwstate); break;
	case 0xFF05: gb->hwstate.tima = value; break;
	case 0xFF06: gb->hwstate.tma = value; break;
	case 0xFF07: write_tac(value, &gb->hwstate); break;
	case 0xFF0F: gb->hwstate.int_flags = value; break;
	case 0xFF40: gb->gpu.lcdc.value = value; break;
	case 0xFF41: write_stat(value, &gb->gpu); break;
	case 0xFF42: gb->gpu.scy = value; break;
	case 0xFF43: gb->gpu.scx = value; break;
	case 0xFF44: gb->gpu.ly = 0x00; break;
	case 0xFF45: gb->gpu.lyc = value; break;
	case 0xFF46: dma_transfer(value, gb); break;
	case 0xFF47: gb->gpu.bgp = value; break;
	case 0xFF48: gb->gpu.obp0 = value; break;
	case 0xFF49: gb->gpu.obp1 = value; break;
	case 0xFF4A: gb->gpu.wy = value; break;
	case 0xFF4B: gb->gpu.wx = value; break;
	default:
		debug_printf("required write hardware io address: %4x\n", address);
		break;
	}
}


void write_stat(const uint8_t value, Gpu* const gpu)
{
	gpu->stat.value = (value & 0xf8) | (gpu->stat.value & 0x07);
}


void write_keys(const uint8_t value, Keys* const keys)
{
	switch (value & 0x30) {
	case 0x10: keys->value = 0xD0 | (keys->pad.value >> 4); break;
	case 0x20: keys->value = 0xE0 | (keys->pad.value & 0x0f); break;
	default: keys->value = 0xFF; break;
	}
}


void write_tac(const uint8_t value, HWState* const hwstate)
{
	constexpr const int16_t limits[] { 0x400, 0x10, 0x40, 0x100 };
	hwstate->tac = 0xF8 | (value&0x07);
	hwstate->tima_clock_limit = limits[value&0x03];
}

void write_div(const uint8_t /*value*/, HWState* const hwstate)
{
	hwstate->div = 0x00;
	hwstate->div_clock = 0x00;
}


void dma_transfer(const uint8_t value, Gameboy* const gb)
{
	constexpr const auto nbytes = sizeof(uint8_t) * 0xA0;
	uint16_t source_addr = value * 0x100;
	if (source_addr == 0xC000) {
		memcpy(gb->memory.oam, gb->memory.wram, nbytes);
	} else if (source_addr == 0x8000) {
		memcpy(gb->memory.oam, gb->memory.vram, nbytes);
	} else {
		for (auto& byte : gb->memory.oam)
			byte = gb->Read8(source_addr++);
	}
	gb->cpu.clock += 0x288;
}






} // namespace gbx


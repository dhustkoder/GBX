#include <string.h>
#include "debug.hpp"
#include "gameboy.hpp"

namespace gbx {

static int_fast32_t eval_cart_rom_offset(const Cart& cart, uint16_t address);
static int_fast32_t eval_cart_ram_offset(const Cart& cart, uint16_t address);
static int_fast32_t eval_hram_offset(uint16_t address);
static int_fast32_t eval_oam_offset(uint16_t address);
static int_fast32_t eval_wram_offset(uint16_t address);
static int_fast32_t eval_vram_offset(uint16_t address);

static uint8_t read_cart(const Cart& cart, uint16_t address);
static uint8_t read_hram(const Gameboy& gb, uint16_t address);
static uint8_t read_oam(const Memory& mem, uint16_t address);
static uint8_t read_wram(const Memory& mem, uint16_t address);
static uint8_t read_vram(const Memory& mem, uint16_t address);
static uint8_t read_cart_ram(const Cart& cart, uint16_t address);
static uint8_t read_io(const Gameboy& gb, uint16_t address);

static void write_cart(uint16_t address, uint8_t value, Cart* cart);
static void write_mbc1(uint16_t address, uint8_t value, Cart* cart);
static void write_mbc2(uint16_t address, uint8_t value, Cart* cart);
static void write_hram(uint16_t address, uint8_t value, Gameboy* gb);
static void write_oam(uint16_t address, uint8_t value, Memory* mem);
static void write_wram(uint16_t address, uint8_t value, Memory* mem);
static void write_vram(uint16_t address, uint8_t value, Memory* mem);
static void write_cart_ram(uint16_t address, uint8_t value, Cart* cart);
static void write_io(uint16_t address, uint8_t value, Gameboy* gb);

static void write_lcdc(uint8_t value, Ppu* ppu, HWState* hwstate);
static void write_stat(uint8_t value, Ppu* ppu);
static void write_joypad(uint8_t value, Joypad* keys);
static void write_div(uint8_t value, HWState* hwstate);
static void write_tac(uint8_t value, HWState* hwstate);
static void dma_transfer(uint8_t value, Gameboy* gb);


uint8_t mem_read8(const Gameboy& gb, const uint16_t address)
{
	if (address < 0x8000)
		return read_cart(gb.cart, address);
	else if (address >= 0xFF80)
		return read_hram(gb, address);
	else if (address >= 0xFF00)
		return read_io(gb, address);
	else if (address >= 0xFE00)
		return read_oam(gb.memory, address);
	else if (address >= 0xC000)
		return read_wram(gb.memory, address);
	else if (address >= 0xA000)
		return read_cart_ram(gb.cart, address);
	else
		return read_vram(gb.memory, address);
}


void mem_write8(const uint16_t address, const uint8_t value, Gameboy* const gb)
{
	if (address >= 0xFF80)
		write_hram(address, value, gb);
	else if (address >= 0xFF00)
		write_io(address, value, gb);
	else if (address >= 0xFE00)
		write_oam(address, value, &gb->memory);
	else if (address >= 0xC000)
		write_wram(address, value, &gb->memory);
	else if (address >= 0xA000)
		write_cart_ram(address, value, &gb->cart);
	else if (address >= 0x8000)
		write_vram(address, value, &gb->memory);
	else
		write_cart(address, value, &gb->cart);
}



uint8_t read_cart(const Cart& cart, const uint16_t address)
{
	const auto offset = eval_cart_rom_offset(cart, address);
	return cart.data[offset];
}


uint8_t read_hram(const Gameboy& gb, const uint16_t address)
{
	if (address != 0xFFFF) {
		const auto offset = eval_hram_offset(address);
		return gb.memory.hram[offset];
	} else {
		return gb.hwstate.int_enable;
	}
}


uint8_t read_oam(const Memory& mem, const uint16_t address)
{
	if (address < 0xFEA0) {
		const auto offset = eval_oam_offset(address);
		return mem.oam[offset];
	}
	return 0xFF;
}


uint8_t read_wram(const Memory& mem, const uint16_t address)
{
	const auto offset = eval_wram_offset(address);
	return mem.wram[offset];
}


uint8_t read_vram(const Memory& mem, const uint16_t address)
{
	const auto offset = eval_vram_offset(address);
	return mem.vram[offset];
}


uint8_t read_cart_ram(const Cart& cart, const uint16_t address)
{
	debug_printf("Cartridge RAM: read from $%X\n", address);
	if (cart.ram_enabled) {
		const auto offset = eval_cart_ram_offset(cart, address);
		return cart.data[offset];
	}
	return 0x00;
}


void write_cart(const uint16_t address, const uint8_t value, Cart* const cart)
{
	debug_printf("Cartridge ROM: write $%X to $%X\n", value, address);
	switch (g_cart_info.short_type()) {
	case CartShortType::RomMBC1: write_mbc1(address, value, cart); break;
	case CartShortType::RomMBC2: write_mbc2(address, value, cart); break;
	default: break;
	}

}

void write_mbc1(const uint16_t address, const uint8_t value, Cart* const cart)
{
	const auto eval_rom_bank_offset = [cart] {
		const auto mbc1 = cart->mbc1;
		const auto rom_bank_num = 
		  (mbc1.banking_mode == kRomBankingMode
		   ? mbc1.banks_num : mbc1.banks_num_lower_bits)
		   & (g_cart_info.rom_banks() - 1);

		if (rom_bank_num < 0x02) {
			cart->rom_bank_offset = 0x00;
		} else if (mbc1.banking_mode == kRamBankingMode ||
		            (rom_bank_num != 0x20 && 
			     rom_bank_num != 0x40 && 
			     rom_bank_num != 0x60)) {
			cart->rom_bank_offset = 0x4000 * (rom_bank_num - 1);
		} else {
			cart->rom_bank_offset = 0x4000 * rom_bank_num;
		}
	};

	const auto eval_ram_bank_offset = [cart] {
		if (g_cart_info.type() < CartType::RomMBC1Ram ||
		    g_cart_info.ram_banks() < 2 || !cart->ram_enabled)
			return;
		
		const auto mbc1 = cart->mbc1;
		auto offset = g_cart_info.rom_size() - 0xA000;

		if (mbc1.banking_mode == kRamBankingMode) {
			const auto bank_num =
			  mbc1.banks_num_upper_bits&(g_cart_info.ram_banks() - 1);

			offset += 0x2000 * bank_num;
		}
		cart->ram_bank_offset = offset;
	};

	if (address >= 0x6000) {
		const uint8_t new_mode = value ? 1 : 0;
		if (cart->mbc1.banking_mode != new_mode) {
			cart->mbc1.banking_mode = new_mode;
			eval_rom_bank_offset();
			eval_ram_bank_offset();
		}
	} else if (address >= 0x4000) {
		const uint8_t new_val = value & 0x03;
		if (cart->mbc1.banks_num_upper_bits != new_val) {
			cart->mbc1.banks_num_upper_bits = new_val;
			if (cart->mbc1.banking_mode == kRomBankingMode)
				eval_rom_bank_offset();
			else
				eval_ram_bank_offset();
		}
	} else if (address >= 0x2000) {
		const uint8_t new_val = value & 0x1F;
		if (cart->mbc1.banks_num_lower_bits != new_val) {
			cart->mbc1.banks_num_lower_bits = new_val;
			eval_rom_bank_offset();
		}
	} else {
		const auto new_val = value&0x0F;
		if (new_val == 0x0A && g_cart_info.ram_banks() && !cart->ram_enabled) {
			enable_ram(cart);
			eval_ram_bank_offset();
		} else if (new_val != 0x0A && cart->ram_enabled) {
			disable_ram(cart);
		}
	}
}

void write_mbc2(const uint16_t address, const uint8_t value, Cart* const cart)
{
	if (address > 0x3FFF)
		return;

	const auto addr_bit = test_bit(0, get_msb(address));
	if (address >= 0x2000 && addr_bit) {
		const uint8_t new_val = value & 0x0F;
		if (cart->mbc2.rom_bank_num != new_val) {
			cart->mbc2.rom_bank_num = new_val;
			const auto mask = g_cart_info.rom_banks() - 1;
			const auto bank_num = cart->mbc2.rom_bank_num & mask;
			cart->rom_bank_offset = bank_num < 0x02 ? 0x00 : (0x4000 * (bank_num - 1));
		}
	} else if (address <= 0x1FFF && !addr_bit) {
		const auto new_val = value&0x0F;
		if (new_val == 0x0A && !cart->ram_enabled)
			enable_ram(cart);
		else if (new_val != 0x0A && cart->ram_enabled)
			disable_ram(cart);
	}
}

void write_hram(const uint16_t address, const uint8_t value, Gameboy* const gb)
{
	if (address != 0xFFFF) {
		const auto offset = eval_hram_offset(address);
		gb->memory.hram[offset] = value;
	} else {
		gb->hwstate.int_enable = value&0x1F;
	}
}


void write_oam(const uint16_t address, const uint8_t value, Memory* const mem)
{
	if (address < 0xFEA0) {
		const auto offset = eval_oam_offset(address);
		mem->oam[offset] = value;
	}
}

void write_wram(const uint16_t address, const uint8_t value, Memory* const mem)
{
	const auto offset = eval_wram_offset(address);
	mem->wram[offset] = value;
}


void write_vram(const uint16_t address, const uint8_t value, Memory* const mem)
{
	const auto offset = eval_vram_offset(address);
	mem->vram[offset] = value;
}


void write_cart_ram(const uint16_t address, const uint8_t value, Cart* const cart)
{
	debug_printf("Cartridge RAM: write $%X to $%X\n", value, address);
	if (cart->ram_enabled) {
		const auto offset = eval_cart_ram_offset(*cart, address);
		cart->data[offset] = value;
	}
}

uint8_t read_io(const Gameboy& gb, const uint16_t address)
{
	debug_printf("Hardware I/O: read $%X\n", address);

	if (address >= 0xFF10 && address <= 0xFF3F)
		return read_apu_register(gb.apu, address);

	switch (address) {
	case 0xFF00: return gb.joypad.reg.value;
	case 0xFF04: return gb.hwstate.div;
	case 0xFF05: return gb.hwstate.tima;
	case 0xFF06: return gb.hwstate.tma;
	case 0xFF07: return gb.hwstate.tac;
	case 0xFF0F: return gb.hwstate.int_flags;
	case 0xFF40: return gb.ppu.lcdc.value;
	case 0xFF41: return gb.ppu.stat.value;
	case 0xFF42: return gb.ppu.scy;
	case 0xFF43: return gb.ppu.scx;
	case 0xFF44: return gb.ppu.ly;
	case 0xFF45: return gb.ppu.lyc;
	case 0xFF47: return gb.ppu.bgp.value;
	case 0xFF48: return gb.ppu.obp0.value;
	case 0xFF49: return gb.ppu.obp1.value;
	case 0xFF4A: return gb.ppu.wy;
	case 0xFF4B: return gb.ppu.wx;
	default: break;
	}

	return 0xFF;
}

void write_io(const uint16_t address, const uint8_t value, Gameboy* const gb)
{
	debug_printf("Hardware I/O: write $%X to $%X\n", value, address);

	if (address >= 0xFF10 && address <= 0xFF3F) {
		write_apu_register(address, value, &gb->apu);
		return;
	}

	switch (address) {
	case 0xFF00: write_joypad(value, &gb->joypad); break;
	case 0xFF04: write_div(value, &gb->hwstate); break;
	case 0xFF05: gb->hwstate.tima = value; break;
	case 0xFF06: gb->hwstate.tma = value; break;
	case 0xFF07: write_tac(value, &gb->hwstate); break;
	case 0xFF0F: gb->hwstate.int_flags = value&0x1F; break;
	case 0xFF40: write_lcdc(value, &gb->ppu, &gb->hwstate); break;
	case 0xFF41: write_stat(value, &gb->ppu); break;
	case 0xFF42: gb->ppu.scy = value; break;
	case 0xFF43: gb->ppu.scx = value; break;
	case 0xFF44: gb->ppu.ly = 0x00; break;
	case 0xFF45: gb->ppu.lyc = value; break;
	case 0xFF46: dma_transfer(value, gb); break;
	case 0xFF47: write_palette(value, &gb->ppu.bgp); break;
	case 0xFF48: write_palette(value, &gb->ppu.obp0); break;
	case 0xFF49: write_palette(value, &gb->ppu.obp1); break;
	case 0xFF4A: gb->ppu.wy = value; break;
	case 0xFF4B: gb->ppu.wx = value; break;
	default: break;
	}
}


void write_lcdc(const uint8_t value, Ppu* const ppu, HWState* const hwstate)
{
	const auto old_lcd_off = !ppu->lcdc.lcd_on;
	ppu->lcdc.value = value;
	const auto new_lcd_off = !ppu->lcdc.lcd_on;
	if (new_lcd_off) {
		ppu->clock = 0;
		ppu->ly = 0;
		set_ppu_mode(PpuMode::HBlank, ppu, hwstate);
	} else if (old_lcd_off) {
		set_ppu_mode(PpuMode::SearchOAM, ppu, hwstate);
	}
}

void write_stat(const uint8_t value, Ppu* const ppu)
{
	ppu->stat.value = (value&0xF8) | (ppu->stat.value&0x87);
}


void write_joypad(const uint8_t value, Joypad* const pad)
{
	pad->reg.value = (pad->reg.value&0xCF) | (value&0x30);
	const auto buttons = pad->keys.buttons;
	const auto directions = pad->keys.directions;

	switch (static_cast<JoypadMode>(pad->reg.mode)) {
	case JoypadMode::Buttons: pad->reg.keys = buttons; break;
	case JoypadMode::Directions: pad->reg.keys = directions; break;
	case JoypadMode::Both: pad->reg.keys = buttons&directions; break;
	default: pad->reg.keys = 0xF; break;
	}
}


void write_tac(const uint8_t value, HWState* const hwstate)
{
	const auto tac = hwstate->tac = 0xF8|(value&0x07);
	if (hwstate->tima_clock_limit == 0 && test_bit(2, tac))
		hwstate->tima_clock_limit = kTimaClockLimits[tac&3];
}

void write_div(const uint8_t /*value*/, HWState* const hwstate)
{
	hwstate->div = 0;
	hwstate->div_clock = 0;
}



void dma_transfer(const uint8_t value, Gameboy* const gb)
{
	constexpr const auto nbytes = sizeof(Memory::oam);
	const uint16_t address = value * 0x100;
	if (address <= 0x7F5F) {
		const auto offset = eval_cart_rom_offset(gb->cart, address);
		memcpy(gb->memory.oam, &gb->cart.data[offset], nbytes);
	} else if (address <= 0x9F5F) {
		const auto offset = eval_vram_offset(address);
		memcpy(gb->memory.oam, &gb->memory.vram[offset], nbytes);
	} else if (address >= 0xC000 && address <= 0xFD5F) {
		const auto offset = eval_wram_offset(address);
		memcpy(gb->memory.oam, &gb->memory.wram[offset], nbytes);
	} else {
		debug_printf("DMA TRANSFER OPTIMIZATION MISSED!\n");
		auto addr = address;
		for (auto& byte : gb->memory.oam)
			byte = mem_read8(*gb, addr++);
	}
}



int_fast32_t eval_cart_rom_offset(const Cart& cart, const uint16_t address)
{
	assert(address < 0x8000);
	
	const auto offset = address < 0x4000 ? address : cart.rom_bank_offset + address;

	assert(offset >= 0 && address < g_cart_info.rom_size());
	return offset;
}

int_fast32_t eval_cart_ram_offset(const Cart& cart, const uint16_t address)
{
	assert(address >= 0xA000 && address <= 0xBFFF);

	const int_fast32_t offset = cart.ram_bank_offset + address;
	
	assert(offset >= 0 && static_cast<uint_fast32_t>(offset) < (g_cart_info.rom_size() + g_cart_info.ram_size()));

	return offset;
}


int_fast32_t eval_hram_offset(const uint16_t address)
{
	assert(address >= 0xFF80 && address <= 0xFFFE);
	const auto offset = address - 0xFF80;
	assert(offset >= 0 && static_cast<size_t>(offset) < sizeof(Memory::hram));
	return offset;
}

int_fast32_t eval_oam_offset(const uint16_t address)
{
	assert(address >= 0xFE00 && address <= 0xFE9F);
	const auto offset = address - 0xFE00;
	assert(offset >= 0 &&
	       static_cast<size_t>(offset) < sizeof(Memory::oam));
	return offset;
}

int_fast32_t eval_wram_offset(const uint16_t address)
{
	assert(address >= 0xC000 && address <= 0xFDFF);
	const auto offset = address < 0xE000 ?
		address - 0xC000 : address - 0xE000;
	assert(offset >= 0 &&
	       static_cast<size_t>(offset) < sizeof(Memory::wram));
	return offset;
}

int_fast32_t eval_vram_offset(const uint16_t address)
{
	assert(address >= 0x8000 && address <= 0x9FFF);
	const auto offset = address - 0x8000;
	assert(offset >= 0 &&
	       static_cast<size_t>(offset) < sizeof(Memory::vram));
	return offset;
}



} // namespace gbx


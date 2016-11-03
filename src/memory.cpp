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

static void write_lcdc(uint8_t value, Gpu* gpu);
static void write_stat(uint8_t value, Gpu* gpu);
static void write_keys(uint8_t value, Keys* keys);
static void write_div(uint8_t value, HWState* hwstate);
static void write_tac(uint8_t value, HWState* hwstate);
static void dma_transfer(uint8_t value, Gameboy* gb);


uint8_t Gameboy::Read8(const uint16_t address) const 
{
	if (address < 0x8000)
		return read_cart(cart, address);
	else if (address >= 0xFF80)
		return read_hram(*this, address);
	else if (address >= 0xFF00)
		return read_io(*this, address);
	else if (address >= 0xFE00)
		return read_oam(memory, address);
	else if (address >= 0xC000)
		return read_wram(memory, address);
	else if (address >= 0xA000)
		return read_cart_ram(cart, address);
	else
		return read_vram(memory, address);
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
	Write8(address, get_lsb(value));
	Write8(address + 1, get_msb(value));
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
	if (Cart::info.short_type == Cart::ShortType::RomMBC1)
		write_mbc1(address, value, cart);
	else if (Cart::info.short_type == Cart::ShortType::RomMBC2)
		write_mbc2(address, value, cart);
}

void write_mbc1(const uint16_t address, const uint8_t value, Cart* const cart)
{
	const auto eval_rom_bank_offset = [cart] {
		const auto mbc1 = cart->mbc1;
		const auto banking_mode = mbc1.banking_mode;
		const auto rom_bank_num = (banking_mode
			? mbc1.banks_num_lower_bits 
			: mbc1.banks_num) & (Cart::info.rom_banks - 1);

		if (rom_bank_num < 0x02) {
			cart->rom_bank_offset = 0x00;
		} else if (banking_mode == Cart::RamBankingMode ||
		            (rom_bank_num != 0x20 && 
			     rom_bank_num != 0x40 && 
			     rom_bank_num != 0x60)) {
			cart->rom_bank_offset = 0x4000 * (rom_bank_num - 1);
		} else {
			cart->rom_bank_offset = 0x4000 * rom_bank_num;
		}
	};
	const auto eval_ram_bank_offset = [cart] {
		const auto mbc1 = cart->mbc1;
		if (Cart::info.type >= Cart::Type::RomMBC1Ram &&
		    Cart::info.ram_banks > 1 &&
		    mbc1.banking_mode == Cart::RamBankingMode &&
		    cart->ram_enabled && mbc1.banks_num_upper_bits) {
			const auto init_offset = Cart::info.rom_size - 0xA000;
			const auto ram_bank_num = mbc1.banks_num_upper_bits;
			const auto bank_offset = 0x2000 * ram_bank_num;
			cart->ram_bank_offset = init_offset + bank_offset;
		}
	};

	auto& mbc1 = cart->mbc1;
	if (address >= 0x6000) {
		const uint8_t new_mode = value ? 1 : 0;
		if (mbc1.banking_mode != new_mode) {
			mbc1.banking_mode = new_mode;
			eval_rom_bank_offset();
			eval_ram_bank_offset();
		}
	} else if (address >= 0x4000) {
		const uint8_t new_val = value & 0x03;
		if (mbc1.banks_num_upper_bits != new_val) {
			mbc1.banks_num_upper_bits = new_val;
			if (mbc1.banking_mode == Cart::RomBankingMode)
				eval_rom_bank_offset();
			else
				eval_ram_bank_offset();
		}
	} else if (address >= 0x2000) {
		const uint8_t new_val = value & 0x1F;
		if (mbc1.banks_num_lower_bits != new_val) {
			mbc1.banks_num_lower_bits = new_val;
			eval_rom_bank_offset();
		}
	} else {
		const auto new_val = value&0x0F;
		const auto ram_banks = Cart::info.ram_banks;
		if (new_val == 0x0A && ram_banks && !cart->ram_enabled) {
			enable_cart_ram(cart);
			eval_ram_bank_offset();
		} else if (new_val != 0x0A && cart->ram_enabled) {
			disable_cart_ram(cart);
		}
	}
}

void write_mbc2(const uint16_t address, const uint8_t value, Cart* const cart)
{
	if (address > 0x3FFF || !test_bit(0, get_msb(address)))
		return;

	auto& mbc2 = cart->mbc2;
	if (address >= 0x2000) {
		const uint8_t new_val = value & 0x0F;
		if (mbc2.rom_bank_num != new_val) {
			mbc2.rom_bank_num = new_val;
			const auto mask = Cart::info.rom_banks - 1;
			const auto bank_num = mbc2.rom_bank_num & mask;
			cart->rom_bank_offset = bank_num < 0x02
			  ? 0x00 : (0x4000 * (bank_num - 1));
		}
	} else {
		const auto new_val = value&0x0F;
		if (new_val == 0x0A && !cart->ram_enabled)
			enable_cart_ram(cart);
		else if (new_val != 0x0A && cart->ram_enabled)
			disable_cart_ram(cart);
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
	default: break;
	}

	return 0;
}


void write_io(const uint16_t address, const uint8_t value, Gameboy* const gb)
{
	debug_printf("Hardware I/O: write $%X to $%X\n", value, address);
	switch (address) {
	case 0xFF00: write_keys(value, &gb->keys); break;
	case 0xFF04: write_div(value, &gb->hwstate); break;
	case 0xFF05: gb->hwstate.tima = value; break;
	case 0xFF06: gb->hwstate.tma = value; break;
	case 0xFF07: write_tac(value, &gb->hwstate); break;
	case 0xFF0F: gb->hwstate.int_flags = value&0x1F; break;
	case 0xFF40: write_lcdc(value, &gb->gpu); break;
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
	default: break;
	}
}


void write_lcdc(const uint8_t value, Gpu* const gpu)
{
	const auto old_lcd_off = !gpu->lcdc.lcd_on;
	gpu->lcdc.value = value;
	const auto new_lcd_off = !gpu->lcdc.lcd_on;
	if (new_lcd_off) {
		gpu->clock = 0;
		gpu->ly = 0;
		gpu->stat.mode = Gpu::Mode::HBlank;
	} else if (old_lcd_off) {
		gpu->stat.mode = Gpu::Mode::SearchOAM;
	}
}

void write_stat(const uint8_t value, Gpu* const gpu)
{
	gpu->stat.value = (value&0xf8) | (gpu->stat.value&0x87);
}


void write_keys(const uint8_t value, Keys* const keys)
{
	const auto pad = keys->pad.value;
	switch (value&0x30) {
	case 0x10: keys->value = 0xD0 | (pad >> 4); break;
	case 0x20: keys->value = 0xE0 | (pad & 0x0F); break;
	case 0x00: keys->value = 0xC0 | ((pad >> 4) | (pad & 0x0F)); break;
	default: break;
	}
}


void write_tac(const uint8_t value, HWState* const hwstate)
{
	constexpr const int16_t limits[] { 0x400, 0x10, 0x40, 0x100 };
	hwstate->tac = 0xF8|(value&0x07);
	hwstate->tima_clock_limit = limits[value&0x03];
	hwstate->tima_clock = 0x00;
}

void write_div(const uint8_t /*value*/, HWState* const hwstate)
{
	hwstate->div = 0x00;
	hwstate->div_clock = 0x00;
}


void dma_transfer(const uint8_t value, Gameboy* const gb)
{
	constexpr const auto nbytes = sizeof(uint8_t) * 0xA0;
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
			byte = gb->Read8(addr++);
	}
}



int_fast32_t eval_cart_rom_offset(const Cart& cart, const uint16_t address)
{
	assert(address < 0x8000);
	const auto offset = address < 0x4000
		? address : cart.rom_bank_offset + address;
	assert(offset >= 0 && address < Cart::info.rom_size);
	return offset;
}

int_fast32_t eval_cart_ram_offset(const Cart& cart, const uint16_t address)
{
	assert(address >= 0xA000 && address <= 0xBFFF);
	const auto offset = cart.ram_bank_offset + address;
	assert(offset >= 0 &&
	       offset < (Cart::info.rom_size + Cart::info.ram_size));
	return offset;
}


int_fast32_t eval_hram_offset(const uint16_t address)
{
	assert(address >= 0xFF80 && address <= 0xFFFE);
	const auto offset = address - 0xFF80;
	assert(offset >= 0 && 
	       static_cast<size_t>(offset) < sizeof(Memory::hram));
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


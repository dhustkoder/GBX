#ifndef GBX_MEMORY_HPP_
#define GBX_MEMORY_HPP_
#include "Common.hpp"

namespace gbx {




constexpr const size_t GB_MEMORY_MAX_SIZE = 64_Kib;
constexpr const size_t CARTRIDGE_MAX_SIZE = 32_Kib;
constexpr const size_t CARTRIDGE_MIN_SIZE = 32_Kib;

constexpr const uint16_t CARTRIDGE_ENTRY_ADDR = 0x100;
constexpr const uint16_t INTERRUPT_VBLANK_ADDR = 0x40;
constexpr const uint16_t INTERRUPT_LCDC_ADDR = 0x48;
constexpr const uint16_t INTERRUPT_TIMER_ADDR = 0x50;
constexpr const uint16_t INTERRUPT_SERIAL_ADDR = 0x58;
constexpr const uint16_t INTERRUPT_JOYPAD_ADDR = 0x60;


struct Memory
{
	uint8_t zero_page[127];
	uint8_t home[32_Kib];
	uint8_t vram[8_Kib];
	uint8_t ram[8_Kib];
	uint8_t oam[160];
};










}
#endif

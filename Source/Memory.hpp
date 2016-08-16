#ifndef GBX_MEMORY_HPP_
#define GBX_MEMORY_HPP_
#include "Common.hpp"
#include "Cartridge.hpp"

namespace gbx {

/*	General Memory Map (see References/GBPandocs.html)
 *	0000-3FFF   16KB ROM Bank 00     (in cartridge, fixed at bank 00)
 *	4000-7FFF   16KB ROM Bank 01..NN (in cartridge, switchable bank number)
 *	8000-9FFF   8KB Video RAM (VRAM) (switchable bank 0-1 in CGB Mode)
 *	A000-BFFF   8KB External RAM     (in cartridge, switchable bank, if any)
 *	C000-CFFF   4KB Work RAM Bank 0 (WRAM)
 *	D000-DFFF   4KB Work RAM Bank 1 (WRAM)  (switchable bank 1-7 in CGB Mode)
 *	E000-FDFF   Same as C000-DDFF (ECHO)    (typically not used)
 *	FE00-FE9F   Sprite Attribute Table (OAM)
 *	FEA0-FEFF   Not Usable
 *	FF00-FF7F   I/O Ports
 *	FF80-FFFE   High RAM (HRAM)
 *	FFFF        Interrupt Enable Register
 */


constexpr const size_t GB_MEMORY_MAX_SIZE = 64_Kib;


constexpr const uint16_t CARTRIDGE_ENTRY_ADDR = 0x100;
constexpr const uint16_t INTERRUPT_VBLANK_ADDR = 0x40;
constexpr const uint16_t INTERRUPT_LCD_STAT_ADDR = 0x48;
constexpr const uint16_t INTERRUPT_TIMER_ADDR = 0x50;
constexpr const uint16_t INTERRUPT_SERIAL_ADDR = 0x58;
constexpr const uint16_t INTERRUPT_JOYPAD_ADDR = 0x60;


struct Memory
{
	Cartridge cart;
	uint8_t hram[127];
	uint8_t vram[8_Kib];
	uint8_t wram[8_Kib];
	uint8_t oam[160];
};










}
#endif

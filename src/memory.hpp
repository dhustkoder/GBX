#ifndef GBX_MEMORY_HPP_
#define GBX_MEMORY_HPP_
#include "common.hpp"

namespace gbx {

/*	General Memory Map (see references/bgb_doc_html)
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
struct Gameboy;
struct Memory {
	uint8_t hram[127];
	uint8_t wram[8_Kib];
	uint8_t vram[8_Kib];
	uint8_t oam[160];
};

extern uint8_t mem_read8(const Gameboy& gb, uint16_t address);
extern void mem_write8(uint16_t address, uint8_t value, Gameboy* gb);

inline uint16_t mem_read16(const Gameboy& gb, const uint16_t address)
{
	return concat_bytes(mem_read8(gb, address + 1),
                             mem_read8(gb, address));
}

inline void mem_write16(const uint16_t address, const uint16_t value, Gameboy* const gb)
{
	mem_write8(address, get_lsb(value), gb);
	mem_write8(address + 1, get_msb(value), gb);
}


} // namespace gbx
#endif


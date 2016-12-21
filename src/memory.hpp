#ifndef GBX_MEMORY_HPP_
#define GBX_MEMORY_HPP_
#include "common.hpp"

namespace gbx {

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


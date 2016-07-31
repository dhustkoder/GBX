#ifndef GBX_MEMORY_HPP_
#define GBX_MEMORY_HPP_
#include "Common.hpp"

namespace gbx {

constexpr const size_t MEMORY_TOTAL_SIZE = 64_Kib;
constexpr const size_t CARTRIDGE_MAX_SIZE = 32_Kib;
constexpr const size_t CARTRIDGE_MIN_SIZE = 32_Kib;

constexpr const uint16_t CARTRIDGE_ENTRY_ADDR = 0x100;






struct Memory 
{
	int8_t ReadS8(const uint16_t address) const;
	uint8_t ReadU8(const uint16_t address) const;
	uint16_t ReadU16(const uint16_t address) const;
	void WriteU8(const uint16_t address, const uint8_t value);
	void WriteU16(const uint16_t address, const uint16_t value);
	
	uint8_t* data;
};















}
#endif

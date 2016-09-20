#ifndef GBX_CARTRIDGE_HPP_
#define GBX_CARTRIDGE_HPP_
#include <Utix/Ints.h>
#include "Common.hpp"

namespace gbx {

constexpr const size_t CartridgeMaxSize = 32_Kib;
constexpr const size_t CartridgeMinSize = 32_Kib;

enum class CartridgeType : uint8_t 
{
	RomOnly = 0x0,
	RomMBC1 = 0x1,
};

enum class System : uint8_t  
{
	Gameboy, 
	GameboyColor, 
	SuperGameboy
};

struct CartridgeInfo 
{
	char internal_name[0x11];
	size_t size;
	CartridgeType type;
	System system;
};

struct Cartridge 
{
	uint8_t rom_banks[32_Kib];
	static CartridgeInfo info;
};










}
#endif

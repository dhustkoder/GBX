#ifndef GBX_CARTRIDGE_HPP_
#define GBX_CARTRIDGE_HPP_
#include <Utix/Ints.h>
#include "Common.hpp"

namespace gbx {

constexpr const size_t CARTRIDGE_MAX_SIZE = 32_Kib;
constexpr const size_t CARTRIDGE_MIN_SIZE = 32_Kib;

enum class CartridgeType : uint8_t 
{
	ROM_ONLY = 0x0,
	ROM_MBC1 = 0x1,
};

enum class System : uint8_t 
{
	GAMEBOY, GAMEBOY_COLOR, SUPER_GAMEBOY
};

struct CartridgeInfo
{
	char internal_name[17];
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

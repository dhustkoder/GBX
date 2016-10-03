#ifndef GBX_CARTRIDGE_HPP_
#define GBX_CARTRIDGE_HPP_
#include "Common.hpp"

namespace gbx {

constexpr const size_t kCartridgeMaxSize = 32_Kib;
constexpr const size_t kCartridgeMinSize = 32_Kib;

struct Cartridge
{
	enum class Type : uint8_t {
		RomOnly = 0x0,
		RomMBC1 = 0x1,
	};
	enum class System : uint8_t {
		Gameboy, 
		GameboyColor, 
		SuperGameboy
	};

	uint8_t rom_banks[32_Kib];

	static struct Info {
		char internal_name[0x11];
		size_t size;
		Cartridge::Type type;
		Cartridge::System system;
		bool loaded = false;
	} info;
};


} // namespace gbx
#endif


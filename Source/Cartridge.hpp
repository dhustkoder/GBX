#ifndef GBX_CARTRIDGE_HPP_
#define GBX_CARTRIDGE_HPP_
#include "Common.hpp"

namespace gbx {

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

	static struct Info {
		char internal_name[0x11];
		size_t size;
		Cartridge::Type type;
		Cartridge::System system;
		bool ram_enable = false;
	} info;

	union {
		struct {
			uint8_t rom_bank_num_lower_bits : 5;
			uint8_t ram_bank_num : 2;
			uint8_t mode : 1;
		};
		uint8_t rom_bank_num : 7;
	};

	uint8_t rom_banks[];
};

constexpr const size_t kCartridgeMaxSize = 64_Kib;
constexpr const size_t kCartridgeMinSize = 32_Kib;
constexpr const Cartridge::Type kSupportedCartridgeTypes[] {
	Cartridge::Type::RomOnly,
	Cartridge::Type::RomMBC1
};



} // namespace gbx
#endif


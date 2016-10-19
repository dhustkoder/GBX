#ifndef GBX_CARTRIDGE_HPP_
#define GBX_CARTRIDGE_HPP_
#include "Common.hpp"

namespace gbx {

struct Cartridge
{
	enum class Type : uint8_t {
		RomOnly = 0x00,
		RomMBC1 = 0x01,
		RomMBC1Ram = 0x02,
		RomMBC1RamBattery = 0x03
	};
	enum class System : uint8_t {
		Gameboy, 
		GameboyColor, 
		SuperGameboy
	};

	static struct Info {
		char internal_name[0x11];
		size_t rom_size;
		size_t ram_size;
		Cartridge::Type type;
		Cartridge::System system;
	} info;

	union {
		struct {
			uint8_t banks_num_lower_bits : 5;
			uint8_t banks_num_upper_bits : 2;
			uint8_t banking_mode : 1;
		};
		uint8_t banks_num : 7;
	};
	
	int32_t rom_bank_offset;
	int32_t ram_bank_offset;
	uint8_t banks[];
};

constexpr const size_t kCartridgeMaxSize = 64_Kib;
constexpr const size_t kCartridgeMinSize = 32_Kib;

constexpr const Cartridge::Type kSupportedCartridgeTypes[] {
	Cartridge::Type::RomOnly,
	Cartridge::Type::RomMBC1,
	Cartridge::Type::RomMBC1Ram,
	Cartridge::Type::RomMBC1RamBattery
};

constexpr const Cartridge::System kSupportedCartridgeSystems[] {
	Cartridge::System::Gameboy
};


} // namespace gbx
#endif


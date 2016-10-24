#ifndef GBX_CARTRIDGE_HPP_
#define GBX_CARTRIDGE_HPP_
#include "Common.hpp"

namespace gbx {

struct Cart
{
	enum class Type : uint8_t {
		RomOnly = 0x00,
		RomMBC1 = 0x01,
		RomMBC1Ram = 0x02,
		RomMBC1RamBattery = 0x03,
		RomMBC2 = 0x05,
		RomMBC2Battery = 0x06
	};
	enum class System : uint8_t {
		Gameboy, 
		GameboyColorCompat,
		GameboyColorOnly,	
		SuperGameboy
	};

	static struct Info {
		char internal_name[0x11];
		int32_t rom_size;
		int32_t ram_size;
		Cart::Type type;
		Cart::System system;
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

constexpr const int32_t kCartridgeMaxSize = 64_Kib;
constexpr const int32_t kCartridgeMinSize = 32_Kib;

constexpr const Cart::Type kSupportedCartridgeTypes[] {
	Cart::Type::RomOnly,
	Cart::Type::RomMBC1,
	Cart::Type::RomMBC1Ram,
	Cart::Type::RomMBC1RamBattery
};

constexpr const Cart::System kSupportedCartridgeSystems[] {
	Cart::System::Gameboy,
	Cart::System::GameboyColorCompat
};


} // namespace gbx
#endif


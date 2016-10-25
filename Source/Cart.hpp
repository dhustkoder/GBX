#ifndef GBX_CART_HPP_
#define GBX_CART_HPP_
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
	enum class ShortType : uint8_t {
		RomOnly,
		RomMBC1,
		RomMBC2
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
		Cart::ShortType short_type;
		Cart::System system;
	} info;

	union {
		union {
			struct {
				uint8_t banks_num_lower_bits : 5;
				uint8_t banks_num_upper_bits : 2;
				uint8_t banking_mode : 1;
			};
			uint8_t banks_num : 7;
		} mbc1;

		union {
			uint8_t rom_bank_num;
		} mbc2;
	};
	
	int32_t rom_bank_offset;
	union {
		int32_t ram_bank_offset;
		const int32_t ram_enabled;
	};

	uint8_t banks[];
};

constexpr const int32_t kCartridgeMaxSize = 64_Kib;
constexpr const int32_t kCartridgeMinSize = 32_Kib;

constexpr const Cart::Type kSupportedCartridgeTypes[] {
	Cart::Type::RomOnly,
	Cart::Type::RomMBC1,
	Cart::Type::RomMBC1Ram,
	Cart::Type::RomMBC1RamBattery,
	Cart::Type::RomMBC2,
	Cart::Type::RomMBC2Battery
};

constexpr const Cart::System kSupportedCartridgeSystems[] {
	Cart::System::Gameboy,
	Cart::System::GameboyColorCompat
};

constexpr Cart::ShortType get_short_type(const Cart::Type type)
{
	using Type = Cart::Type;
	using ShortType = Cart::ShortType;
	return (type > Type::RomOnly && type <= Type::RomMBC1RamBattery)
	    ? ShortType::RomMBC1
	    : (type > Type::RomMBC1RamBattery && type <= Type::RomMBC2Battery)
	    ? ShortType::RomMBC2
	    : ShortType::RomOnly;
}


} // namespace gbx
#endif


#ifndef GBX_CART_HPP_
#define GBX_CART_HPP_
#include <stdio.h>
#include "common.hpp"

namespace gbx {

enum class CartType : uint8_t {
	RomOnly = 0x00,
	RomMBC1 = 0x01,
	RomMBC1Ram = 0x02,
	RomMBC1RamBattery = 0x03,
	RomMBC2 = 0x05,
	RomMBC2Battery = 0x06
};

enum class CartShortType : uint8_t {
	RomOnly,
	RomMBC1,
	RomMBC2
};

enum class CartSystem : uint8_t {
	Gameboy, 
	GameboyColorCompat,
	GameboyColorOnly
};

enum CartBankingMode : uint8_t {
	kRomBankingMode,
	kRamBankingMode
};

constexpr const CartType kSupportedCartridgeTypes[]{
	CartType::RomOnly,
	CartType::RomMBC1,
	CartType::RomMBC1Ram,
	CartType::RomMBC1RamBattery,
	CartType::RomMBC2,
	CartType::RomMBC2Battery
};

constexpr const CartSystem kSupportedCartridgeSystems[]{
	CartSystem::Gameboy,
	CartSystem::GameboyColorCompat
};

constexpr const CartType kBatteryCartridgeTypes[]{
	CartType::RomMBC1RamBattery,
	CartType::RomMBC2Battery
};

struct Cart {
	static char internal_name[17];
	static owner<char*> sav_file_path;
	static long rom_size;
	static long ram_size;
	static int rom_banks;
	static int ram_banks;
	static CartType type;
	static CartShortType short_type;
	static CartSystem system;

	union {
		union {
			uint8_t banks_num : 7;
			struct {
				uint8_t banks_num_lower_bits : 5;
				uint8_t banks_num_upper_bits : 2;
				uint8_t banking_mode : 1;
			};
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

	uint8_t data[];
};


inline void enable_ram(Cart* const cart) 
{
	cart->ram_bank_offset = cart->rom_size - 0xA000;
}

inline void disable_ram(Cart* const cart)
{
	cart->ram_bank_offset = 0x00;
}


} // namespace gbx
#endif


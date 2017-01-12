#ifndef GBX_CART_HPP_
#define GBX_CART_HPP_
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

struct Cart {
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


extern struct CartInfo {
	char internal_name[17] { 0 };
	owner<char*> sav_file_path = nullptr;
	long rom_size = 0;
	long ram_size = 0;
	int rom_banks = 0;
	int ram_banks = 0;
	CartType type = CartType::RomOnly;
	CartShortType short_type = CartShortType::RomOnly;
	CartSystem system = CartSystem::Gameboy;
} cart_info;

constexpr const CartType kSupportedCartridgeTypes[] {
	CartType::RomOnly,
	CartType::RomMBC1,
	CartType::RomMBC1Ram,
	CartType::RomMBC1RamBattery,
	CartType::RomMBC2,
	CartType::RomMBC2Battery
};

constexpr const CartSystem kSupportedCartridgeSystems[] {
	CartSystem::Gameboy,
	CartSystem::GameboyColorCompat
};

constexpr const CartType kBatteryCartridgeTypes[] {
	CartType::RomMBC1RamBattery,
	CartType::RomMBC2Battery
};


inline const uint8_t* get_rom(const Cart& cart)
{
	return &cart.data[0];
}

inline const uint8_t* get_ram(const Cart& cart)
{
	return &cart.data[cart_info.rom_size];
}

inline uint8_t* get_rom(Cart* const cart)
{
	return const_cast<uint8_t*>(get_rom(*cart));
}

inline uint8_t* get_ram(Cart* const cart)
{
	return const_cast<uint8_t*>(get_ram(*cart));
}

inline void enable_ram(Cart* const cart) 
{
	cart->ram_bank_offset = cart_info.rom_size - 0xA000;
}

inline void disable_ram(Cart* const cart)
{
	cart->ram_bank_offset = 0x00;
}


} // namespace gbx
#endif


#ifndef GBX_CART_HPP_
#define GBX_CART_HPP_
#include <stdio.h>
#include "common.hpp"

namespace gbx {

struct Gameboy;

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


extern class CartInfo {
public:
	const char* internal_name() const { return m_internal_name; }
	uint32_t rom_size() const { return m_rom_size; }
	uint32_t ram_size() const { return m_ram_size; }
	uint8_t rom_banks() const { return m_rom_banks; }
	uint8_t ram_banks() const { return m_ram_banks; }
	CartType type()     const { return m_type; }
	CartShortType short_type() const { return m_short_type; }
	CartSystem system()        const { return m_system; }

private:
	friend Gameboy* create_gameboy(const char*);
	friend void destroy_gameboy(Gameboy*);

	char m_internal_name[17] { 0 };
	char* m_sav_file_path = nullptr;

	uint32_t m_rom_size = 0;
	uint32_t m_ram_size = 0;
	uint8_t m_rom_banks = 0;
	uint8_t m_ram_banks = 0;

	CartType m_type = CartType::RomOnly;
	CartShortType m_short_type = CartShortType::RomOnly;
	CartSystem m_system = CartSystem::Gameboy;

} g_cart_info;


inline void enable_ram(Cart* const cart) 
{
	cart->ram_bank_offset = g_cart_info.rom_size() - 0xA000;
}


inline void disable_ram(Cart* const cart)
{
	cart->ram_bank_offset = 0x00;
}


} // namespace gbx
#endif


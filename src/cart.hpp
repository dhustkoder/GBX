#ifndef GBX_CART_HPP_
#define GBX_CART_HPP_
#include "common.hpp"

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
		GameboyColorOnly
	};

	static struct Info {
		char internal_name[17];
		int32_t rom_size;
		int32_t ram_size;
		uint8_t rom_banks;
		uint8_t ram_banks;
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

	/* rom_bank_offset and ram_bank_offset:
	 * Both points to their right bank offset at data[]
	 * minus the memory map address value for that device. 
	 * That means if the right ROM bank is bank 1 or 0
	 * (the fixed ROM bank is always offset 0 at data[],
	 *  rom_bank_offset is used for switchable banks only)
	 * the rom_bank_offset is equal to 0x4000 - 0x4000
	 * if the right ROM bank is 2 then rom_bank_offset is 0x8000 - 0x4000
	 * we do this on the write operation to the MBC then it save us
	 * from doing the subtraction of 0x4000 on the read operations,
	 * which is used much more often. 
	 * The same applies to ram_bank_offset 
	 * but with a subtraction of - 0xA000, so
	 * if the right RAM bank is bank 0, the ram_bank_offset is
	 * equal to the rom_size - 0xA000, because we allocate the
	 * space for cartridge RAM at the end of data[], after the ROM content.
	 * Then if the right RAM bank is bank 1
	 * the ram_bank_offset is equal to ((rom_size + 0x2000) - 0xA000).
	 * Also if ram_bank_offset equal to 0, that means
	 * the cartridge RAM is disabled
	 */
	int32_t rom_bank_offset;
	union {
		int32_t ram_bank_offset;
		const int32_t ram_enabled;
	};

	uint8_t data[];
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


inline void enable_cart_ram(Cart* const cart) 
{
	cart->ram_bank_offset = Cart::info.rom_size - 0xA000;
}

inline void disable_cart_ram(Cart* const cart)
{
	cart->ram_bank_offset = 0x00;
}


} // namespace gbx
#endif


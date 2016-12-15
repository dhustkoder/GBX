#ifndef GBX_CART_HPP_
#define GBX_CART_HPP_
#include "common.hpp"

namespace gbx {

struct Gameboy;

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
	enum BankingMode : uint8_t {
		RomBankingMode,
		RamBankingMode
	};

private:
	static struct Info {
		char internal_name[18] { 0 };
		owner<char*> sav_file_path = nullptr;
		long rom_size = 0;
		long ram_size = 0;
		int rom_banks = 0;
		int ram_banks = 0;
		Type type = Type::RomOnly;
		ShortType short_type = ShortType::RomOnly;
		System system = System::Gameboy;
	} info;

	friend long get_rom_size(const Cart&);
	friend long get_ram_size(const Cart&);
	friend int get_rom_banks(const Cart&);
	friend int get_ram_banks(const Cart&);
	friend Type get_type(const Cart&);
	friend ShortType get_short_type(const Cart&);
	friend System get_system(const Cart&);
	friend owner<Gameboy*> create_gameboy(const char*);
	friend void destroy_gameboy(owner<Gameboy*>);
public:

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

constexpr const Cart::Type kBatteryCartridgeTypes[] {
	Cart::Type::RomMBC1RamBattery,
	Cart::Type::RomMBC2Battery
};



inline long get_rom_size(const Cart&)
{
	return Cart::info.rom_size;
}

inline long get_ram_size(const Cart&)
{
	return Cart::info.ram_size;
}

inline int get_rom_banks(const Cart&)
{
	return Cart::info.rom_banks;
}

inline int get_ram_banks(const Cart&)
{
	return Cart::info.ram_banks;
}

inline Cart::Type get_type(const Cart&)
{
	return Cart::info.type;
}

inline Cart::ShortType get_short_type(const Cart&)
{
	return Cart::info.short_type;
}

inline Cart::System get_system(const Cart&)
{
	return Cart::info.system;
}

inline const uint8_t* get_rom(const Cart& cart)
{
	return &cart.data[0];
}

inline const uint8_t* get_ram(const Cart& cart)
{
	return &cart.data[get_rom_size(cart)];
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
	cart->ram_bank_offset = get_rom_size(*cart) - 0xA000;
}

inline void disable_ram(Cart* const cart)
{
	cart->ram_bank_offset = 0x00;
}


} // namespace gbx
#endif


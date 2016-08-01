#ifndef GBX_CARTRIDGE_HPP_
#define GBX_CARTRIDGE_HPP_
#include <Utix/Ints.h>

namespace gbx {

enum class CartridgeType : uint8_t 
{
	ROM_ONLY = 0x0,
	ROM_MBC1 = 0x1,
	ROM_MBC1_RAM = 0x2,
	ROM_MBC1_RAM_BATT = 0x3,
	ROM_MBC = 0x5,
	ROM_MBC2_BATT = 0x6,
	ROM_RAM = 0x8,
	ROM_RAM_BATT = 0x9,
	ROM_MMM01 = 0xB,
	ROM_MMM01_SRAM = 0xC,
	ROM_MMM01_SRAM_BATT = 0xD,
	ROM_MBC3_TIMER_BATT = 0xF,
	ROM_MBC3_TIMER_RAM_BATT = 0x10,
	ROM_MBC3 = 0x11,
	ROM_MBC3_RAM = 0x12,
	ROM_MBC3_RAM_BATT = 0x13,
	ROM_MBC5 = 0x19,
	ROM_MBC5_RAM = 0x1A,
	ROM_MBC5_RAM_BATT = 0x1B,
	ROM_MBC5_RUMBLE = 0x1C,
	ROM_MBC5_RUMBLE_SRAM = 0x1D,
	ROM_MBC5_RUMBLE_SRAM_BATT = 0x1E,
	POCKET_CAMERA = 0x1F,
	BANDAI_TAMA5 = 0xFD,
	HUDSON_HUC_3 = 0xFE,
	Hudson_HUC_3 = 0xFF
};

enum class System : uint8_t 
{
	GAMEBOY, GAMEBOY_COLOR, SUPER_GAMEBOY
};


struct CartridgeInfo
{
	char internal_name[17];
	size_t size;
	CartridgeType type;
	System system;
};



extern CartridgeInfo get_cartridge_info(const uint8_t* memory);










}
#endif
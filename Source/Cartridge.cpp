#include <stdio.h>
#include <string.h>
#include "Common.hpp"
#include "Gameboy.hpp"

namespace gbx {

static void fill_cartridge_info(Cartridge* cart);

Cartridge::Info Cartridge::info;

// copies the ROM file (if it is a 32_Kib ROM)
// into the fixed_home - home in memory
bool Gameboy::LoadRom(const char* const file_name) 
{
	{
		FILE* const file = fopen(file_name, "r");
		if (file == nullptr) {
			perror("Couldn't open file");
			return false;
		}

		const auto file_guard = finally([=]{
			fclose(file);
		});
	
		fseek(file, 0, SEEK_END);
		const auto file_size = static_cast<size_t>(ftell(file));
		
		if (file_size > CartridgeMaxSize || file_size < CartridgeMinSize) {
			fprintf(stderr, "size of \'%s\': %zu bytes is incompatible!\n",
			        file_name, file_size);
			return false;
		}

		fseek(file, 0, SEEK_SET);
		fread(memory.cart.rom_banks, sizeof(uint8_t), file_size, file);

		if (ferror(file)) {
			perror("error while reading from file");
			return false;
		}
	} 

	fill_cartridge_info(&memory.cart);
	return this->Reset();
}


// parse ROM header for information
void fill_cartridge_info(Cartridge* const cart)
{
	auto& cinfo = cart->info;

	// 0134 - 0142 game's title
	memcpy(cinfo.internal_name, &cart->rom_banks[0x134], 0x10);
	cinfo.internal_name[0x10] = '\0';

	const auto super_gb_check = cart->rom_banks[0x146];
	if (super_gb_check == 0x03) {
		cinfo.system = Cartridge::System::SuperGameboy;
	} else {
		const auto color_check = cart->rom_banks[0x143];
		if (color_check == 0x80)
			cinfo.system = Cartridge::System::GameboyColor;
		else
			cinfo.system = Cartridge::System::Gameboy;
	}

	cinfo.type = static_cast<Cartridge::Type>(cart->rom_banks[0x147]);
	const uint8_t size_code = cart->rom_banks[0x148];

	switch (size_code) {
	case 0x00: cinfo.size = 32_Kib; break;    // 2 banks
	case 0x01: cinfo.size = 64_Kib; break;    // 4 banks
	//case 0x02: cinfo.size = 128_Kib; break; // 8 banks
	//case 0x03: cinfo.size = 256_Kib; break; // 16 banks
	//case 0x04: cinfo.size = 512_Kib; break; // 32 banks
	//case 0x05: cinfo.size = 1_Mib; break;   // 64 banks
	//case 0x06: cinfo.size = 2_Mib; break;   // 128 banks
	default: cinfo.size = 0; break;
	}
}



} // namespace gbx


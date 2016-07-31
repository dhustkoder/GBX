#include <stdio.h>
#include <Utix/ScopeExit.h>
#include "Memory.hpp"
#include "Cartridge.hpp"

namespace gbx {


bool FillCartridgeInfo(const char* file_name, CartridgeInfo* const cinfo)
{
	FILE* const file = fopen(file_name, "r");
	if (!file) {
		perror("couldn't open file");
		return false;
	}
	
	const auto file_guard = utix::MakeScopeExit([=]{ fclose(file); });
	
	fseek(file, 0, SEEK_END);
	const auto file_size = static_cast<size_t>(ftell(file));
	if (file_size > CARTRIDGE_MAX_SIZE || file_size < CARTRIDGE_MIN_SIZE) {
		fprintf(stderr, "\'%s\' size not compatible!\n", file_name);
		return false;
	}

	cinfo->size = file_size;
	
	fseek(file, 0x134, SEEK_SET);

	fread(cinfo->internal_name, sizeof(uint8_t), 16, file);

	fseek(file, 0x146, SEEK_SET);

	const auto system_type = fgetc(file);
	
	if (system_type == 0x00) {
		fseek(file, 0x143, SEEK_SET);
		const auto is_color = fgetc(file);
		if(is_color == 0x80)
			cinfo->system = System::GAMEBOY_COLOR;
		else
			cinfo->system = System::GAMEBOY;
	}
	else if (system_type == 0x03) {
		cinfo->system = System::SUPER_GAMEBOY;
	}
	else {
		fprintf(stderr, "unkown Cartridge system!\n");
		return false;
	}

	fseek(file, 0x147, SEEK_SET);
	cinfo->type = static_cast<CartridgeType>(fgetc(file));

	return true;
}














}

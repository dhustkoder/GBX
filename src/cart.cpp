#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "gameboy.hpp"

namespace gbx {

CartInfo g_cart_info;

inline void reset(Gameboy* gb);

inline bool extract_rom_header_info(FILE* rom_file,
                                    char(*namebuffer)[17],
                                    CartType* type,
                                    CartShortType* short_type,
                                    CartSystem* system,
                                    uint32_t* rom_size,
                                    uint32_t* ram_size,
                                    uint8_t* rom_banks,
                                    uint8_t* ram_banks);

inline bool extract_rom_data(FILE* rom_file, Cart* cart);
inline owner<char*> eval_sav_file_path(const char* rom_file_path);
inline bool load_sav_file(const char* sav_file_path, Cart* cart);
inline void update_sav_file(const Cart& cart, const char* sav_file_path);


owner<Gameboy*> create_gameboy(const char* const rom_file_path)
{
	owner<FILE* const> rom_file = fopen(rom_file_path, "rb");

	if (rom_file == nullptr) {
		perror("Couldn't open file");
		return nullptr;
	}


	const auto file_guard = finally([rom_file] {
		fclose(rom_file);
	});


	const bool success = extract_rom_header_info
	                   (
	                       rom_file,
	                       &g_cart_info.m_internal_name,
	                       &g_cart_info.m_type,
	                       &g_cart_info.m_short_type,
	                       &g_cart_info.m_system,
	                       &g_cart_info.m_rom_size,
	                       &g_cart_info.m_ram_size,
	                       &g_cart_info.m_rom_banks,
	                       &g_cart_info.m_ram_banks
	                    );

	if (!success)
		return nullptr;


	const auto memsize = sizeof(Gameboy) + g_cart_info.m_rom_size + g_cart_info.m_ram_size;
	owner<Gameboy* const> gb = static_cast<Gameboy*>(malloc(memsize));
	if (gb == nullptr) {
		perror("Couldn't allocate memory");
		return nullptr;
	}

	auto gb_guard = finally([gb] {
		destroy_gameboy(gb);
	});

	reset(gb);

	if (is_in_array(kBatteryCartridgeTypes, g_cart_info.m_type)) {
		g_cart_info.m_sav_file_path = eval_sav_file_path(rom_file_path);
		if (g_cart_info.m_sav_file_path == nullptr ||
		    !load_sav_file(g_cart_info.m_sav_file_path, &gb->cart))
			    return nullptr;
	}

	if (!extract_rom_data(rom_file, &gb->cart))
		return nullptr;

	printf("CARTRIDGE INFO\n"
	       "NAME: %s\n"
	       "ROM SIZE: %u\n"
	       "RAM SIZE: %u\n"
	       "ROM BANKS: %u\n"
	       "RAM BANKS: %u\n"
	       "TYPE CODE: %u\n"
	       "SYSTEM CODE: %u\n",
	       g_cart_info.m_internal_name,
	       g_cart_info.m_rom_size, g_cart_info.m_ram_size,
	       g_cart_info.m_rom_banks, g_cart_info.m_ram_banks,
	       static_cast<int>(g_cart_info.m_type),
	       static_cast<int>(g_cart_info.m_system));

	gb_guard.abort();
	return gb;
}


void destroy_gameboy(owner<Gameboy*> gb)
{
	if (g_cart_info.m_sav_file_path != nullptr) {
		update_sav_file(gb->cart, g_cart_info.m_sav_file_path);
		free(g_cart_info.m_sav_file_path);
		g_cart_info.m_sav_file_path = nullptr;
	}

	free(gb);
}


void reset(Gameboy* const gb)
{
	memset(gb, 0, sizeof(*gb));

	// init the system
	// up to now only Gameboy (DMG) mode is supported
	gb->cpu.pc = 0x0100;
	gb->cpu.sp = 0xFFFE;
	gb->cpu.af = 0x01B0;
	gb->cpu.bc = 0x0013;
	gb->cpu.de = 0x00D8;
	gb->cpu.hl = 0x014D;

	gb->gpu.lcdc.value = 0x91;
	gb->gpu.stat.value = 0x85;
	write_pallete(0xFC, &gb->gpu.bgp);
	write_pallete(0xFF, &gb->gpu.obp0);
	write_pallete(0xFF, &gb->gpu.obp1);

	gb->apu.ch1.nr10.value = 0x80;
	write_nr11(0xBF, &gb->apu);
	gb->apu.ch1.nr12.value = 0xF3;
	gb->apu.ch1.nr14.value = 0xBF;
	write_nr21(0x3F, &gb->apu);
	gb->apu.ch2.nr24.value = 0xBF;
	gb->apu.ch3.nr30.value = 0x7F;
	write_nr31(0xFF, &gb->apu);
	gb->apu.ch3.nr32.value = 0x9F;
	gb->apu.ch3.nr33.value = 0xBF;
	write_nr41(0xFF, &gb->apu);
	gb->apu.ch4.nr44.value = 0xBF;
	gb->apu.ctl.nr50.value = 0x77;
	gb->apu.ctl.nr51.value = 0xF3;
	gb->apu.ctl.nr52.value = 0xF1;

	gb->hwstate.tac = 0xF8;

	gb->joypad.reg.value = 0xFF;
	gb->joypad.keys.both = 0xFF;
}


bool extract_rom_data(FILE* const rom_file, Cart* const cart)
{
	const size_t rom_size = g_cart_info.rom_size();

	errno = 0;
	if (fseek(rom_file, 0, SEEK_SET) != 0 || fread(cart->data, 1, rom_size, rom_file) < rom_size) {
		if (errno != 0)
			perror("Couldn't read from file");
		else
			fputs("ROM's size is invalid\n", stderr);
		return false;
	}

	return true;
}


owner<char*> eval_sav_file_path(const char* const rom_file_path)
{
	const auto rom_path_size = strlen(rom_file_path);
	const auto sav_path_size = rom_path_size + 5;

	owner<char* const> sav_file_path = 
	  static_cast<char*>(calloc(sav_path_size, sizeof(char)));

	if (sav_file_path == nullptr) {
		perror("Couldn't allocate memory");
		return nullptr;
	}

	auto sav_file_path_guard = finally([sav_file_path] {
		free(sav_file_path);
	});

	const size_t dot_offset = [rom_file_path, rom_path_size]()-> size_t {
		const char* p = &rom_file_path[rom_path_size - 1];
		while (*p != '.' && p != &rom_file_path[0])
			--p;
		return (*p == '.')
			? p - &rom_file_path[0] : rom_path_size - 1;
	}();

	memcpy(sav_file_path, rom_file_path, sizeof(char) * dot_offset);
	strcat(sav_file_path, ".sav");

	sav_file_path_guard.abort();
	return sav_file_path;
}


bool load_sav_file(const char* const sav_file_path, Cart* const cart)
{
	if (owner<FILE* const> sav_file = fopen(sav_file_path, "rb+")) {
		const auto sav_file_guard = finally([sav_file] {
			fclose(sav_file);
		});
		
		const size_t ram_size = g_cart_info.ram_size();
		uint8_t* const ram = &cart->data[g_cart_info.rom_size()];
		if (fread(ram, 1, ram_size, sav_file) < ram_size)
			fputs("Error while reading sav file\n", stderr);

	} else if (errno != ENOENT) {
		perror("Couldn't open sav file");
		return false;
	}

	return true;
}


void update_sav_file(const Cart& cart, const char* const sav_file_path)
{
	owner<FILE* const> sav_file = fopen(sav_file_path, "wb");
	if (sav_file == nullptr) {
		perror("Couldn't open sav file");
		return;
	}

	const auto sav_file_guard = finally([sav_file] {
		fclose(sav_file);
	});

	const size_t ram_size = g_cart_info.ram_size();
	const uint8_t* const ram = &cart.data[g_cart_info.rom_size()];

	if (fwrite(ram, 1, ram_size, sav_file) < ram_size)
		perror("Error while updating sav file");
}




inline bool header_read_name(const uint8_t(&header)[0x4F], char(*buffer)[17]);
inline bool header_read_types_and_sizes(const uint8_t(&header)[0x4F],
                                        CartType* type,
                                        CartShortType* short_type,
                                        CartSystem* system,
                                        uint32_t* rom_size,
                                        uint32_t* ram_size,
                                        uint8_t* rom_banks,
                                        uint8_t* ram_banks);

bool extract_rom_header_info(FILE* const rom_file,
                             char(* const namebuffer)[17],
                             CartType* const type,
                             CartShortType* const short_type,
                             CartSystem* const system,
                             uint32_t* const rom_size,
                             uint32_t* const ram_size,
                             uint8_t* const rom_banks,
                             uint8_t* const ram_banks)
{
	uint8_t header[0x4F];
	errno = 0;
	if (fseek(rom_file, 0x100, SEEK_SET) != 0 || fread(header, 1, 0x4F, rom_file) < 0x4F) {
		if (errno != 0)
			perror("Couldn't read from file");
		else
			fputs("ROM's size is invalid\n", stderr);
		return false;
	}

	return header_read_name(header, namebuffer) && 
	       header_read_types_and_sizes(header, type, short_type, system,
	                                   rom_size, ram_size, rom_banks, ram_banks);
}


bool header_read_name(const uint8_t(&header)[0x4F], char(*const buffer)[17])
{
	memcpy(buffer, &header[0x34], 16);
	(*buffer)[16] = '\0';

	if (strlen(*buffer) == 0) {
		fputs("ROM's internal name is invalid\n", stderr);
		return false;
	}

	return true;
}


bool header_read_types_and_sizes(const uint8_t(&header)[0x4F],
                                 CartType* const type,
                                 CartShortType* const short_type,
                                 CartSystem* const system,
                                 uint32_t* const rom_size,
                                 uint32_t* const ram_size,
                                 uint8_t* const rom_banks,
                                 uint8_t* const ram_banks)
{
	*type = static_cast<CartType>(header[0x47]);
	*system = [&] {
		switch (header[0x43]) {
		case 0xC0: return CartSystem::GameboyColorOnly; break;
		case 0x80: return CartSystem::GameboyColorCompat; break;
		default: return CartSystem::Gameboy; break;
		}
	}();

	if (!is_in_array(kSupportedCartridgeTypes, *type)) {
		fprintf(stderr, "Cartridge %s %u not supported\n",
			"type", static_cast<unsigned>(*type));
		return false;
	} else if (!is_in_array(kSupportedCartridgeSystems, *system)) {
		fprintf(stderr, "Cartridge %s %u not supported\n",
			"system", static_cast<unsigned>(*system));
		return false;
	}

	if (*type >= CartType::RomMBC1 && *type <= CartType::RomMBC1RamBattery)
		*short_type = CartShortType::RomMBC1;
	else if (*type >= CartType::RomMBC2 && *type <= CartType::RomMBC2Battery)
		*short_type = CartShortType::RomMBC2;
	else
		*short_type = CartShortType::RomOnly;


	struct SizeInfo { const uint32_t size; const uint8_t banks; };
	constexpr const SizeInfo rom_sizes[7]{
		{ 32_Kib, 2 },{ 64_Kib, 4 },{ 128_Kib, 8 },{ 256_Kib, 16 },
		{ 512_Kib, 32 },{ 1_Mib, 64 },{ 2_Mib, 128 }
	};
	constexpr const SizeInfo ram_sizes[4]{
		{ 0, 0 },{ 2_Kib, 1 },{ 8_Kib, 1 },{ 32_Kib, 4 }
	};

	const auto rom_code = header[0x48];
	const auto ram_code = header[0x49];

	if (rom_code >= arr_size(rom_sizes) ||
		ram_code >= arr_size(ram_sizes)) {
		fputs("Invalid size codes\n", stderr);
		return false;
	}

	*rom_size = rom_sizes[rom_code].size;
	*rom_banks = rom_sizes[rom_code].banks;
	*ram_size = ram_sizes[ram_code].size;
	*ram_banks = ram_sizes[ram_code].banks;

	if (*short_type == CartShortType::RomOnly && (*ram_size != 0x00 || *rom_size != 32_Kib)) {
		fputs("Invalid size codes for RomOnly\n", stderr);
		return false;
	} else if (*short_type == CartShortType::RomMBC2) {
		if (*rom_size <= 256_Kib && *ram_size == 0x00) {
			*ram_size = 512;
			*ram_banks = 1;
		} else {
			fputs("Invalid size codes for MBC2\n", stderr);
			return false;
		}
	}

	return true;
}



} // namespace gbx

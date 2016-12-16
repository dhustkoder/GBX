#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "instructions.hpp"
#include "gameboy.hpp"

namespace gbx {

extern void update_gpu(int16_t cycles, const Memory& mem, HWState* hwstate, Gpu* gpu);
static void update_timers(int16_t cycles, HWState* hwstate);
static void update_interrupts(Gameboy* gb);

void reset(Gameboy* const gb)
{
	memset(gb, 0, sizeof(*gb));
	memset(&Gpu::screen[0][0], 0xFF, sizeof(Gpu::screen));

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
	gb->gpu.bgp = 0xFC;
	gb->gpu.obp0 = 0xFF;
	gb->gpu.obp1 = 0xFF;

	gb->hwstate.tac = 0xF8;

	gb->joypad.reg.value = 0xFF;
	gb->joypad.buttons.value = 0xF;
	gb->joypad.directions.value = 0xF;
}


void run_for(const int32_t clock_limit, Gameboy* const gb)
{
	do {
		const auto before = gb->cpu.clock;
		update_interrupts(gb);

		if (!gb->hwstate.flags.cpu_halt) {
			const uint8_t opcode = mem_read8(*gb, gb->cpu.pc++);
			main_instructions[opcode](gb);
			gb->cpu.clock += clock_table[opcode];
		} else {
			gb->cpu.clock += 4;
		}

		const auto step_cycles =
		  static_cast<int16_t>(gb->cpu.clock - before);

		update_gpu(step_cycles, gb->memory, &gb->hwstate, &gb->gpu);
		update_timers(step_cycles, &gb->hwstate);
	} while (gb->cpu.clock < clock_limit ||
	         (gb->gpu.ly > 0 && gb->gpu.ly < 144));

	gb->cpu.clock -= clock_limit;
}


void update_timers(const int16_t cycles, HWState* const hwstate)
{
	hwstate->div_clock += cycles;
	if (hwstate->div_clock >= 0x100) {
		++hwstate->div;
		hwstate->div_clock -= 0x100;
	}

	const auto tac = hwstate->tac;
	if (test_bit(2, tac)) {
		hwstate->tima_clock += cycles;
		while (hwstate->tima_clock >= hwstate->tima_clock_limit) {
			inc_tima(hwstate);
			hwstate->tima_clock -= hwstate->tima_clock_limit;
			hwstate->tima_clock_limit = kTimaClockLimits[tac&3];
		}
	}
}


void update_interrupts(Gameboy* const gb)
{
	const uint8_t pendents = get_pendent_interrupts(gb->hwstate);
	const auto flags = gb->hwstate.flags;
	if (pendents && flags.cpu_halt) {
		gb->hwstate.flags.cpu_halt = 0x00;
		gb->cpu.clock += 4;
	}

	if (flags.ime == 0x00) {
		return;
	} else if (flags.ime == 0x01) {
		gb->hwstate.flags.ime = 0x02;
		return;
	}

	if (!pendents)
		return;

	gb->hwstate.flags.ime = 0x00;
	for (const auto interrupt : kInterrupts) {
		if (pendents & interrupt.mask) {
			clear_interrupt(interrupt, &gb->hwstate);
			stack_push16(gb->cpu.pc, gb);
			gb->cpu.pc = interrupt.addr;
			gb->cpu.clock += 20;
			break;
		}
	}
}


inline bool extract_rom_header(FILE* rom_file, uint8_t(*buffer)[0x4F]);
inline bool extract_rom_data(FILE* rom_file, size_t rom_size, Cart* cart);
inline bool header_read_name(const uint8_t(&header)[0x4F], char(*buffer)[17]);

inline bool header_read_types(const uint8_t(&header)[0x4F],
                               CartType* type,
			       CartShortType* short_type,
			       CartSystem* system);

inline bool header_read_sizes(const uint8_t(&header)[0x4F],
                               CartShortType short_type,
                               long* rom_size, long* ram_size,
			       int* rom_banks, int* ram_banks);

inline owner<char*> eval_sav_file_path(const char* rom_file_path);
inline bool load_sav_file(const char* sav_file_path, Cart* cart);
inline void update_sav_file(const Cart& cart, const char* sav_file_path);
CartInfo cart_info;


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

	uint8_t header[0x4F];
	auto& info = cart_info;
	if (!extract_rom_header(rom_file, &header) ||
	    !header_read_name(header, &info.internal_name) ||
	    !header_read_types(header, &info.type,
	                      &info.short_type, &info.system) ||
	    !header_read_sizes(header, info.short_type,
		                &info.rom_size, &info.ram_size,
				&info.rom_banks, &info.ram_banks)) {
		return nullptr;
	}

	const auto memsize = sizeof(Gameboy) + info.rom_size + info.ram_size;
	owner<Gameboy* const> gb = static_cast<Gameboy*>(malloc(memsize));
	if (gb == nullptr) {
		perror("Couldn't allocate memory");
		return nullptr;
	}

	auto gb_guard = finally([gb] {
		destroy_gameboy(gb);
	});

	reset(gb);

	if (is_in_array(kBatteryCartridgeTypes, info.type)) {
		info.sav_file_path = eval_sav_file_path(rom_file_path);
		if (info.sav_file_path == nullptr ||
		     !load_sav_file(info.sav_file_path, &gb->cart))
			return nullptr;
	}

	if (!extract_rom_data(rom_file, info.rom_size, &gb->cart))
		return nullptr;

	printf("CARTRIDGE INFO\n"
	       "NAME: %s\n"
	       "ROM SIZE: %zu\n"
	       "RAM SIZE: %zu\n"
	       "ROM BANKS: %d\n"
	       "RAM BANKS: %d\n"
	       "TYPE CODE: %u\n"
	       "SYSTEM CODE: %u\n",
	       info.internal_name,
	       info.rom_size, info.ram_size,
	       info.rom_banks, info.ram_banks,
	       static_cast<unsigned>(info.type),
	       static_cast<unsigned>(info.system));

	gb_guard.Abort();
	return gb;
}


void destroy_gameboy(owner<Gameboy* const> gb)
{
	if (owner<char* const> sav_file_path = cart_info.sav_file_path) {
		const auto sav_file_path_guard = finally([sav_file_path] {
			free(sav_file_path);
			cart_info.sav_file_path = nullptr;
		});
		update_sav_file(gb->cart, sav_file_path);
	}

	free(gb);
}


bool extract_rom_header(FILE* const rom_file, uint8_t(*const buffer)[0x4F])
{
	errno = 0;
	if (fseek(rom_file, 0x100, SEEK_SET) != 0 ||
	     fread(buffer, 1, 0x4F, rom_file) < 0x4F) {
		if (errno != 0)
			perror("Error while reading from file");
		else
			fprintf(stderr, "Error while reading from file\n");
		return false;
	}
	return true;
}


bool extract_rom_data(FILE* const rom_file,
                      const size_t rom_size,
                      Cart* const cart)
{
	errno = 0;
	if (fseek(rom_file, 0, SEEK_SET) != 0 ||
	     fread(cart->data, 1, rom_size, rom_file) < rom_size) {
		if (errno != 0)
			perror("Error while reading from file");
		else
			fprintf(stderr, "Error while reading from file\n");
		return false;
	}
	return true;
}


bool header_read_name(const uint8_t(&header)[0x4F], char(*const buffer)[17])
{
	memcpy(buffer, &header[0x34], 16);
	(*buffer)[16] = '\0';
	if (strlen(*buffer) == 0) {
		fprintf(stderr, "The ROM's internal name is invalid.\n");
		return false;
	}
	return true;
}


bool header_read_types(const uint8_t(&header)[0x4F],
                        CartType* const type,
                        CartShortType* const short_type,
                        CartSystem* const system)
{
	using Type = CartType;

	*type = static_cast<CartType>(header[0x47]);

	switch (header[0x43]) {
	case 0xC0: *system =  CartSystem::GameboyColorOnly; break;
	case 0x80: *system = CartSystem::GameboyColorCompat; break;
	default: *system = CartSystem::Gameboy; break;
	}
	

	if (!is_in_array(kSupportedCartridgeTypes, *type)) {
		fprintf(stderr, "Cartridge type %u not supported\n",
		        static_cast<unsigned>(*type));
		return false;
	} else if (!is_in_array(kSupportedCartridgeSystems, *system)) {
		fprintf(stderr, "Cartridge system %u not supported\n",
		        static_cast<unsigned>(*system));
		return false;
	}

	if (*type >= Type::RomMBC1 && *type <= Type::RomMBC1RamBattery)
		*short_type = CartShortType::RomMBC1;
	else if (*type >= Type::RomMBC2 && *type <= Type::RomMBC2Battery)
		*short_type = CartShortType::RomMBC2;
	else
		*short_type = CartShortType::RomOnly;

	return true;
}


bool header_read_sizes(const uint8_t(&header)[0x4F],
		        const CartShortType short_type,
                        long* const rom_size, long* const ram_size,
                        int* const rom_banks, int* const ram_banks)
{
	struct SizeInfo { const size_t size; const int banks; };
	constexpr const SizeInfo rom_sizes[7] {
		{32_Kib, 2}, {64_Kib, 4}, {128_Kib, 8}, {256_Kib, 16},
		{512_Kib, 32}, {1_Mib, 64}, {2_Mib, 128}
	};
	constexpr const SizeInfo ram_sizes[4] { 
		{0, 0}, {2_Kib, 1}, {8_Kib, 1}, {32_Kib, 4}
	};
	const uint8_t size_codes[2] { header[0x48], header[0x49] };
	
	if (size_codes[0] >= 7 || size_codes[1] >= 4) {
		fprintf(stderr, "Invalid size codes\n");
		return false;
	}
	
	const auto rom_size_tmp = rom_sizes[size_codes[0]].size;
	const auto rom_banks_tmp = rom_sizes[size_codes[0]].banks;
	auto ram_size_tmp = ram_sizes[size_codes[1]].size;
	auto ram_banks_tmp = ram_sizes[size_codes[1]].banks;

	if (short_type == CartShortType::RomOnly &&
	     (ram_size_tmp != 0x00 || rom_size_tmp != 32_Kib)) {
		fprintf(stderr, "Invalid size codes for RomOnly\n");
		return false;
	} else if (short_type == CartShortType::RomMBC2) {
		if (rom_size_tmp <= 256_Kib && ram_size_tmp == 0x00) {
			ram_size_tmp = 512;
			ram_banks_tmp = 1;
		} else {
			fprintf(stderr, "Invalid size codes for MBC2\n");
			return false;
		}
	}

	*rom_size = static_cast<long>(rom_size_tmp);
	*ram_size = static_cast<long>(ram_size_tmp);
	*rom_banks = rom_banks_tmp;
	*ram_banks = ram_banks_tmp;
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

	const size_t dot_offset = 
	[rom_file_path, rom_path_size]()-> size_t {
		const char* p = &rom_file_path[rom_path_size - 1];
		while (*p != '.' && p != &rom_file_path[0])
			--p;
		return (*p == '.')
			? p - &rom_file_path[0] : rom_path_size - 1;
	}();

	memcpy(sav_file_path, rom_file_path, sizeof(char) * dot_offset);
	strcat(sav_file_path, ".sav");
	sav_file_path_guard.Abort();
	return sav_file_path;
}


bool load_sav_file(const char* const sav_file_path, Cart* const cart)
{
	if (owner<FILE* const> sav_file = fopen(sav_file_path, "rb+")) {
		const auto sav_file_guard = finally([sav_file] {
			fclose(sav_file);
		});
		const size_t ram_size = get_ram_size(*cart);
		uint8_t* const ram = get_ram(cart);	
		if (fread(ram, 1, ram_size, sav_file) < ram_size)
			fprintf(stderr, "Error while loading sav file");
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

	const size_t ram_size = get_ram_size(cart);
	const uint8_t* const ram = get_ram(cart);
	if (fwrite(ram, 1, ram_size, sav_file) < ram_size)
		perror("Error while updating sav file");
}


} // namespace gbx


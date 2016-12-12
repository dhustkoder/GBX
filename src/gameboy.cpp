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
Cart::Info Cart::info;

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
		const int32_t before = gb->cpu.clock;
		update_interrupts(gb);

		int16_t step_cycles;
		if (!gb->hwstate.flags.cpu_halt) {
			const uint8_t opcode = mem_read8(*gb, gb->cpu.pc++);
			main_instructions[opcode](gb);
			step_cycles = clock_table[opcode];
		} else {
			step_cycles = 4;
		}

		const int32_t after = gb->cpu.clock;
		gb->cpu.clock += step_cycles;
		step_cycles += static_cast<int16_t>(after - before);
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
	for (const auto inter : interrupts::array) {
		if (pendents & inter.mask) {
			clear_interrupt(inter, &gb->hwstate);
			stack_push16(gb->cpu.pc, gb);
			gb->cpu.pc = inter.addr;
			gb->cpu.clock += 20;
			break;
		}
	}
}

static owner<char*> eval_sav_file_path(const char* rom_file_path);
static bool load_sav_file(const char* sav_file_path, Cart* cart);
static void update_sav_file(const Cart& cart, const char* sav_file_path);

owner<Gameboy*> create_gameboy(const char* const rom_file_path)
{
	using Type = Cart::Type;
	using ShortType = Cart::ShortType;
	using System = Cart::System;

	owner<FILE* const> rom_file = fopen(rom_file_path, "rb");
	if (rom_file == nullptr) {
		perror("Couldn't open file");
		return nullptr;
	}

	const auto file_guard = finally([rom_file] {
		fclose(rom_file);
	});

	uint8_t header[0x4F];
	fseek(rom_file, 0x100, SEEK_SET);
	if (fread(header, 1, 0x4F, rom_file) < 0x4F) {
		fprintf(stderr, "Error while reading from file\n");
		return nullptr;
	}

	const auto system = [&header] ()-> System {
		switch (header[0x43]) {
		case 0xC0: return System::GameboyColorOnly;
		case 0x80: return System::GameboyColorCompat;
		default: return System::Gameboy;
		}
	}();

	const auto type = static_cast<Type>(header[0x47]);

	if (!is_in_array(kSupportedCartridgeTypes, type)) {
		fprintf(stderr, "Cartridge type %u not supported\n",
		        static_cast<unsigned>(type));
		return nullptr;
	} else if (!is_in_array(kSupportedCartridgeSystems, system)) {
		fprintf(stderr, "Cartridge system %u not supported\n",
		        static_cast<unsigned>(system));
		return nullptr;
	}
	
	const auto short_type = [type] ()-> ShortType {
		if (type >= Type::RomMBC1 && type <= Type::RomMBC1RamBattery)
			return ShortType::RomMBC1;
		else if (type >= Type::RomMBC2 && type <= Type::RomMBC2Battery)
			return ShortType::RomMBC2;
		else
			return ShortType::RomOnly;
	}();

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
		return nullptr;
	}
	
	const auto rom_size = rom_sizes[size_codes[0]].size;
	const auto rom_banks = rom_sizes[size_codes[0]].banks;
	auto ram_size = ram_sizes[size_codes[1]].size;
	auto ram_banks = ram_sizes[size_codes[1]].banks;

	if (short_type == ShortType::RomOnly &&
	    (ram_size != 0x00 || rom_size != 32_Kib)) {
		fprintf(stderr, "Invalid size codes for RomOnly\n");
		return nullptr;
	} else if (short_type == ShortType::RomMBC2) {
		if (rom_size <= 256_Kib && ram_size == 0x00) {
			ram_size = 512;
			ram_banks = 1;
		} else {
			fprintf(stderr, "Invalid size codes for MBC2\n");
			return nullptr;
		}
	}

	owner<Gameboy* const> gb =
	  static_cast<Gameboy*>(malloc(sizeof(Gameboy) + rom_size + ram_size));
	
	if (gb == nullptr) {
		perror("Couldn't allocate memory");
		return nullptr;
	}

	auto gb_guard = finally([gb] {
		destroy_gameboy(gb);
	});

	fseek(rom_file, 0, SEEK_SET);

	if (fread(&gb->cart.data[0], 1, rom_size, rom_file) < rom_size) {
		fprintf(stderr, "Error while reading from file\n");
		return nullptr;
	}

	memcpy(Cart::info.internal_name, &header[0x34], 16);
	Cart::info.rom_size = rom_size;
	Cart::info.ram_size = ram_size;
	Cart::info.rom_banks = rom_banks;
	Cart::info.ram_banks = ram_banks;
	Cart::info.type = type;
	Cart::info.short_type = short_type;
	Cart::info.system = system;

	if (is_in_array(kBatteryCartridgeTypes, type)) {
		Cart::info.sav_file_path = eval_sav_file_path(rom_file_path);
		if (!load_sav_file(Cart::info.sav_file_path, &gb->cart))
			return nullptr;
	}

	printf("CARTRIDGE INFO\n"
	       "NAME: %s\n"
	       "ROM SIZE: %zu\n"
	       "RAM SIZE: %zu\n"
	       "ROM BANKS: %d\n"
	       "RAM BANKS: %d\n"
	       "TYPE CODE: %u\n"
	       "SYSTEM CODE: %u\n",
	       Cart::info.internal_name,
	       rom_size, ram_size,
	       rom_banks, ram_banks,
	       static_cast<unsigned>(type),
	       static_cast<unsigned>(system));

	reset(gb);
	gb_guard.Abort();
	return gb;
}


void destroy_gameboy(owner<Gameboy* const> gb)
{
	assert(gb != nullptr);

	if (owner<char* const> sav_file_path = Cart::info.sav_file_path) {
		update_sav_file(gb->cart, sav_file_path);
		free(sav_file_path);
		Cart::info.sav_file_path = nullptr;
	}

	free(gb);
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


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
	memset(Gpu::screen, 0xFF, sizeof(Gpu::screen));

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
	// TODO: pass instr_timing test, get rid of vsync hack
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

	if (test_bit(2, hwstate->tac)) {
		hwstate->tima_clock += cycles;
		if (hwstate->tima_clock >= hwstate->tima_clock_limit) {
			inc_tima(hwstate);
			hwstate->tima_clock -= hwstate->tima_clock_limit;
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


static bool eval_header_info(FILE* rom_file);
static bool eval_and_load_sav_file(const char* rom_file_path, Cart* cart);
static void update_sav_file(const Cart& cart);

owner<Gameboy*> create_gameboy(const char* const rom_file_path)
{
	const owner<FILE*> rom_file = fopen(rom_file_path, "rb");
	if (rom_file == nullptr) {
		perror("Couldn't open file");
		return nullptr;
	}

	const auto file_guard = finally([rom_file] {
		fclose(rom_file);
	});	

	if (!eval_header_info(rom_file))
		return nullptr;

	owner<Gameboy* const> gb = 
	  static_cast<Gameboy*>(malloc(sizeof(Gameboy) +
	                              Cart::info.rom_size + 
	                              Cart::info.ram_size));
	if (gb == nullptr) {
		perror("Couldn't allocate memory");
		return nullptr;
	}

	bool success = false;
	const auto gb_guard = finally([&success, gb] {
		if (!success)
			destroy_gameboy(gb);
	});

	fseek(rom_file, 0, SEEK_SET);
	const size_t rom_size = Cart::info.rom_size;
	if (fread(gb->cart.data, 1, rom_size, rom_file) < rom_size) {
		fprintf(stderr, "Error while reading from file\n");
		return nullptr;
	}

	constexpr const Cart::Type battery_cart_types[] {
		Cart::Type::RomMBC1RamBattery,
		Cart::Type::RomMBC2Battery
	};
	
	if (is_in_array(battery_cart_types, Cart::info.type)) {
		if (!eval_and_load_sav_file(rom_file_path, &gb->cart))
			return nullptr;
	}

	printf("CARTRIDGE INFO\n"
	       "NAME: %s\n"
	       "ROM SIZE: %d\n"
	       "RAM SIZE: %d\n"
	       "ROM BANKS: %d\n"
	       "RAM BANKS: %d\n"
	       "TYPE CODE: %u\n"
	       "SYSTEM CODE: %u\n",
	       Cart::info.internal_name,
	       Cart::info.rom_size, Cart::info.ram_size,
	       Cart::info.rom_banks, Cart::info.ram_banks,
	       static_cast<unsigned>(Cart::info.type),
	       static_cast<unsigned>(Cart::info.system));

	reset(gb);
	success = true;
	return gb;
}


void destroy_gameboy(const owner<Gameboy*> gb)
{
	assert(gb != nullptr);

	if (Cart::info.sav_file_path != nullptr) {
		update_sav_file(gb->cart);
		free(Cart::info.sav_file_path);
		Cart::info.sav_file_path = nullptr;
	}

	free(gb);
}



bool eval_header_info(FILE* const rom_file)
{
	uint8_t header[0x4F];
	fseek(rom_file, 0x100, SEEK_SET);
	if (fread(header, 1, 0x4F, rom_file) < 0x4F) {
		fprintf(stderr, "Error while reading from file\n");
		return false;
	}

	memcpy(Cart::info.internal_name, &header[0x34], 16);

	switch (header[0x43]) {
	case 0xC0: Cart::info.system = Cart::System::GameboyColorOnly; break;
	case 0x80: Cart::info.system = Cart::System::GameboyColorCompat; break;
	default: Cart::info.system = Cart::System::Gameboy; break;
	}

	Cart::info.type = static_cast<Cart::Type>(header[0x47]);

	if (!is_in_array(kSupportedCartridgeTypes, Cart::info.type)) {
		fprintf(stderr, "Cartridge type %u not supported\n",
		        static_cast<unsigned>(Cart::info.type));
		return false;
	} else if (!is_in_array(kSupportedCartridgeSystems, Cart::info.system)) {
		fprintf(stderr, "Cartridge system %u not supported\n",
		        static_cast<unsigned>(Cart::info.system));
		return false;
	}
	
	if (Cart::info.type >= Cart::Type::RomMBC1 &&
	     Cart::info.type <= Cart::Type::RomMBC1RamBattery) {
		Cart::info.short_type = Cart::ShortType::RomMBC1;
	} else if (Cart::info.type >= Cart::Type::RomMBC2 &&
	            Cart::info.type <= Cart::Type::RomMBC2Battery) {
		Cart::info.short_type = Cart::ShortType::RomMBC2;
	} else {
		Cart::info.short_type = Cart::ShortType::RomOnly;
	}

	struct SizeInfo { const int32_t size; const uint8_t banks; };
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
	
	Cart::info.rom_size = rom_sizes[size_codes[0]].size;
	Cart::info.rom_banks = rom_sizes[size_codes[0]].banks;
	Cart::info.ram_size = ram_sizes[size_codes[1]].size;
	Cart::info.ram_banks = ram_sizes[size_codes[1]].banks;

	if (Cart::info.short_type == Cart::ShortType::RomOnly) {
		if (Cart::info.ram_size != 0x00 || Cart::info.rom_size != 32_Kib) {
			fprintf(stderr, "Invalid size codes for RomOnly\n");
			return false;
		}
	} else if (Cart::info.short_type == Cart::ShortType::RomMBC2) {
		if (Cart::info.rom_size <= 256_Kib && Cart::info.ram_size == 0x00) {
			Cart::info.ram_size = 512;
			Cart::info.ram_banks = 1;
		} else {
			fprintf(stderr, "Invalid size codes for MBC2\n");
			return false;
		}
	}

	return true;
}

bool eval_and_load_sav_file(const char* const rom_file_path, Cart* const cart)
{
	const auto rom_path_size = strlen(rom_file_path);
	const auto sav_path_size = rom_path_size + 5;
	owner<char* const> sav_file_path = 
		static_cast<char*>(calloc(sav_path_size, sizeof(char)));

	if (sav_file_path == nullptr) {
		perror("Couldn't allocate memory");
		return false;
	}

	const size_t dot_offset = 
	[rom_file_path, rom_path_size]()-> size_t {
		const char* p = &rom_file_path[rom_path_size - 1];
		while (*p != '.' && p != &rom_file_path[0])
			--p;
		return (*p == '.')
			? p - &rom_file_path[0] : rom_path_size - 1;
	}();

	const auto size = sizeof(char) * dot_offset;
	memcpy(sav_file_path, rom_file_path, size);
	strcat(sav_file_path, ".sav");

	if (owner<FILE* const> sav_file = fopen(sav_file_path, "rb+")) {
		const auto sav_file_guard = finally([sav_file] {
			fclose(sav_file);
		});
		const size_t ram_size = Cart::info.ram_size;
		uint8_t* const ram = &cart->data[Cart::info.rom_size];
		if (fread(ram, 1, ram_size, sav_file) < ram_size)
			fprintf(stderr, "Error while loading sav file");
	} else if (errno != ENOENT) {
		perror("Couldn't open sav file");
		free(sav_file_path);
		return false;
	}

	Cart::info.sav_file_path = sav_file_path;
	return true;
}

void update_sav_file(const Cart& cart)
{
	owner<FILE* const> sav_file = fopen(Cart::info.sav_file_path, "wb");
	if (sav_file == nullptr) {
		perror("Couldn't open sav file");
		return;
	}
	
	const auto sav_file_guard = finally([sav_file] {
		fclose(sav_file);
	});

	const size_t ramsize = Cart::info.ram_size;
	const uint8_t* const ram = &cart.data[Cart::info.rom_size];
	if (fwrite(ram, 1, ramsize, sav_file) < ramsize)
		perror("Error while writting sav file");
}


} // namespace gbx


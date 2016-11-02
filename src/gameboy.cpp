#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "instructions.hpp"
#include "gameboy.hpp"

namespace gbx {


extern void update_gpu(int16_t cycles, const Memory& mem, HWState* hwstate, Gpu* gpu);
static void update_timers(int16_t cycles, HWState* hwstate);
static void update_interrupts(Gameboy* gb);
Cart::Info Cart::info;

void Gameboy::Reset()
{
	memset(this, 0, sizeof(Gameboy));
	memset(Gpu::screen, 0xFF, sizeof(Gpu::screen));

	// init the system
	// up to now only Gameboy (DMG) mode is supported
	cpu.pc = 0x0100;
	cpu.sp = 0xFFFE;
	cpu.af = 0x01B0;
	cpu.bc = 0x0013;
	cpu.de = 0x00D8;
	cpu.hl = 0x014D;

	gpu.lcdc.value = 0x91;
	gpu.stat.value = 0x85;
	gpu.bgp = 0xFC;
	gpu.obp0 = 0xFF;
	gpu.obp1 = 0xFF;

	hwstate.tac = 0xF8;

	keys.value = 0xCF;
	keys.pad.value = 0xFF;
}


void Gameboy::Run(const int32_t cycles)
{
	do {
		const int32_t before = cpu.clock;
		update_interrupts(this);

		int16_t step_cycles;
		if (!hwstate.flags.cpu_halt) {
			const uint8_t opcode = Read8(cpu.pc++);
			main_instructions[opcode](this);
			step_cycles = clock_table[opcode];
		} else {
			step_cycles = 4;
		}

		const int32_t after = cpu.clock;
		cpu.clock += step_cycles;
		step_cycles += static_cast<int16_t>(after - before);
		update_gpu(step_cycles, memory, &hwstate, &gpu);
		update_timers(step_cycles, &hwstate);
	} while (cpu.clock <= cycles);
	cpu.clock -= cycles;
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
			if (++hwstate->tima == 0x00) {
				hwstate->tima = hwstate->tma;
				request_interrupt(interrupts::timer, hwstate);
			}
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
			gb->PushStack16(gb->cpu.pc);
			gb->cpu.pc = inter.addr;
			gb->cpu.clock += 20;
			break;
		}
	}
}



static owner<Gameboy*> allocate_gb(const char* rom_path);
static bool fill_cart_info(const uint8_t(&header)[0x4F]);

owner<Gameboy*> create_gameboy(const char* const rom_path)
{
	if (const owner<Gameboy*> gb = allocate_gb(rom_path)) {
		gb->Reset();
		return gb;
	}
	return nullptr;
}

void destroy_gameboy(const owner<Gameboy*> gb)
{
	assert(gb != nullptr);
	free(gb);
}

// allocate gameboy struct on the heap 
// with size (size of Gameboy + size of ROM + size of cartridge RAM)
owner<Gameboy*> allocate_gb(const char* const rom_path)
{
	const owner<FILE*> file = fopen(rom_path, "rb");
	if (file == nullptr) {
		perror("Couldn't open file");
		return nullptr;
	}

	const auto file_guard = finally([=] {
		fclose(file);
	});
	
	{
		uint8_t header[0x4F];
		fseek(file, 0x100, SEEK_SET);
		if (fread(header, 1, 0x4F, file) < 0x4F) {
			fprintf(stderr, "Error while reading from file.\n");
			return nullptr;
		} else if (!fill_cart_info(header)) {
			return nullptr;
		}
	}

	owner<Gameboy* const> gb = 
	  reinterpret_cast<Gameboy*>(malloc(sizeof(Gameboy) +
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

	fseek(file, 0, SEEK_SET);
	const size_t rom_size = Cart::info.rom_size;
	const size_t bytes_read = fread(gb->cart.data, 1, rom_size, file);
	if (bytes_read < rom_size) {
		fprintf(stderr, "Error while reading from file.\n");
		return nullptr;
	}

	success = true;
	return gb;
}


bool fill_cart_info(const uint8_t(&header)[0x4F])
{
	auto& cinfo = Cart::info;
	memcpy(cinfo.internal_name, &header[0x34], 16);
	cinfo.internal_name[16] = '\0';

	switch (header[0x43]) {
	case 0xC0: cinfo.system = Cart::System::GameboyColorOnly; break;
	case 0x80: cinfo.system = Cart::System::GameboyColorCompat; break;
	default: cinfo.system = Cart::System::Gameboy; break;
	}

	cinfo.type = static_cast<Cart::Type>(header[0x47]);

	const auto is_supported_type = [](const Cart::Type type) {
		for (const auto supported_type : kSupportedCartridgeTypes)
			if (supported_type == type)
				return true;
		return false;
	};
	const auto is_supported_system = [](const Cart::System system) {
		for (const auto supported_system : kSupportedCartridgeSystems)
			if (supported_system == system)
				return true;
		return false;
	};

	if (!is_supported_type(cinfo.type)) {
		fprintf(stderr, "Cartridge type %u not supported.\n",
		        static_cast<unsigned>(cinfo.type));
		return false;
	} else if (!is_supported_system(cinfo.system)) {
		fprintf(stderr, "Cartridge system %u not supported.\n",
		        static_cast<unsigned>(cinfo.system));
		return false;
	}
	
	if (cinfo.type >= Cart::Type::RomMBC1 &&
	     cinfo.type <= Cart::Type::RomMBC1RamBattery) {
		cinfo.short_type = Cart::ShortType::RomMBC1;
	} else if (cinfo.type >= Cart::Type::RomMBC2 &&
	            cinfo.type <= Cart::Type::RomMBC2Battery) {
		cinfo.short_type = Cart::ShortType::RomMBC2;
	} else {
		cinfo.short_type = Cart::ShortType::RomOnly;
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
		fprintf(stderr, "Invalid size codes.\n");
		return false;
	}
	
	cinfo.rom_size = rom_sizes[size_codes[0]].size;
	cinfo.rom_banks = rom_sizes[size_codes[0]].banks;
	cinfo.ram_size = ram_sizes[size_codes[1]].size;
	cinfo.ram_banks = ram_sizes[size_codes[1]].banks;

	if (cinfo.short_type == Cart::ShortType::RomOnly) {
		if (cinfo.ram_size != 0x00 || cinfo.rom_size != 32_Kib) {
			fprintf(stderr, "invalid size codes for RomOnly!\n");
			return false;
		}
	} else if (cinfo.short_type == Cart::ShortType::RomMBC2) {
		if (cinfo.rom_size <= 256_Kib && cinfo.ram_size == 0x00) {
			cinfo.ram_size = 512;
			cinfo.ram_banks = 1;
		} else {
			fprintf(stderr, "invalid size codes for MBC2!\n");
			return false;
		}
	}

	printf("CARTRIDGE INFO\n"
	       "NAME: %s\n"
	       "ROM SIZE: %d\n"
	       "RAM SIZE: %d\n"
	       "ROM BANKS: %d\n"
	       "RAM BANKS: %d\n"
	       "TYPE CODE: %u\n"
	       "SYSTEM CODE: %u\n",
	       cinfo.internal_name,
	       cinfo.rom_size, cinfo.ram_size,
	       cinfo.rom_banks, cinfo.ram_banks,
	       static_cast<unsigned>(cinfo.type),
	       static_cast<unsigned>(cinfo.system));

	return true;
}



} // namespace gbx


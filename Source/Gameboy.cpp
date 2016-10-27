#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Instructions.hpp"
#include "Gameboy.hpp"

namespace gbx {


extern void update_gpu(int16_t cycles, const Memory& mem, HWState* hwstate, Gpu* gpu);
static void update_timers(int16_t cycles, HWState* hwstate);
static void update_interrupts(Gameboy* gb);
static int16_t step_machine(Gameboy* gb);
Cart::Info Cart::info;

void Gameboy::Reset()
{
	memset(this, 0, sizeof(Gameboy));
	memset(Gpu::screen, 0xFF, sizeof(Gpu::screen));

	// init the system
	// up to now only Gameboy mode is supported
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

	// addresses and inital values for hardware registers
	// Write8(0xFF05, 0x00); TIMA, in HWState
	// Write8(0xFF06, 0x00); TMA, in HWState
	// Write8(0xFF07, 0x00); TAC, in HWState
	// Write8(0xFF10, 0x80); NR10
	// Write8(0xFF11, 0xBF); NR11
	// Write8(0xFF12, 0xF3); NR12
	// Write8(0xFF14, 0xBF); NR14
	// Write8(0xFF16, 0x3F); NR21
	// Write8(0xFF17, 0x00); NR22
	// Write8(0xFF19, 0xBF); NR24
	// Write8(0xFF1A, 0x7F); NR30
	// Write8(0xFF1B, 0xFF); NR31
	// Write8(0xFF1C, 0x9F); NR32
	// Write8(0xFF1E, 0xBF); NR33
	// Write8(0xFF20, 0xFF); NR41
	// Write8(0xFF21, 0x00); NR42
	// Write8(0xFF22, 0x00); NR43
	// Write8(0xFF23, 0xBF); NR30
	// Write8(0xFF24, 0x77); NR50
	// Write8(0xFF25, 0xF3); NR51
	// Write8(0xFF26, 0xF1); NR52
	// Write8(0xFF40, 0x91); LCDC, in GPU
	// Write8(0xFF42, 0x00); SCY, in GPU
	// Write8(0xFF43, 0x00); SCX, in GPU
	// Write8(0xFF45, 0x00); LYC, in GPU
	// Write8(0xFF47, 0xFC); BGP, in GPU
	// Write8(0xFF48, 0xFF); OBP0, in GPU
	// Write8(0xFF49, 0xFF); OBP1, in GPU
	// Write8(0xFF4A, 0x00); WY, in GPU
	// Write8(0xFF4B, 0x00); WX, in GPU
}


void Gameboy::Run(const int32_t cycles)
{
	do {
		const auto step_cycles = step_machine(this);
		update_gpu(step_cycles, memory, &hwstate, &gpu);
		update_timers(step_cycles, &hwstate);
		update_interrupts(this);
	} while (cpu.clock < cycles);
	cpu.clock = 0;
}


int16_t step_machine(Gameboy* const gb)
{
	if (!get_flags(gb->hwstate, HWState::CpuHalt)) {
		const int32_t before = gb->cpu.clock;
		const uint8_t opcode = gb->Read8(gb->cpu.pc++);
		main_instructions[opcode](gb);
		const uint8_t step_cycles = clock_table[opcode];
		const int32_t after = gb->cpu.clock;
		gb->cpu.clock += step_cycles;
		const auto add_cycles = static_cast<int16_t>(after - before);
		return step_cycles + add_cycles;
	}
	gb->cpu.clock += 4;
	return 4;
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
	auto& hwstate = gb->hwstate;
	const uint8_t pendents = get_pendent_interrupts(hwstate);
	const auto flags = hwstate.flags;

	if (pendents && (flags & HWState::CpuHalt))
		clear_flags(HWState::CpuHalt, &hwstate);

	if (!(flags & HWState::IntMasterEnable)) {
		return;
	} else if (!(flags & HWState::IntMasterActive)) {
		set_flags(HWState::IntMasterActive, &hwstate);
		return;
	}

	if (!pendents)
		return;

	clear_flags(HWState::IntMasterEnable | HWState::IntMasterActive,
	            &gb->hwstate);
	
	for (const auto inter : interrupts::array) {
		if (pendents & inter.mask) {
			clear_interrupt(inter, &hwstate);
			gb->PushStack16(gb->cpu.pc);
			gb->cpu.pc = inter.addr;
			gb->cpu.clock += 16;
			update_gpu(16, gb->memory, &gb->hwstate, &gb->gpu);
			update_timers(16, &gb->hwstate);
		}
	}
}



static owner<Gameboy*> allocate_gb(const char* rom_path);
static bool fill_cart_info(FILE* const cart);

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

	if (!fill_cart_info(file))
		return nullptr;

	const owner<Gameboy*> gb = 
		reinterpret_cast<Gameboy*>(malloc(sizeof(Gameboy) +
		                           Cart::info.rom_size + 
		                           Cart::info.ram_size));
	if (gb == nullptr) {
		perror("failed to allocate memory");
		return nullptr;
	}

	bool success = false;
	const auto gb_guard = finally([&success, gb] {
		if (!success)
			destroy_gameboy(gb);
	});

	fseek(file, 0, SEEK_SET);
	fread(gb->cart.data, 1, Cart::info.rom_size, file);

	if (ferror(file)) {
		perror("error while reading from file");
		return nullptr;
	}

	success = true;
	return gb;
}


bool fill_cart_info(FILE* const file)
{
	const auto read_buff = [=](long int offset, size_t size, void* buffer) {
		fseek(file, offset, SEEK_SET);
		fread(buffer, 1, size, file);
	};
	const auto read_byte = [=](long int offset) {
		fseek(file, offset, SEEK_SET);
		return static_cast<uint8_t>(fgetc(file));
	};

	auto& cinfo = Cart::info;

	// 0134 - 0143 game's title
	read_buff(0x134, 16, cinfo.internal_name);
	cinfo.internal_name[16] = '\0';

	switch (read_byte(0x143)) {
	case 0xC0: cinfo.system = Cart::System::GameboyColorOnly; break;
	case 0x80: cinfo.system = Cart::System::GameboyColorCompat; break;
	default: cinfo.system = Cart::System::Gameboy; break;
	}

	cinfo.type = static_cast<Cart::Type>(read_byte(0x147));

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

	uint8_t size_codes[2];
	read_buff(0x148, 0x02, size_codes);

	switch (size_codes[0]) {
	case 0x00: cinfo.rom_size = 32_Kib; break;  // 2 banks
	case 0x01: cinfo.rom_size = 64_Kib; break;  // 4 banks
	case 0x02: cinfo.rom_size = 128_Kib; break; // 8 banks
	case 0x03: cinfo.rom_size = 256_Kib; break; // 16 banks
	case 0x04: cinfo.rom_size = 512_Kib; break; // 32 banks
	case 0x05: cinfo.rom_size = 1_Mib; break;   // 64 banks
	case 0x06: cinfo.rom_size = 2_Mib; break;   // 128 banks
	default:
		fprintf(stderr,"Couldn't eval ROM information\n");
		return false;
	}

	switch (size_codes[1]) {
	case 0x00: cinfo.ram_size = 0x00; break;   // NO RAM
	case 0x01: cinfo.ram_size = 2_Kib; break;  // 1/4 bank
	case 0x02: cinfo.ram_size = 8_Kib; break;  // 1 bank
	case 0x03: cinfo.ram_size = 32_Kib; break; // 4 banks
	default:
		fprintf(stderr, "Couldn't eval RAM information\n");
		return false;
	}

	cinfo.rom_banks =
	  static_cast<int16_t>(static_cast<uint32_t>(cinfo.rom_size) >> 0x0E);

	if (cinfo.short_type == Cart::ShortType::RomMBC1 && cinfo.ram_size) {
		cinfo.ram_banks = cinfo.ram_size == 2_Kib ? 1
		 : static_cast<int8_t>
		    (static_cast<uint32_t>(cinfo.ram_size) >> 0x0D);
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


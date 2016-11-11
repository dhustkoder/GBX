#ifndef GBX_GAMEBOY_HPP_
#define GBX_GAMEBOY_HPP_
#include "cpu.hpp"
#include "gpu.hpp"
#include "cart.hpp"
#include "joypad.hpp"
#include "hwstate.hpp"
#include "memory.hpp"

namespace gbx {

struct Gameboy
{
	Gameboy()=delete;
	Gameboy(Gameboy&)=delete;
	Gameboy(Gameboy&&)=delete;
	~Gameboy()=delete;
	Gameboy&operator=(Gameboy&)=delete;
	Gameboy&operator=(Gameboy&&)=delete;

	Joypad joypad;
	HWState hwstate;
	Gpu gpu;
	Cpu cpu;
	Memory memory;
	Cart cart;
};

extern owner<Gameboy*> create_gameboy(const char* rom_file_path);
extern void destroy_gameboy(owner<Gameboy*> gb);
extern void reset(Gameboy* gb);
extern void run_for(int32_t clock_limit, Gameboy* gb);

inline void stack_push8(const uint8_t value, Gameboy* const gb)
{
	mem_write8(--gb->cpu.sp, value, gb);
}


inline void stack_push16(const uint16_t value, Gameboy* const gb)
{
	gb->cpu.sp -= 2;
	mem_write16(gb->cpu.sp, value, gb);
}


inline uint8_t stack_pop8(Gameboy* const gb)
{
	return mem_read8(*gb, gb->cpu.sp++);
}


inline uint16_t stack_pop16(Gameboy* const gb)
{
	const uint16_t val = mem_read16(*gb, gb->cpu.sp);
	gb->cpu.sp += 2;
	return val;
}



} // namespace gbx
#endif

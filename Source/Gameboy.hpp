#ifndef GBX_GAMEBOY_HPP_
#define GBX_GAMEBOY_HPP_
#include "Cpu.hpp"
#include "Gpu.hpp"
#include "Keys.hpp"
#include "HWState.hpp"
#include "Memory.hpp"

namespace gbx {

struct Gameboy 
{
	Gameboy()=delete;
	Gameboy(Gameboy&)=delete;
	Gameboy(Gameboy&&)=delete;
	~Gameboy()=delete;
	Gameboy&operator=(Gameboy&)=delete;
	Gameboy&operator=(Gameboy&&)=delete;

	void Reset();
	void Run(uint32_t cycles);

	uint8_t Read8(uint16_t address) const;
	uint16_t Read16(uint16_t address) const;
	void Write8(uint16_t address, uint8_t value);
	void Write16(uint16_t address, uint16_t value);

	void PushStack8(uint8_t value);
	void PushStack16(uint16_t value);
	uint8_t PopStack8();
	uint16_t PopStack16();

	Cpu cpu;
	Gpu gpu;
	Keys keys;
	HWState hwstate;
	Memory memory;
	Cartridge cart;
};


extern owner<Gameboy*> create_gameboy(const char* rom_path);
extern void destroy_gameboy(owner<Gameboy*> gb);


} // namespace gbx
#endif

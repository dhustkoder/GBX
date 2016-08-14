#ifndef GBX_GAMEBOY_HPP_
#define GBX_GAMEBOY_HPP_
#include "CPU.hpp"
#include "GPU.hpp"
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


	bool LoadRom(const char* file);
	bool Reset();
	void Run(const uint32_t cycles);
	uint8_t Step();
	void UpdateGPU(const uint8_t cycles);
	void UpdateHWState(const uint8_t cycles);
	void UpdateInterrupts();

	int8_t ReadS8(const uint16_t address) const;
	uint8_t ReadU8(const uint16_t address) const;
	uint16_t ReadU16(const uint16_t address) const;
	void WriteU8(const uint16_t address, const uint8_t value);
	void WriteU16(const uint16_t address, const uint16_t value);

	void PushStack8(const uint8_t value);
	void PushStack16(const uint16_t value);
	uint8_t PopStack8();
	uint16_t PopStack16();


	CPU cpu;
	GPU gpu;
	Keys keys;
	HWState hwstate;
	Memory memory;
};


extern Gameboy* create_gameboy();
extern void destroy_gameboy(Gameboy* const);






}
#endif

#ifndef GBX_GAMEBOY_HPP_
#define GBX_GAMEBOY_HPP_
#include "CPU.hpp"
#include "Memory.hpp"

namespace gbx {


struct Interrupts
{
	uint8_t master;
	uint8_t enable;
	uint8_t flags;
};






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
	Interrupts interrupts;
	Memory memory;
};


extern Gameboy* create_gameboy();
extern void destroy_gameboy(Gameboy* const);








}
#endif

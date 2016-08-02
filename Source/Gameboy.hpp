#ifndef GBX_GAMEBOY_HPP_
#define GBX_GAMEBOY_HPP_
#include "CPU.hpp"
#include "Memory.hpp"

namespace gbx {

enum InterruptFlags : uint8_t 
{
	INTERRUPT_VBLANK = 0x01, INTERRUPT_LCDC = 0x02,
	INTERRUPT_TIMER = 0x04, INTERRUPT_SERIAL = 0x08,
	INTERRUPT_JOYPAD = 0x10
};


struct States
{
	enum Flags : uint8_t
	{
		INTERRUPT_MASTER_ENABLED = 0x01,
		INTERRUPT_MASTER_ACTIVE = 0x02
	};

	uint8_t flags;
	uint8_t interrupt_enable;
	uint8_t interrupt_flags;
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
	bool Step();


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
	States states;
	Memory memory;
};


extern Gameboy* create_gameboy();
extern void destroy_gameboy(Gameboy* const);








}
#endif

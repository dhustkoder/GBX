#ifndef GBX_GAMEBOY_HPP_
#define GBX_GAMEBOY_HPP_
#include "CPU.hpp"
#include "Memory.hpp"

namespace gbx {

enum Interrupts : uint8_t 
{
	INTERRUPT_VBLANK = 0x01, INTERRUPT_LCDC = 0x02,
	INTERRUPT_TIMER = 0x04, INTERRUPT_SERIAL = 0x08,
	INTERRUPT_JOYPAD = 0x10
};


struct HWState
{
	enum HWFlags : uint8_t
	{
		INTERRUPT_MASTER_ENABLED = 0x01,
		INTERRUPT_MASTER_ACTIVE = 0x20
	};
	
	uint8_t hwflags;
	uint8_t interrupt_enable;
	uint8_t interrupt_flags;
};



struct LCD
{
	enum CONTROL : uint8_t 
	{
		LCD_CONTROL_OP = 0x80,
		WIN_TILE_MAP_SELECT = 0x40,
		WIN_ON_OFF = 0x20,
		BG_WIN_TILE_MAP_SELECT = 0x10,
		BG_TILE_MAP_SELECT = 0x08,
		OBJ_SIZE = 0x04,
		OBJ_ON_OFF = 0x02,
		BG_ON_OFF = 0x01
	};

	int16_t counter;
	uint8_t control;
	uint8_t scanline;
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
	void Step();
	void StepInterrupts();

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
	LCD lcd;
	HWState hwstate;
	Memory memory;
};


extern Gameboy* create_gameboy();
extern void destroy_gameboy(Gameboy* const);








}
#endif

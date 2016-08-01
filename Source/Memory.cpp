#include <stdio.h>
#include "Gameboy.hpp"
#include "Memory.hpp"

namespace gbx {




int8_t Gameboy::ReadS8(const uint16_t address) const {
	// TODO: this might not be totally portable 
	return static_cast<int8_t>(ReadU8(address));
}




uint8_t Gameboy::ReadU8(const uint16_t address) const {
	return memory[address];
}



uint16_t Gameboy::ReadU16(const uint16_t address) const {
	return ConcatBytes(memory[address + 1], memory[address]);
}



void Gameboy::WriteU8(const uint16_t address, const uint8_t value) {
	memory[address] = value;
}



void Gameboy::WriteU16(const uint16_t address, const uint16_t value) {
	Split16(value, &memory[address + 1], &memory[address]);
}





void Gameboy::PushStack8(const uint8_t value) 
{
	const uint16_t sp = cpu.GetSP() - 1;
	WriteU8(sp, value);
	cpu.SetSP(sp);
}


void Gameboy::PushStack16(const uint16_t value) 
{
	const uint16_t sp = cpu.GetSP() - 2;
	WriteU16(sp, value);
	cpu.SetSP(sp);
}



uint8_t Gameboy::PopStack8() 
{
	const uint16_t sp = cpu.GetSP();
	const uint8_t val = ReadU8(sp);
	cpu.SetSP(sp + 1);
	return val;
}




uint16_t Gameboy::PopStack16() 
{
	const uint16_t sp = cpu.GetSP();
	const uint16_t val = ReadU16(sp);
	cpu.SetSP(sp + 2);
	return val;
}









	

}

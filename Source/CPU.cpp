#include <stdio.h>
#include "Debug.hpp"
#include "CPU.hpp"


namespace gbx {







void CPU::PrintFlags() const 
{
	const auto f = GetF();
	const auto z = f & FLAG_Z ? 1 : 0;
	const auto n = f & FLAG_N ? 1 : 0;
	const auto h = f & FLAG_H ? 1 : 0;
	const auto c = f & FLAG_C ? 1 : 0;
	debug_printf("CPU FLAGS: Z(%x), N(%x), H(%x), C(%x)\n", z, n, h, c);
}





void CPU::PrintRegisters() const 
{
	debug_printf("PC: %4x\n"
	             "SP: %4x\n"
	             "AF: %4x\n"
	             "BC: %4x\n"
	             "DE: %4x\n"
	             "HL: %4x\n", pc, sp,
	             af.pair, bc.pair, de.pair, hl.pair);
}








uint8_t CPU::RR(const uint8_t value)
{
	// flags effect: Z 0 0 C
	const uint8_t old_carry = GetFlags(FLAG_C);
	const uint8_t old_bit0 = value & 0x01;
	const uint8_t result = old_carry ? ((value>>1) | 0x80) : (value>>1);
	if (old_bit0)
		SetF(CheckZ(result) | FLAG_C);
	else
		SetF(CheckZ(result));

	return result;
}


uint8_t CPU::RRC(const uint8_t value)
{
	// flags effect: Z 0 0 C
	const uint8_t old_bit0 = (value & 0x01);
	uint8_t result;
	if (old_bit0) {
		result = (value >> 1) | 0x80;
		SetF(CheckZ(result) | FLAG_C);
	} else {
		result = value >> 1;
		SetF(CheckZ(result));
	}

	return result;
}



uint8_t CPU::RL(const uint8_t value)
{
	// flags effect: Z 0 0 C
	const uint8_t old_bit7 = value & 0x80;
	const uint8_t old_carry = GetFlags(FLAG_C);
	const uint8_t result = old_carry ? (value << 1) | 0x01 : (value << 1);
	if (old_bit7)
		SetF(CheckZ(result) | FLAG_C);
	else
		SetF(CheckZ(result));

	return result;
}


uint8_t CPU::RLC(const uint8_t value)
{
	// flags effect: Z 0 0 C
	const uint8_t old_bit7 = value & 0x80;
	uint8_t result;
	if (old_bit7) {
		result = (value << 1) | 0x01;
		SetF(CheckZ(result) | FLAG_C);
	} else {
		result = value << 1;
		SetF(CheckZ(result));
	}

	return result;
}





uint8_t CPU::SLA(const uint8_t value)
{
	// flags  effect: Z 0 0 C
	const uint8_t old_bit7 = value & 0x80;
	const uint8_t result = value << 1;
	if (old_bit7)
		SetF(CheckZ(result) | FLAG_C);
	else
		SetF(CheckZ(result));

	return result;
}



uint8_t CPU::SRL(const uint8_t value)
{
	// flags effect: Z 0 0 C
	const uint8_t old_bit0 = value & 0x01;
	const uint8_t result = value >> 1;
	if (old_bit0)
		SetF(CheckZ(result) | FLAG_C);
	else
		SetF(CheckZ(result));

	return result;
}


uint8_t CPU::SRA(const uint8_t value)
{
	// flags effect: Z 0 0 C
	const uint8_t old_bit0 = value & 0x01;
	const uint8_t result = (value & 0x80) | (value >> 1);
	if (old_bit0)
		SetF(CheckZ(result) | FLAG_C);
	else
		SetF(CheckZ(result));

	return result;
}


uint8_t CPU::SWAP(const uint8_t value) 
{
	// flags effect: Z 0 0 0
	const uint8_t result = ((value & 0x0f) << 4) | ((value & 0xf0) >> 4);
	SetF(CheckZ(result));
	return result;
}





















}

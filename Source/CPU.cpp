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





void CPU::ADD_HL(const uint16_t second) 
{
	// flags effect: - 0 H C
	const auto first = GetHL();
	const uint32_t result = first + second;
	const Flags hc = CheckH_bit11(first, second) |
	                 CheckC_bit15(result);
	SetF(GetFlags(FLAG_Z) | hc);
	SetHL(static_cast<uint16_t>(result));
}






uint8_t CPU::ADC(const uint8_t first, const uint8_t second) 
{
	// flags effect Z 0 H C
	const uint8_t carry = GetFlags(FLAG_C) ? 1 : 0;
	const uint16_t sec_n_carry = second + carry;
	const uint16_t result = first + sec_n_carry;
	uint8_t flags = 0x00;

	if (CheckC_bit7(result))
		flags = FLAG_C;
	if (CheckZ(result & 0xFF))
		flags |= FLAG_Z;
	if (((first&0xf)+(second&0xf)+carry) > 0x0f)
		flags |= FLAG_H;

	SetF(flags);
	return static_cast<uint8_t>(result);
}






uint8_t CPU::SBC(const uint8_t first, const uint8_t second)
{
	// flags effect: Z 1 H C
	const uint16_t sec_n_carry = GetFlags(FLAG_C) ? second + 1 : second;
	const uint32_t result = first - sec_n_carry;
	uint8_t flags = FLAG_N;

	if (CheckC_borrow(first, sec_n_carry))
		flags |= FLAG_C;
	if (CheckZ(result & 0xFF))
		flags |= FLAG_Z;
	if (((result ^ second ^ first) & 0x10) == 0x10)
		flags |= FLAG_H;

	SetF(flags);
	return static_cast<uint8_t>(result);
}








uint8_t CPU::ADD(const uint8_t first, const uint8_t second) 
{
	// flags effect Z 0 H C
	const uint16_t result = first + second;
	uint8_t zhc = 0x00;

	if (CheckZ(result&0xff))
		zhc = FLAG_Z; 
	if (CheckH_bit3(first, second))
		zhc |= FLAG_H;
	if (CheckC_bit7(result))
		zhc |= FLAG_C;

	SetF(zhc);
	return static_cast<uint8_t>(result);
}









uint8_t CPU::SUB(const uint8_t first, const uint8_t second) 
{
	// flags effect: Z 1 H C
	const uint16_t result = first - second;
	uint8_t zhc = FLAG_N;
	
	if (CheckZ(result&0xff))
		zhc |= FLAG_Z;
	if (CheckH_borrow(first, second))
		zhc |= FLAG_H;
	if (CheckC_borrow(first, second))
		zhc |= FLAG_C;
	
	SetF(zhc);
	return static_cast<uint8_t>(result);
}








uint8_t CPU::INC(const uint8_t first) 
{
	// flags effect: Z 0 H -
	const uint8_t result = first + 1;
	uint8_t zh = 0x00;

	if (CheckZ(result))
		zh = FLAG_Z;
	if (CheckH_bit3(first, 1))
		zh |= FLAG_H;

	SetF(zh | GetFlags(FLAG_C));
	return result;
}








uint8_t CPU::DEC(const uint8_t first) 
{
	// flags effect: Z 1 H -
	const uint8_t result = first - 1;
	uint8_t flags = FLAG_N | GetFlags(FLAG_C);

	if (CheckZ(result))
		flags |= FLAG_Z;
	if (CheckH_borrow(first, 1))
		flags |= FLAG_H;

	SetF(flags);
	return result;
}



void CPU::CP(const uint8_t value)
{
	// flags effect: Z 1 H C
	SUB(GetA(), value);
}




uint8_t CPU::OR(const uint8_t first, const uint8_t second) 
{
	// flags effect: Z 0 0 0
	const uint8_t result = first | second;
	SetF(CheckZ(result));
	return result;
}








uint8_t CPU::AND(const uint8_t first, const uint8_t second) 
{
	// flags effect: Z 0 1 0
	const uint8_t result = first & second;
	SetF(CheckZ(result) | FLAG_H);
	return result;
}








uint8_t CPU::XOR(const uint8_t first, const uint8_t second) 
{
	// flags effect: Z 0 0 0
	const uint8_t result = first ^ second;
	SetF(CheckZ(result));
	return result;
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
	}
	else {
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
	}
	else {
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





void CPU::BIT(const uint8_t bit, const uint8_t value)
{
	// flags effect: Z 0 1 -
	const Flags z = CheckZ(value & (0x01 << bit));
	SetF(z | FLAG_H | GetFlags(FLAG_C));
}





















}

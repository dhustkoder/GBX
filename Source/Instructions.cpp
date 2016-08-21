#include <stdio.h>
#include "Gameboy.hpp"
#include "Instructions.hpp"


namespace gbx {



/* common names:
 * d8:  immediate 8 bit value
 * d16: immediate 16 bit value
 * a8:  8 bit unsigned data, which are added to $FF00 in certain instructions (replacement for missing IN and OUT instructions)
 * a16: 16 bit address
 * r8: means 8 bit signed data, which are added to program counter
 * n: 8 bit data
 * nn: 16 bit data
 */

/* NOTE: 
 * IF the instruction modifies the FLAGS state, use CPU methods to calculate if available,
 * then store the return result in the target register/variable
 */

inline uint8_t get_d8(Gameboy* const gb)
{
	const uint8_t d8 = gb->ReadU8(gb->cpu.pc++);
	return d8;
}


inline uint16_t get_a8(Gameboy* const gb)
{
	return 0xFF00 + get_d8(gb);
}


inline int8_t get_r8(Gameboy* const gb)
{
	const int8_t r8 = gb->ReadS8(gb->cpu.pc++);
	return r8;
}


inline uint16_t get_d16(Gameboy* const gb)
{
	const uint16_t d16 = gb->ReadU16(gb->cpu.pc);
	gb->cpu.pc += 2;
	return d16;
}


inline uint16_t get_a16(Gameboy* const gb)
{
	return get_d16(gb);
}


inline void jr_nn(const bool cond, Gameboy* const gb)
{
	if (cond) {
		const int8_t r8 = get_r8(gb);
		gb->cpu.pc += r8;
		gb->cpu.clock += 4;
	} else {
		++gb->cpu.pc;
	}
}



inline void jp_nn(const bool cond, Gameboy* const gb)
{
	if (cond) {
		gb->cpu.pc = gb->ReadU16(gb->cpu.pc);
		gb->cpu.clock += 4;
	} else {
		gb->cpu.pc += 2;
	}
}



inline void ret_nn(const bool cond, Gameboy* const gb)
{
	if (cond) {
		gb->cpu.pc = gb->PopStack16();
		gb->cpu.clock += 12;
	}
}

inline void call_nn(const bool cond, Gameboy* const gb)
{
	if (cond) {
		const uint16_t addr = get_a16(gb);
		gb->PushStack16(gb->cpu.pc);
		gb->cpu.pc = addr;
	} else {
		gb->cpu.pc += 2;
	}
}


inline void rst_nn(const uint16_t addr, Gameboy* const gb)
{
	gb->PushStack16(gb->cpu.pc);
	gb->cpu.pc = addr;
}


inline void add_hl_nn(const uint16_t second, CPU* const cpu)
{
	// flags effect: - 0 H C
	const uint16_t first = cpu->GetHL();
	const uint32_t result = first + second;
	uint8_t hc = 0x00;

	if (CheckC_bit15(result))
		hc = CPU::FLAG_C;
	if ((result ^ first ^ second) & 0x1000)
		hc |= CPU::FLAG_H;

	cpu->SetF(cpu->GetFlags(CPU::FLAG_Z) | hc);
	cpu->SetHL(static_cast<uint16_t>(result));
}


inline void cp_n(const uint8_t value, CPU* const cpu)
{
	// flags effect: Z 1 H C
	cpu->SUB(cpu->GetA(), value);
}



inline void add_a_n(const uint8_t second, CPU* const cpu) {
	cpu->af.ind.a = cpu->ADD(cpu->af.ind.a, second);
}

inline void sub_a_n(const uint8_t second, CPU* const cpu) {
	cpu->af.ind.a = cpu->SUB(cpu->af.ind.a, second);
}

inline void adc_a_n(const uint8_t second, CPU* const cpu) {
	cpu->af.ind.a = cpu->ADC(cpu->af.ind.a, second);
}

inline void sbc_a_n(const uint8_t second, CPU* const cpu) {
	cpu->af.ind.a = cpu->SBC(cpu->af.ind.a, second);
}

inline void and_a_n(const uint8_t second, CPU* const cpu) {
	cpu->af.ind.a = cpu->AND(second, cpu->af.ind.a);
}

inline void xor_a_n(const uint8_t second, CPU* const cpu) {
	cpu->af.ind.a = cpu->XOR(second, cpu->af.ind.a);
}

inline void or_a_n(const uint8_t second, CPU* const cpu) {
	cpu->af.ind.a = cpu->OR(second, cpu->af.ind.a);
}




// Main instructions implementation:
// 0x00
void nop_00(Gameboy* const)
{

}



void ld_01(Gameboy* const gb) 
{
	// LD BC, d16
	gb->cpu.SetBC(get_d16(gb));
}



void ld_02(Gameboy* const gb) 
{
	// LD (BC), A
	gb->WriteU8(gb->cpu.GetBC(), gb->cpu.GetA());
}



void inc_03(Gameboy* const gb) 
{
	// INC BC
	gb->cpu.SetBC( gb->cpu.GetBC() + 1 );	
}



void inc_04(Gameboy* const gb) 
{ 
	// INC B ( Z 0 H - )
	const uint8_t b = gb->cpu.GetB();
	gb->cpu.SetB( gb->cpu.INC(b) );
}



void dec_05(Gameboy* const gb) 
{
	// DEC B ( Z 1 H - )
	const uint8_t b = gb->cpu.GetB();
	gb->cpu.SetB( gb->cpu.DEC(b) );
}




void ld_06(Gameboy* const gb) 
{
	// LD B, d8
	gb->cpu.SetB(get_d8(gb));
}




void rlca_07(Gameboy* const gb)
{
	// RLCA  ( 0 0 0 C )
	const uint8_t a = gb->cpu.GetA();
	const uint8_t old_bit7 = a & 0x80;
	const uint8_t result = a << 1;
	if (old_bit7) {
		gb->cpu.SetA(result | 0x01);
		gb->cpu.SetF(CPU::FLAG_C);
	}
	else {
		gb->cpu.SetA(result);
		gb->cpu.SetF(0x00);
	}
}




void ld_08(Gameboy* const gb)
{
	// LD (a16), SP
	const uint16_t a16 = get_a16(gb);
	gb->WriteU16(a16, gb->cpu.sp);
}



void add_09(Gameboy* const gb) 
{
	// ADD HL, BC
	add_hl_nn(gb->cpu.GetBC(), &gb->cpu);
}



void ld_0A(Gameboy* const gb)
{ 
	// LD A, (BC)
	const uint16_t bc = gb->cpu.GetBC();
	gb->cpu.SetA(gb->ReadU8(bc));
}





void dec_0B(Gameboy* const gb) 
{
	// DEC BC
	--gb->cpu.bc.pair;
}




void inc_0C(Gameboy* const gb) 
{ 
	// INC C ( Z 0 H - )
	const uint8_t c = gb->cpu.GetC();
	gb->cpu.SetC(gb->cpu.INC(c));	
}



void dec_0D(Gameboy* const gb) 
{
	// DEC C ( Z 1 H - )
	const uint8_t c = gb->cpu.GetC();
	gb->cpu.SetC(gb->cpu.DEC(c));
}



void ld_0E(Gameboy* const gb) 
{ 
	// LD C, d8 ( loads immediate 8 bit value into C )
	gb->cpu.SetC(get_d8(gb));
}



void rrca_0F(Gameboy* const gb)
{
	// RRCA ( 0 0 0 C )
	const uint8_t a = gb->cpu.GetA();
	const uint8_t old_bit0 = a & 0x01;
	const uint8_t result = a >> 1;
	if (old_bit0) {
		gb->cpu.SetA(result | 0x80);
		gb->cpu.SetF(CPU::FLAG_C);
	}
	else {
		gb->cpu.SetA(result);
		gb->cpu.SetF(0x00);
	}
}






// 0x10
void stop_10(Gameboy* const gb) {  ++gb->cpu.pc; }



void ld_11(Gameboy* const gb) 
{
	// LD DE, d16 ( store immediate d16 value into DE )
	gb->cpu.SetDE(get_d16(gb));
}



void ld_12(Gameboy* const gb) 
{
	// LD (DE), A ( store A into mem address pointed by DE )
	gb->WriteU8(gb->cpu.GetDE(), gb->cpu.GetA());
}



void inc_13(Gameboy* const gb) 
{
	// INC DE
	++gb->cpu.de.pair;
}



void inc_14(Gameboy* const gb)
{
	// INC D ( Z 0 H - )
	const uint8_t result = gb->cpu.INC(gb->cpu.GetD());
	gb->cpu.SetD(result);
}

void dec_15(Gameboy* const gb)
{
	// DEC D ( Z 1 H - )
	gb->cpu.SetD(gb->cpu.DEC(gb->cpu.GetD()));
}



void ld_16(Gameboy* const gb) 
{
	// LD D, d8
	gb->cpu.SetD(get_d8(gb));
}



void rla_17(Gameboy* const gb)
{
	// RLA ( 0 0 0 C )
	const uint8_t a = gb->cpu.GetA();
	const uint8_t old_bit7 = a & 0x80;
	const uint8_t old_carry = gb->cpu.GetFlags(CPU::FLAG_C);
	const uint8_t result = a << 1;
	gb->cpu.SetA(old_carry ? (result | 0x01) : result);
	gb->cpu.SetF(old_bit7 ? CPU::FLAG_C : 0x00);
	
}



void jr_18(Gameboy* const gb) 
{
	// JR r8 ( Add r8 to pc and jump to it )
	const int8_t r8 = get_r8(gb);
	gb->cpu.pc += r8;
}




void add_19(Gameboy* const gb) 
{
	// ADD HL, DE ( - 0 H C )
	add_hl_nn(gb->cpu.GetDE(), &gb->cpu);
}





void ld_1A(Gameboy* const gb) 
{
	// LD A, (DE) ( store value in mem address pointed by DE into A )
	const uint16_t de = gb->cpu.GetDE();
	gb->cpu.SetA(gb->ReadU8(de));
}






void dec_1B(Gameboy* const gb)
{ 
	 // DEC DE
	--gb->cpu.de.pair;
}






void inc_1C(Gameboy* const gb) 
{
	// INC E ( Z 0 H - )
	const uint8_t e = gb->cpu.GetE();
	gb->cpu.SetE(gb->cpu.INC(e));
}





void dec_1D(Gameboy* const gb)
{ 
	// DEC E ( Z 1 H - )
	const uint8_t e = gb->cpu.GetE();
	gb->cpu.SetE(gb->cpu.DEC(e));
}





void ld_1E(Gameboy* const gb)
{
	// LD E, d8
	gb->cpu.SetE(get_d8(gb));
}



void rra_1F(Gameboy* const gb)
{
	// RRA
	const uint8_t a = gb->cpu.GetA();
	const uint8_t old_bit0 = a & 0x01;
	const uint8_t old_carry = gb->cpu.GetFlags(CPU::FLAG_C);
	const uint8_t result = old_carry ? (a >> 1) | 0x01 : (a >> 1);
	gb->cpu.SetF(old_bit0 ? CPU::FLAG_C : 0x00);
	gb->cpu.SetA(result);
}








// 0x20
void jr_20(Gameboy* const gb) 
{
	// JR NZ, r8 ( jump if Z flags is reset )
	// clock cycles: 12 if jumps, 8 if not
	jr_nn(gb->cpu.GetFlags(CPU::FLAG_Z) == 0, gb);
}




void ld_21(Gameboy* const gb) 
{
	// LD HL, d16 ( load immediate 16 bit value into HL )
	gb->cpu.SetHL(get_d16(gb));
}




void ld_22(Gameboy* const gb) 
{
	// LD (HL+), A
	// Put A into memory address HL. Increment HL
	gb->WriteU8(gb->cpu.hl.pair++, gb->cpu.GetA());
}




void inc_23(Gameboy* const gb) 
{
	// INC HL
	++gb->cpu.hl.pair;
}



void inc_24(Gameboy* const gb)
{
	// INC H ( Z 0 H - )
	const uint8_t result = gb->cpu.INC(gb->cpu.GetH());
	gb->cpu.SetH(result);
}


void dec_25(Gameboy* const gb) 
{ 
	// DEC H ( Z 1 H - )
	const uint8_t h = gb->cpu.GetH();
	gb->cpu.SetH(gb->cpu.DEC(h));
}



void ld_26(Gameboy* const gb)
{
	//  LD H, d8
	gb->cpu.SetH(get_d8(gb));
}






void daa_27(Gameboy* const gb)
{ 
	// DAA
	uint16_t bcd = gb->cpu.GetA();
	
	if (gb->cpu.GetFlags(CPU::FLAG_N)) {
		if (gb->cpu.GetFlags(CPU::FLAG_H))
			bcd = (bcd - 0x06) & 0xFF;
		if (gb->cpu.GetFlags(CPU::FLAG_C))
			bcd -= 0x60;
	} 
	else {
		if (gb->cpu.GetFlags(CPU::FLAG_H) || (bcd&0xF)>9) 
			bcd  += 0x06;
		if (gb->cpu.GetFlags(CPU::FLAG_C) || bcd > 0x9F)
			bcd += 0x60;
	}

	gb->cpu.SetA(static_cast<uint8_t>(bcd));
	const uint8_t carry_flag = bcd >= 100 ? CPU::FLAG_C : 0;
	gb->cpu.SetF(CheckZ(bcd&0xff) | gb->cpu.GetFlags(CPU::FLAG_N) | carry_flag);
}






void jr_28(Gameboy* const gb) 
{
	// JP Z, r8 ( jump if Z flag is set )
	// clock cycles: if jumps 12, else 8
	jr_nn(gb->cpu.GetFlags(CPU::FLAG_Z) != 0, gb);
}




void add_29(Gameboy* const gb)
{
	// ADD HL, HL
	add_hl_nn(gb->cpu.GetHL(), &gb->cpu);
}




void ld_2A(Gameboy* const gb) 
{
	// LD A, (HL+) 
	// store value in address pointed by HL into A, increment HL
	const uint8_t value = gb->ReadU8(gb->cpu.hl.pair++);
	gb->cpu.SetA(value);
}





void dec_2B(Gameboy* const gb) 
{
	// DEC HL
	--gb->cpu.hl.pair;
}




void inc_2C(Gameboy* const gb)
{
	// INC L ( Z 0 H - )
	const uint8_t l = gb->cpu.GetL();
	gb->cpu.SetL(gb->cpu.INC(l));
}



void dec_2D(Gameboy* const gb) 
{
	// DEC L ( Z 1 H - )
	const uint8_t l = gb->cpu.GetL();
	gb->cpu.SetL(gb->cpu.DEC(l));
}





void ld_2E(Gameboy* const gb)
{
	// LD L, d8
	gb->cpu.SetL(get_d8(gb));
}





void cpl_2F(Gameboy* const gb) 
{
	// CPL ( Complement A register, flip all bits )
	// flags affected: - 1 1 -
	const uint8_t a = gb->cpu.GetA();
	gb->cpu.SetA(~a);
	gb->cpu.SetFlags(CPU::FLAG_N | CPU::FLAG_H);
}







// 0x30
void jr_30(Gameboy* const gb) 
{
	// JR NC, r8 ( jump if C flag is reset )
	jr_nn(gb->cpu.GetFlags(CPU::FLAG_C) == 0, gb);
}



void ld_31(Gameboy* const gb) 
{
	// LD SP, d16
	gb->cpu.sp = get_d16(gb);
}



void ld_32(Gameboy* const gb) 
{
	// LD (HL-), A or LD (HLD), A or LDD (HL), A
	// store A into memory pointed by HL, Decrements HL
	const uint16_t hl = gb->cpu.GetHL();	
	gb->WriteU8(hl, gb->cpu.GetA());
	gb->cpu.SetHL(hl - 1);
}



void inc_33(Gameboy* const gb)
{
	// INC SP
	++gb->cpu.sp;
}



void inc_34(Gameboy* const gb) 
{
	// INC (HL) ( increment value pointed by hl in memory )
	const uint16_t hl = gb->cpu.GetHL();
	gb->WriteU8(hl, gb->cpu.INC(gb->ReadU8(hl)));
}




void dec_35(Gameboy* const gb)
{
	// DEC (HL) ( Z 1 H - )
	const uint16_t hl = gb->cpu.GetHL();
	const uint8_t value = gb->ReadU8(hl);
	gb->WriteU8(hl, gb->cpu.DEC(value));
}





void ld_36(Gameboy* const gb) 
{
	// LD (HL), d8 ( store d8 into mem address pointed by HL )
	gb->WriteU8(gb->cpu.GetHL(), get_d8(gb));
}






void scf_37(Gameboy* const gb)
{
	// SCF ( - 0 0 1 )
	gb->cpu.SetF(gb->cpu.GetFlags(CPU::FLAG_Z) | CPU::FLAG_C);
}






void jr_38(Gameboy* const gb) 
{
	// JR C, r8 ( jump if C flag is set )
	jr_nn(gb->cpu.GetFlags(CPU::FLAG_C) != 0, gb);
}





void add_39(Gameboy* const gb)
{
	// ADD HL, SP
	add_hl_nn(gb->cpu.sp, &gb->cpu);
}




void ld_3A(Gameboy* const gb)
{
	// LD A, (HL-) or LD A,(HLD) or LDD A,(HL)
	// load value in mem pointed by HL in A, decrement HL
	const uint16_t hl = gb->cpu.GetHL();
	gb->cpu.SetA(gb->ReadU8(hl));
	gb->cpu.SetHL(hl - 1);
}




void dec_3B(Gameboy* const gb)
{
	// DEC SP
	--gb->cpu.sp;
}




void inc_3C(Gameboy* const gb) 
{
	// INC A ( Z 0 H - )
	const uint8_t a = gb->cpu.GetA();
	gb->cpu.SetA(gb->cpu.INC(a));
}




void dec_3D(Gameboy* const gb) 
{ 
	// DEC A ( Z 1 H - )
	const uint8_t a = gb->cpu.GetA();
	gb->cpu.SetA(gb->cpu.DEC(a));
}



void ld_3E(Gameboy* const gb) 
{ 
	// LD A, d8 ( loads immediate 8 bit value into A )
	gb->cpu.SetA(get_d8(gb));
}



void ccf_3F(Gameboy* const gb)
{
	// CCF ( - 0 0 C )
	const CPU::Flags old_zero = gb->cpu.GetFlags(CPU::FLAG_Z);
	const CPU::Flags old_carry = gb->cpu.GetFlags(CPU::FLAG_C);
	gb->cpu.SetF(old_carry ? old_zero : old_zero | CPU::FLAG_C);
}





// 0x40
// ld_40 LD B, B ( nop )


void ld_41(Gameboy* const gb)
{
	// LD B, C
	gb->cpu.SetB(gb->cpu.GetC());
}

void ld_42(Gameboy* const gb)
{
	// LD B, D
	gb->cpu.SetB(gb->cpu.GetD());
}

void ld_43(Gameboy* const gb)
{ 
	// LD B, E
	gb->cpu.SetB(gb->cpu.GetE());
}

void ld_44(Gameboy* const gb)
{ 
	// LD B, H
	gb->cpu.SetB(gb->cpu.GetH());
}

void ld_45(Gameboy* const gb)
{
	// LD B, L
	gb->cpu.SetB(gb->cpu.GetL());
}



void ld_46(Gameboy* const gb) 
{
	// LD B, (HL)
	const uint16_t hl = gb->cpu.GetHL();
	gb->cpu.SetB(gb->ReadU8(hl));
}





void ld_47(Gameboy* const gb) 
{
	// LD B, A
	gb->cpu.SetB(gb->cpu.GetA());
}







void ld_48(Gameboy* const gb)
{ 
	// LD C, B
	gb->cpu.SetC(gb->cpu.GetB());
}


// ld_49 LD C, C ( nop )


void ld_4A(Gameboy* const gb)
{
	// LD C, D
	gb->cpu.SetC(gb->cpu.GetD());
}

void ld_4B(Gameboy* const gb)
{
	// LD C, E
	gb->cpu.SetC(gb->cpu.GetE());
}

void ld_4C(Gameboy* const gb)
{
	// LD C, H
	gb->cpu.SetC(gb->cpu.GetH());
}


void ld_4D(Gameboy* const gb)
{ 
	// LD C, L
	gb->cpu.SetC(gb->cpu.GetL());
}





void ld_4E(Gameboy* const gb) 
{
	// LD C, (HL)
	const uint16_t hl = gb->cpu.GetHL();
	gb->cpu.SetC(gb->ReadU8(hl));
}





void ld_4F(Gameboy* const gb) 
{
	// LD C, A
	gb->cpu.SetC(gb->cpu.GetA());
}









// 0x50
void ld_50(Gameboy* const gb) 
{
	// LD D, B
	gb->cpu.SetD(gb->cpu.GetB());
}

void ld_51(Gameboy* const gb)
{
	// LD D, C
	gb->cpu.SetD(gb->cpu.GetC());
}


// ld_52 LD D, D ( nop )


void ld_53(Gameboy* const gb)
{
	// LD D, E
	gb->cpu.SetD(gb->cpu.GetE());
}



void ld_54(Gameboy* const gb) 
{ 
	// LD D, H
	gb->cpu.SetD(gb->cpu.GetH());
}


void ld_55(Gameboy* const gb)
{
	// LD D, L
	gb->cpu.SetD(gb->cpu.GetL());
}


void ld_56(Gameboy* const gb) 
{
	// LD D, (HL)
	const uint16_t hl = gb->cpu.GetHL();
	gb->cpu.SetD(gb->ReadU8(hl)); 
}



void ld_57(Gameboy* const gb) 
{
	// LD D, A
	gb->cpu.SetD(gb->cpu.GetA());
}




void ld_58(Gameboy* const gb)
{
	// LD E, B
	gb->cpu.SetE(gb->cpu.GetB());
}

void ld_59(Gameboy* const gb)
{
	// LD E, C
	gb->cpu.SetE(gb->cpu.GetC());
}

void ld_5A(Gameboy* const gb)
{
	// LD E, D
	gb->cpu.SetE(gb->cpu.GetD());
}

// ld_5B LD E, E ( nop )


void ld_5C(Gameboy* const gb)
{
	// LD E, H
	gb->cpu.SetE(gb->cpu.GetH());
}



void ld_5D(Gameboy* const gb) 
{ 
	// LD E, L
	gb->cpu.SetE(gb->cpu.GetL());
}




void ld_5E(Gameboy* const gb) 
{
	// LD E, (HL)
	const uint16_t hl = gb->cpu.GetHL();
	gb->cpu.SetE(gb->ReadU8(hl));
}




void ld_5F(Gameboy* const gb) 
{
	// LD E, A
	gb->cpu.SetE(gb->cpu.GetA());
}









// 0x60
void ld_60(Gameboy* const gb) 
{
	// LD H, B
	gb->cpu.SetH(gb->cpu.GetB());
}



void ld_61(Gameboy* const gb) 
{ 
	// LD H, C
	gb->cpu.SetH(gb->cpu.GetC());
}



void ld_62(Gameboy* const gb) 
{
	// LD H, D
	gb->cpu.SetH(gb->cpu.GetD());
}


void ld_63(Gameboy* const gb)
{
	// LD H, E
	gb->cpu.SetH(gb->cpu.GetE());
}

// ld_64 LD H, H ( nop )


void ld_65(Gameboy* const gb)
{
	// LD H, L
	gb->cpu.SetH(gb->cpu.GetL());
}

void ld_66(Gameboy* const gb)
{
	// LD H, (HL)
	const uint8_t value = gb->ReadU8(gb->cpu.GetHL());
	gb->cpu.SetH(value);
}



void ld_67(Gameboy* const gb) 
{
	// LD H, A
	gb->cpu.SetH(gb->cpu.GetA());
}



void ld_68(Gameboy* const gb)
{
	// LD L, B
	gb->cpu.SetL(gb->cpu.GetB());
}



void ld_69(Gameboy* const gb) 
{
	// LD L, C
	gb->cpu.SetL(gb->cpu.GetC());
}



void ld_6A(Gameboy* const gb)
{
	// LD L, D
	gb->cpu.SetL(gb->cpu.GetD());
}




void ld_6B(Gameboy* const gb) 
{
	// LD L, E
	gb->cpu.SetL(gb->cpu.GetE());
}



void ld_6C(Gameboy* const gb) 
{
	// LD L, H
	gb->cpu.SetL(gb->cpu.GetH());

}


// ld_6D LD L, L ( nop )


void ld_6E(Gameboy* const gb)
{
	// LD L, (HL)
	const uint16_t hl = gb->cpu.GetHL();
	gb->cpu.SetL(gb->ReadU8(hl));
}





void ld_6F(Gameboy* const gb)
{ 
	// LD L, A
	gb->cpu.SetL(gb->cpu.GetA());
}






// 0x70
void ld_70(Gameboy* const gb) 
{
	// LD (HL), B
	const uint16_t hl = gb->cpu.GetHL();
	gb->WriteU8(hl, gb->cpu.GetB());
}




void ld_71(Gameboy* const gb) 
{
	// LD (HL), C
	const uint16_t hl = gb->cpu.GetHL();
	gb->WriteU8(hl, gb->cpu.GetC());
}





void ld_72(Gameboy* const gb) 
{
	// LD (HL), D
	const uint16_t hl = gb->cpu.GetHL();
	gb->WriteU8(hl, gb->cpu.GetD());
}



void ld_73(Gameboy* const gb)
{
	// LD (HL), E
	const uint16_t hl = gb->cpu.GetHL();
	gb->WriteU8(hl, gb->cpu.GetE());
}



void ld_74(Gameboy* const gb)
{
	// LD (HL), H
	gb->WriteU8(gb->cpu.GetHL(), gb->cpu.GetH());
}

void ld_75(Gameboy* const gb)
{
	// LD (HL), L
	gb->WriteU8(gb->cpu.GetHL(), gb->cpu.GetL());
}


void halt_76(Gameboy* const gb)
{
	// halt cpu until interrupt occurs
	if (gb->hwstate.GetIntMaster() && gb->hwstate.interrupt_enable) {
		if (gb->hwstate.GetFlags(HWState::CPU_HALT) == 0)
			gb->hwstate.SetFlags(HWState::CPU_HALT);

		--gb->cpu.pc;
	}
}



void ld_77(Gameboy* const gb) 
{ 
	// LD (HL), A
	const uint16_t hl = gb->cpu.GetHL();
	gb->WriteU8(hl, gb->cpu.GetA());
}



void ld_78(Gameboy* const gb) 
{
	// LD A, B
	gb->cpu.SetA(gb->cpu.GetB());
}




void ld_79(Gameboy* const gb) 
{
	// LD A, C
	gb->cpu.SetA(gb->cpu.GetC());
}




void ld_7A(Gameboy* const gb) 
{
	// LD A, D
	gb->cpu.SetA(gb->cpu.GetD());
}



void ld_7B(Gameboy* const gb)
{ 
	// LD A, E
	gb->cpu.SetA(gb->cpu.GetE());
}




void ld_7C(Gameboy* const gb) 
{
	// LD A, H
	gb->cpu.SetA(gb->cpu.GetH());
}



void ld_7D(Gameboy* const gb)
{ 
	// LD A, L
	gb->cpu.SetA(gb->cpu.GetL());
}



void ld_7E(Gameboy* const gb) 
{
	// LD A, (HL)
	const uint16_t hl = gb->cpu.GetHL();
	gb->cpu.SetA(gb->ReadU8(hl));
}


// ld_7f LD A, A ( nop )


// 0x80
void add_80(Gameboy* const gb)
{ 
	// ADD A, B
	add_a_n(gb->cpu.GetB(), &gb->cpu);
}



void add_81(Gameboy* const gb)
{ 
	// ADD A, C
	add_a_n(gb->cpu.GetC(), &gb->cpu);
}


void add_82(Gameboy* const gb) 
{ 
	// ADD A, D
	add_a_n(gb->cpu.GetD(), &gb->cpu);
}


void add_83(Gameboy* const gb)
{ 
	// ADD A, E
	add_a_n(gb->cpu.GetE(), &gb->cpu);
}

void add_84(Gameboy* const gb)
{
	// ADD A, H
	add_a_n(gb->cpu.GetH(), &gb->cpu);
}



void add_85(Gameboy* const gb)
{
	// ADD A, L
	add_a_n(gb->cpu.GetL(), &gb->cpu);
}




void add_86(Gameboy* const gb)
{ 
	// ADD A, (HL)
	add_a_n(gb->ReadU8(gb->cpu.GetHL()), &gb->cpu);
}




void add_87(Gameboy* const gb) 
{
	// ADD A, A
	add_a_n(gb->cpu.GetA(), &gb->cpu);
}



void adc_88(Gameboy* const gb)
{
	// ADC A, B
	adc_a_n(gb->cpu.GetB(), &gb->cpu);
}


void adc_89(Gameboy* const gb)
{ 
	// ADC A, C
	adc_a_n(gb->cpu.GetC(), &gb->cpu);
}



void adc_8A(Gameboy* const gb)
{
	// ADC A, D
	adc_a_n(gb->cpu.GetD(), &gb->cpu);
}


void adc_8B(Gameboy* const gb)
{
	// ADC A, E
	adc_a_n(gb->cpu.GetE(), &gb->cpu);
}


void adc_8C(Gameboy* const gb)
{
	// ADC A, H
	adc_a_n(gb->cpu.GetH(), &gb->cpu);
}



void adc_8D(Gameboy* const gb)
{ 
	// ADC A, L
	adc_a_n(gb->cpu.GetL(), &gb->cpu);
}



void adc_8E(Gameboy* const gb)
{ 
	// ADC A, (HL)
	adc_a_n(gb->ReadU8(gb->cpu.GetHL()), &gb->cpu);
}


void adc_8F(Gameboy* const gb)
{
	// ADC A, A
	adc_a_n(gb->cpu.GetA(), &gb->cpu);
}




// 0x90
void sub_90(Gameboy* const gb)
{ 
	// SUB B
	sub_a_n(gb->cpu.GetB(), &gb->cpu);
}



void sub_91(Gameboy* const gb)
{ 
	// SUB C
	sub_a_n(gb->cpu.GetC(), &gb->cpu);
}


void sub_92(Gameboy* const gb)
{
	// SUB D
	sub_a_n(gb->cpu.GetD(), &gb->cpu);
}


void sub_93(Gameboy* const gb)
{
	// SUB E
	sub_a_n(gb->cpu.GetE(), &gb->cpu);
}



void sub_94(Gameboy* const gb)
{
	// SUB H
	sub_a_n(gb->cpu.GetH(), &gb->cpu);
}


void sub_95(Gameboy* const gb)
{
	// SUB L
	sub_a_n(gb->cpu.GetL(), &gb->cpu);
}



void sub_96(Gameboy* const gb)
{ 
	// SUB (HL)
	sub_a_n(gb->ReadU8(gb->cpu.GetHL()), &gb->cpu);
}



void sub_97(Gameboy* const gb)
{
	// SUB A
	sub_a_n(gb->cpu.GetA(), &gb->cpu);
}



void sbc_98(Gameboy* const gb)
{
	// SBC A, B
	sbc_a_n(gb->cpu.GetB(), &gb->cpu);
}


void sbc_99(Gameboy* const gb)
{ 
	// SBC A, C
	sbc_a_n(gb->cpu.GetC(), &gb->cpu);
}


void sbc_9A(Gameboy* const gb)
{
	// SBC A, D
	sbc_a_n(gb->cpu.GetD(), &gb->cpu);
}


void sbc_9B(Gameboy* const gb)
{
	// SBC A, E
	sbc_a_n(gb->cpu.GetE(), &gb->cpu);
}

void sbc_9C(Gameboy* const gb)
{
	// SBC A, H
	sbc_a_n(gb->cpu.GetH(), &gb->cpu);
}

void sbc_9D(Gameboy* const gb)
{
	// SBC A, L
	sbc_a_n(gb->cpu.GetL(), &gb->cpu);
}

void sbc_9E(Gameboy* const gb)
{
	// SBC A, (HL)
	sbc_a_n(gb->ReadU8(gb->cpu.GetHL()), &gb->cpu);
}

void sbc_9F(Gameboy* const gb)
{
	// SBC A, A
	sbc_a_n(gb->cpu.GetA(), &gb->cpu);
}









// 0xA0
void and_A0(Gameboy* const gb)
{ 
	// AND B
	and_a_n(gb->cpu.GetB(), &gb->cpu);
}



void and_A1(Gameboy* const gb) 
{
	// AND C
	and_a_n(gb->cpu.GetC(), &gb->cpu);
}



void and_A2(Gameboy* const gb)
{
	// AND D
	and_a_n(gb->cpu.GetD(), &gb->cpu);
}

void and_A3(Gameboy* const gb)
{
	// AND E
	and_a_n(gb->cpu.GetE(), &gb->cpu);
}

void and_A4(Gameboy* const gb)
{
	// AND H
	and_a_n(gb->cpu.GetH(), &gb->cpu);
}

void and_A5(Gameboy* const gb)
{
	// AND L
	and_a_n(gb->cpu.GetL(), &gb->cpu);
}


void and_A6(Gameboy* const gb)
{ 
	// AND (HL)
	and_a_n(gb->ReadU8(gb->cpu.GetHL()), &gb->cpu);
}



void and_A7(Gameboy* const gb) 
{
	// TODO: optimize
	// AND A
	and_a_n(gb->cpu.GetA(), &gb->cpu);
}



void xor_A8(Gameboy* const gb)
{ 
	// XOR B
	xor_a_n(gb->cpu.GetB(), &gb->cpu);
}



void xor_A9(Gameboy* const gb) 
{
	// XOR C
	xor_a_n(gb->cpu.GetC(), &gb->cpu);
}



void xor_AA(Gameboy* const gb)
{
	// XOR D
	xor_a_n(gb->cpu.GetD(), &gb->cpu);
}

void xor_AB(Gameboy* const gb)
{
	// XOR E
	xor_a_n(gb->cpu.GetE(), &gb->cpu);
}

void xor_AC(Gameboy* const gb)
{
	// XOR H
	xor_a_n(gb->cpu.GetH(), &gb->cpu);
}

void xor_AD(Gameboy* const gb)
{
	// XOR L
	xor_a_n(gb->cpu.GetL(), &gb->cpu);
}

void xor_AE(Gameboy* const gb)
{
	// XOR (HL)
	xor_a_n(gb->ReadU8(gb->cpu.GetHL()), &gb->cpu);
}

void xor_AF(Gameboy* const gb) 
{
	// TODO: optimize
	// XOR A
	xor_a_n(gb->cpu.GetA(), &gb->cpu);
}





// 0xB0
void or_B0(Gameboy* const gb) 
{
	// OR B
	or_a_n(gb->cpu.GetB(), &gb->cpu);
}



void or_B1(Gameboy* const gb) 
{
	// OR C
	or_a_n(gb->cpu.GetC(), &gb->cpu);
}




void or_B2(Gameboy* const gb)
{ 
	// OR D
	or_a_n(gb->cpu.GetD(), &gb->cpu);
}


void or_B3(Gameboy* const gb)
{
	// OR E
	or_a_n(gb->cpu.GetE(), &gb->cpu);
}

void or_B4(Gameboy* const gb)
{
	// OR H
	or_a_n(gb->cpu.GetH(), &gb->cpu);
}

void or_B5(Gameboy* const gb)
{
	// OR L
	or_a_n(gb->cpu.GetL(), &gb->cpu);
}




void or_B6(Gameboy* const gb) 
{
	// OR (HL)
	or_a_n(gb->ReadU8(gb->cpu.GetHL()), &gb->cpu);
}



void or_B7(Gameboy* const gb) 
{ 
	// OR A
	or_a_n(gb->cpu.GetA(), &gb->cpu);
}


void cp_B8(Gameboy* const gb)
{ 
	// CP B
	cp_n(gb->cpu.GetB(), &gb->cpu);
}




void cp_B9(Gameboy* const gb) 
{ 
	// CP C
	cp_n(gb->cpu.GetC(), &gb->cpu);
}




void cp_BA(Gameboy* const gb)
{
	// CP D
	cp_n(gb->cpu.GetD(), &gb->cpu);
}

void cp_BB(Gameboy* const gb)
{
	// CP E
	cp_n(gb->cpu.GetE(), &gb->cpu);
}

void cp_BC(Gameboy* const gb)
{
	// CP H
	cp_n(gb->cpu.GetH(), &gb->cpu);
}

void cp_BD(Gameboy* const gb)
{
	// CP L
	cp_n(gb->cpu.GetL(), &gb->cpu);
}


void cp_BE(Gameboy* const gb)
{ 
	// CP (HL)
	const uint16_t hl = gb->cpu.GetHL();
	cp_n(gb->ReadU8(hl), &gb->cpu);
}



void cp_BF(Gameboy* const gb)
{
	// TODO: optimize
	// CP A
	cp_n(gb->cpu.GetA(), &gb->cpu);
}









// 0xC0
void ret_C0(Gameboy* const gb) 
{ 
	// RET NZ
	ret_nn(gb->cpu.GetFlags(CPU::FLAG_Z) == 0, gb);
}



void pop_C1(Gameboy* const gb) 
{
	// POP BC
	gb->cpu.SetBC(gb->PopStack16());
}


void jp_C2(Gameboy* const gb) 
{
	// JP NZ, a16
	jp_nn(gb->cpu.GetFlags(CPU::FLAG_Z) == 0, gb);
}



void jp_C3(Gameboy* const gb) 
{
	// JP a16
	gb->cpu.pc = gb->ReadU16(gb->cpu.pc);
}


void call_C4(Gameboy* const gb) 
{ 
	// CALL NZ, a16
	call_nn(gb->cpu.GetFlags(CPU::FLAG_Z) == 0, gb);
}


void push_C5(Gameboy* const gb) 
{
	// PUSH BC
	gb->PushStack16(gb->cpu.GetBC());
}



void add_C6(Gameboy* const gb)
{ 
	// ADD A, d8 ( Z 0 H C )
	const uint8_t result = gb->cpu.ADD(gb->cpu.GetA(), get_d8(gb));
	gb->cpu.SetA(result);
}



void rst_C7(Gameboy* const gb)
{
	// RST 00h
	rst_nn(0x00, gb);
}



void ret_C8(Gameboy* const gb) 
{
	// RET Z
	ret_nn(gb->cpu.GetFlags(CPU::FLAG_Z) != 0, gb);
}




void ret_C9(Gameboy* const gb) 
{
	// RET
	gb->cpu.pc = gb->PopStack16();
}




void jp_CA(Gameboy* const gb) 
{
	// JP Z, a16
	jp_nn(gb->cpu.GetFlags(CPU::FLAG_Z) != 0, gb);
}





void PREFIX_CB(Gameboy* const gb) 
{
	// PREFIX_CB calls the cb_table -
	// and adds the clock cycles for it
	const uint8_t cb_opcode = get_d8(gb);

	cb_instruction[cb_opcode](gb);
	
	const uint8_t lownibble = GetLowNibble(cb_opcode);

	if (lownibble == 0x06 || lownibble == 0x0E)
		gb->cpu.clock += 16;
	else
		gb->cpu.clock += 4;
}




void call_CC(Gameboy* const gb) 
{
	// CALL Z, a16
	call_nn(gb->cpu.GetFlags(CPU::FLAG_Z) != 0, gb);
}



void call_CD(Gameboy* const gb) 
{
	// CALL a16
	gb->PushStack16(gb->cpu.pc + 2);
	gb->cpu.pc = gb->ReadU16(gb->cpu.pc);
}



void adc_CE(Gameboy* const gb) 
{
	// ADC A, d8 (Z 0 H C)
	const uint8_t result = gb->cpu.ADC(gb->cpu.GetA(), get_d8(gb));
	gb->cpu.SetA(result);
}


void rst_CF(Gameboy* const gb)
{ 
	// RST 08h
	rst_nn(0x08, gb);
}






// 0xD0
void ret_D0(Gameboy* const gb) 
{ 
	// RET NC
	ret_nn(gb->cpu.GetFlags(CPU::FLAG_C) == 0, gb);
}


void pop_D1(Gameboy* const gb) 
{
	// POP DE
	gb->cpu.SetDE(gb->PopStack16());
}



void jp_D2(Gameboy* const gb) 
{ 
	// JP NC, a16
	jp_nn(gb->cpu.GetFlags(CPU::FLAG_C) == 0, gb);
}

// MISSING D3 ----
void call_D4(Gameboy* const gb)
{
	// CALL NC, a16
	call_nn(gb->cpu.GetFlags(CPU::FLAG_C) == 0, gb);
}



void push_D5(Gameboy* const gb) 
{
	// PUSH DE
	gb->PushStack16(gb->cpu.GetDE());
}



void sub_D6(Gameboy* const gb) 
{ 
	// SUB d8 ( Z 1 H C )
	const uint8_t result = gb->cpu.SUB(gb->cpu.GetA(), get_d8(gb));
	gb->cpu.SetA(result);
}


void rst_D7(Gameboy* const gb)
{
	// RST 10h
	rst_nn(0x10, gb);
}

void ret_D8(Gameboy* const gb) 
{ 
	// RET C
	ret_nn(gb->cpu.GetFlags(CPU::FLAG_C) != 0, gb);
}



void reti_D9(Gameboy* const gb)
{ 
	// RETI
	// return and enable interrupts
	gb->cpu.pc = gb->PopStack16();
	gb->hwstate.EnableIntMaster();	
}



void jp_DA(Gameboy* const gb)
{
	// JP C, a16
	jp_nn(gb->cpu.GetFlags(CPU::FLAG_C) != 0, gb);
}

// MISSING DB -----


void call_DC(Gameboy* const gb)
{
	// CALL C, a16
	call_nn(gb->cpu.GetFlags(CPU::FLAG_C) != 0, gb);
}


// MISSING DD -----


void sbc_DE(Gameboy* const gb)
{
	// SBC A, d8
	const uint8_t result = gb->cpu.SBC(gb->cpu.GetA(), get_d8(gb));
	gb->cpu.SetA(result);
}


void rst_DF(Gameboy* const gb)
{ 
	// RST 18h
	rst_nn(0x18, gb);
}




// 0xE0
void ldh_E0(Gameboy* const gb) 
{
	// LDH (a8), A
	gb->WriteU8(get_a8(gb), gb->cpu.GetA());
}



void pop_E1(Gameboy* const gb) 
{
	// POP HL
	gb->cpu.SetHL(gb->PopStack16());
}








void ld_E2(Gameboy* const gb) 
{
	// LD (C), A
	const uint8_t c = gb->cpu.GetC();
	gb->WriteU8(0xFF00 + c, gb->cpu.GetA());
}


// MISSING E3 ----
// MISSING E4 ----



void push_E5(Gameboy* const gb) 
{
	// PUSH HL
	gb->PushStack16(gb->cpu.GetHL());
}



void and_E6(Gameboy* const gb) 
{
	// AND d8
	and_a_n(get_d8(gb), &gb->cpu);
}


void rst_E7(Gameboy* const gb)
{
	// RST 20h
	rst_nn(0x20, gb);
}



void add_E8(Gameboy* const gb) 
{
	// ADD SP, r8 ( 0 0 H C )
	const int8_t r8 = get_r8(gb);
	const uint16_t sp = gb->cpu.sp;
	const uint16_t result = sp + r8;
	uint8_t hc = 0x00;
	
	if ((result & 0xff) < (sp & 0xff))
		hc = CPU::FLAG_C;
	if ((result & 0xf) < (sp & 0xf))
		hc |= CPU::FLAG_H;

	gb->cpu.SetF(hc);
	gb->cpu.sp = static_cast<uint16_t>(result);
}




void jp_E9(Gameboy* const gb) 
{
	// JP (HL)
	gb->cpu.pc = gb->cpu.GetHL();
}



void ld_EA(Gameboy* const gb) 
{
	// LD (a16), A
	gb->WriteU8(get_a16(gb), gb->cpu.GetA());
}




// MISSING EB -----
// MISSING EC -----
// MISSING ED -----
void xor_EE(Gameboy* const gb)
{
	// XOR d8
	xor_a_n(get_d8(gb), &gb->cpu);
}



void rst_EF(Gameboy* const gb) 
{
	// RST 28H
	rst_nn(0x28, gb);
}



// 0XF0
void ldh_F0(Gameboy* const gb) 
{
	// LDH A, (a8) ( put content of memory address  (0xFF00 + immediate 8 bit value(a8)) into A )
	gb->cpu.SetA(gb->ReadU8(get_a8(gb)));
}



void pop_F1(Gameboy* const gb) 
{
	// POP AF ( Z N H C )
	const uint16_t value = gb->PopStack16();
	gb->cpu.SetAF(value & 0xFFF0);
}


void ld_F2(Gameboy* const gb)
{
	// LD A, (C)
	const uint8_t value = gb->ReadU8(0xFF00 + gb->cpu.GetC());
	gb->cpu.SetA(value);
}


void di_F3(Gameboy* const gb)
{
	// DI
	// disable interrupts
	gb->hwstate.DisableIntMaster(); 
}



// MISSING F4 ----


void push_F5(Gameboy* const gb) 
{
	// PUSH AF
	gb->PushStack16(gb->cpu.GetAF());	
}




void or_F6(Gameboy* const gb) 
{ 
	// OR d8
	or_a_n(get_d8(gb), &gb->cpu);
}



void rst_F7(Gameboy* const gb)
{
	// RST 30h
	rst_nn(0x30, gb);
}

void ld_F8(Gameboy* const gb)
{
	// LD HL, SP+r8 ( 0 0 H C )
	const int8_t r8 = get_r8(gb);
	const uint16_t sp = gb->cpu.sp;
	const uint16_t result = sp + r8;
	const uint16_t check = sp ^ r8 ^ result;
	uint8_t hc = 0x00;

	if ((check & 0x100) == 0x100)
		hc = CPU::FLAG_C;
	if ((check & 0x10) == 0x10)
		hc |= CPU::FLAG_H;

	gb->cpu.SetF(hc);
	gb->cpu.SetHL(result);
}

void ld_F9(Gameboy* const gb)
{
	// LD SP, HL
	gb->cpu.sp = gb->cpu.GetHL();
}








void ld_FA(Gameboy* const gb) 
{
	// LD A, (a16)
	gb->cpu.SetA(gb->ReadU8(get_a16(gb)));
}


void ei_FB(Gameboy* const gb)
{ 
	// EI ( enable interrupts )
	gb->hwstate.EnableIntMaster();	
}


// FC MISSING -----
// FD MISSING -----



void cp_FE(Gameboy* const gb) 
{
	// CP d8 ( Z 1 H C )
	cp_n(get_d8(gb), &gb->cpu);
}



void rst_FF(Gameboy* const gb)
{
	// RST 38h
	rst_nn(0x38, gb);
}




// undefined / unknown opcodes
void unknown(Gameboy* const gb) 
{
	fprintf(stderr, "undefined opcode at %4x\n", gb->cpu.pc - 1);
}





const instruction_table_t main_instructions[256] = {
/*        0        1        2        3        4        5        6        7        8        9        A        B        C        D        E        F      */
/*0*/  nop_00,   ld_01,   ld_02,  inc_03,  inc_04,  dec_05,   ld_06, rlca_07,   ld_08,  add_09,   ld_0A,  dec_0B,  inc_0C,  dec_0D,   ld_0E, rrca_0F,
/*1*/ stop_10,   ld_11,   ld_12,  inc_13,  inc_14,  dec_15,   ld_16,  rla_17,   jr_18,  add_19,   ld_1A,  dec_1B,  inc_1C,  dec_1D,   ld_1E,  rra_1F,
/*2*/   jr_20,   ld_21,   ld_22,  inc_23,  inc_24,  dec_25,   ld_26,  daa_27,   jr_28,  add_29,   ld_2A,  dec_2B,  inc_2C,  dec_2D,   ld_2E,  cpl_2F,
/*3*/   jr_30,   ld_31,   ld_32,  inc_33,  inc_34,  dec_35,   ld_36,  scf_37,   jr_38,  add_39,   ld_3A,  dec_3B,  inc_3C,  dec_3D,   ld_3E,  ccf_3F,
/*4*/  nop_00,   ld_41,   ld_42,   ld_43,   ld_44,   ld_45,   ld_46,   ld_47,   ld_48,  nop_00,   ld_4A,   ld_4B,   ld_4C,   ld_4D,   ld_4E,   ld_4F,
/*5*/   ld_50,   ld_51,  nop_00,   ld_53,   ld_54,   ld_55,   ld_56,   ld_57,   ld_58,   ld_59,   ld_5A,  nop_00,   ld_5C,   ld_5D,   ld_5E,   ld_5F,
/*6*/   ld_60,   ld_61,   ld_62,   ld_63,  nop_00,   ld_65,   ld_66,   ld_67,   ld_68,   ld_69,   ld_6A,   ld_6B,   ld_6C,  nop_00,   ld_6E,   ld_6F,
/*7*/   ld_70,   ld_71,   ld_72,   ld_73,   ld_74,   ld_75, halt_76,   ld_77,   ld_78,   ld_79,   ld_7A,   ld_7B,   ld_7C,   ld_7D,   ld_7E,  nop_00,
/*8*/  add_80,  add_81,  add_82,  add_83,  add_84,  add_85,  add_86,  add_87,  adc_88,  adc_89,  adc_8A,  adc_8B,  adc_8C,  adc_8D,  adc_8E,  adc_8F,
/*9*/  sub_90,  sub_91,  sub_92,  sub_93,  sub_94,  sub_95,  sub_96,  sub_97,  sbc_98,  sbc_99,  sbc_9A,  sbc_9B,  sbc_9C,  sbc_9D,  sbc_9E,  sbc_9F,
/*A*/  and_A0,  and_A1,  and_A2,  and_A3,  and_A4,  and_A5,  and_A6,  and_A7,  xor_A8,  xor_A9,  xor_AA,  xor_AB,  xor_AC,  xor_AD,  xor_AE,  xor_AF,
/*B*/   or_B0,   or_B1,   or_B2,   or_B3,   or_B4,   or_B5,   or_B6,   or_B7,   cp_B8,   cp_B9,   cp_BA,   cp_BB,   cp_BC,   cp_BD,   cp_BE,   cp_BF,
/*C*/  ret_C0,  pop_C1,   jp_C2,   jp_C3, call_C4, push_C5,  add_C6,  rst_C7,  ret_C8,  ret_C9,   jp_CA,PREFIX_CB, call_CC, call_CD, adc_CE,  rst_CF,
/*D*/  ret_D0,  pop_D1,   jp_D2, unknown, call_D4, push_D5,  sub_D6,  rst_D7,  ret_D8, reti_D9,   jp_DA, unknown,  call_DC, unknown, sbc_DE,  rst_DF,
/*E*/  ldh_E0,  pop_E1,   ld_E2, unknown, unknown,  push_E5, and_E6,  rst_E7,  add_E8,   jp_E9,   ld_EA, unknown,  unknown, unknown, xor_EE,  rst_EF,
/*F*/  ldh_F0,  pop_F1,   ld_F2, di_F3,   unknown,  push_F5,  or_F6,  rst_F7,   ld_F8,   ld_F9,   ld_FA,  ei_FB,   unknown, unknown, cp_FE,   rst_FF
};


extern const uint8_t clock_table[256] = {
/*     0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
/*0*/  4, 12,  8,  8,  4,  4,  8,  4, 20,  8,  8,  8,  4,  4,  8,  4,
/*1*/  4, 12,  8,  8,  4,  4,  8,  4, 12,  8,  8,  8,  4,  4,  8,  4,
/*2*/  8, 12,  8,  8,  4,  4,  8,  4,  8,  8,  8,  8,  4,  4,  8,  4, 
/*3*/  8, 12,  8,  8, 12, 12, 12,  4,  8,  8,  8,  8,  4,  4,  8,  4,
/*4*/  4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4, 
/*5*/  4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4, 
/*6*/  4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4, 
/*7*/  8,  8,  8,  8,  8,  8,  4,  8,  4,  4,  4,  4,  4,  4,  8,  4,
/*8*/  4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/*9*/  4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/*A*/  4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/*B*/  4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/*C*/  8, 12, 12, 16, 12, 16,  8, 16,  8, 16, 12,  0, 12, 24,  8, 16, 
/*D*/  8, 12, 12,  0, 12, 16,  8, 16,  8, 16, 12,  0, 12,  0,  8, 16, 
/*E*/ 12, 12,  8,  0,  0, 16,  8, 16, 16,  4, 16,  0,  0,  0,  8, 16,
/*F*/ 12, 12,  8,  4,  0, 16,  8, 16, 12,  8, 16,  4,  0,  0,  8, 16
};


}

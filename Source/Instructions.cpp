#include <stdio.h>
#include "Gameboy.hpp"
#include "Instructions.hpp"
namespace gbx {


// common names:
// d8:  immediate 8 bit value
// d16: immediate 16 bit value
// a8:  8 bit unsigned data, which are added to $FF00 in certain instructions (replacement for missing IN and OUT instructions)
// a16: 16 bit address
// r8: means 8 bit signed data, which are added to program counter
// note: on gameboy PC appears to be incremented before the instruction is executed -
// so relative jump instructions (jr) here still need to add its operand length to pc, or to r8 itself 



// Main instructions implementation:
// 0x00
void nop_00(Gameboy* const) {
	// no operation is performed
	// operands: 0
	// clock cyles: 4
	
	printf("NOP\n");
}





void ld_01(Gameboy* const gb) {
	// LD BC, d16
	// load immediate 16 bits value into BC
	// operands: 2
	// clock cyles: 10 or 12 ?
	const auto pc = gb->cpu.GetPC();
	const auto d16 = gb->ReadU16(pc);
	gb->cpu.SetBC(d16);
	gb->cpu.AddPC(2);


	printf("LD BC, %x\n", d16);
}








void ld_02(Gameboy* const gb) {
	// LD (BC), A
	// value in register A is stored in memory location pointed by BC
	// operands: 0
	// clock cycles: 7 or 8 ?
	const auto bc = gb->cpu.GetBC();
	const auto a = gb->cpu.GetA();
	gb->WriteU8(bc, a);
	


	printf("LD (BC), A; ->  BC(%x), A(%x)\n", bc, a);
}







void inc_03(Gameboy* const gb) {
	// INC BC
	// adds one to BC
	// operands: 0
	// clock cyles: 6 or 8 ?
	const auto bc = gb->cpu.GetBC();
	const uint16_t result = bc + 1;
	

	printf("INC BC; -> BC(%x), Result(%x)\n", bc, result);
}








void inc_04(Gameboy* const gb) { 
	// INC B
	// adds one to B
	// operands: 0
	// clock cyles: 4
	// flags affected Z 0 H -
	const auto b = gb->cpu.GetB();
	const auto result = gb->cpu.INC(b);
	gb->cpu.SetB(result);
	

	printf("INC B; -> B(%x), Result(%x) | ", b, result);
	gb->cpu.PrintFlags();
}







void dec_05(Gameboy* const gb) {
	// DEC B
	// decrement B by 1
	// operands: 0
	// clock cycles: 4
	// flags affected Z 1 H -
	const auto b = gb->cpu.GetB();
	const auto result = gb->cpu.DEC(b);
	gb->cpu.SetB(result);
	

	printf("DEC B; -> B(%x), Result(%x) | ", b, result);
	gb->cpu.PrintFlags();
}






void ld_06(Gameboy* const gb) {
	// LD B, d8
	// loads immediate 8 bit value into B
	// operands: 1
	// clock cycles: 7 or 8 ?
	const auto pc = gb->cpu.GetPC();
	const auto d8 = gb->ReadU8(pc);
	gb->cpu.SetB(d8);
	gb->cpu.AddPC(1);


	printf("LD B, %x\n", d8);
}






void rlca_07(Gameboy* const){ ASSERT_INSTR_IMPL();  }
void ld_08(Gameboy* const gb)  { ASSERT_INSTR_IMPL(); gb->cpu.AddPC(2); }
void add_09(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_0A(Gameboy* const)  { ASSERT_INSTR_IMPL();  }





void dec_0B(Gameboy* const gb) {
	// DEC BC
	// decrement BC
	// operands: 0
	// clock cycles: 8
	const auto bc = gb->cpu.GetBC();
	const uint16_t result = bc - 1;
	gb->cpu.SetBC(result);
	
	
	printf("DEC BC; -> BC(%x), Result(%x)\n", bc, result);
}





void inc_0C(Gameboy* const gb) { 
	// INC C
	// increment C
	// operands: 0
	// clock cycles: 4
	// flags affected: Z 0 H -
	const auto c = gb->cpu.GetC();
	const auto result = gb->cpu.INC(c);
	gb->cpu.SetC(result);
	printf("INC C; C(%x), Result(%x) | ", c, result);
	gb->cpu.PrintFlags();
}






void dec_0D(Gameboy* const gb) {
	// DEC C
	// subtract C by one
	// operands: 0
	// clock cycles: 4
	// flags affected: Z 1 H
	const auto c = gb->cpu.GetC();
	const auto result = gb->cpu.DEC(c);
	gb->cpu.SetC(result);
	


	printf("DEC C; -> C(%x), Result(%x) | ", c, result);
	gb->cpu.PrintFlags();
}







void ld_0E(Gameboy* const gb) { 
	// LD C, d8
	// loads immediate 8 bit value into C
	// operands: 1
	// clock cycles: 7 or 8 ?
	const auto pc = gb->cpu.GetPC();
	const auto d8 = gb->ReadU8(pc);
	gb->cpu.SetC(d8);
	gb->cpu.AddPC(1);

	printf("LD C, %x\n", d8);
}






void rrca_0F(Gameboy* const){ ASSERT_INSTR_IMPL();  }












// 0x10
void stop_10(Gameboy* const gb){ ASSERT_INSTR_IMPL(); gb->cpu.AddPC(1); }






void ld_11(Gameboy* const gb) {
	// LD DE, d16
	// store immediate d16 value into DE
	// operands: 2
	// clock cycles: 12
	const auto pc = gb->cpu.GetPC();
	const auto d16 = gb->ReadU16(pc);
	gb->cpu.SetDE(d16);
	gb->cpu.AddPC(2);
	
	
	printf("LD DE, %x\n", d16);
}







void ld_12(Gameboy* const gb) {
	// LD (DE), A
	// store A into mem address pointed by DE
	// operands: 0
	// clock cycles: 8
	const auto de = gb->cpu.GetDE();
	const auto a = gb->cpu.GetA();
	gb->WriteU8(de, a);
	
	
	printf("LD (DE), A; -> DE(%x), A(%x)\n", de, a);
}







void inc_13(Gameboy* const gb) {
	// INC DE
	// increment DE
	// operands: 0
	// clock cycles: 8
	const auto de = gb->cpu.GetDE();
	const uint16_t result = de + 1;
	gb->cpu.SetDE(result);
	
	
	printf("INC DE; -> DE(%x), Result(%x)\n", de, result);
}








void inc_14(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void dec_15(Gameboy* const) { ASSERT_INSTR_IMPL();  }







void ld_16(Gameboy* const gb) {
	// LD D, d8
	// immediate 8 bit value stored into D
	// operands: 1
	// clock cycles: 8
	const auto pc = gb->cpu.GetPC();
	const auto d8 = gb->ReadU8(pc);
	gb->cpu.SetD(d8);
	gb->cpu.AddPC(1);
	
	
	printf("LD D, %x\n", d8);
}








void rla_17(Gameboy* const) { ASSERT_INSTR_IMPL();  }








void jr_18(Gameboy* const gb) {
	// JR r8
	// Add r8 to pc and jump to it.
	// operands: 1
	// clock cycles: 12
	const auto pc = gb->cpu.GetPC();
	const auto r8 = gb->ReadS8(pc);
	const uint16_t address = (pc+r8) + 1;
	gb->cpu.SetPC(address);


	printf("JR %x\n", address);
}






void add_19(Gameboy* const gb) {
	// ADD HL, DE
	// add DE into HL
	// operands: 0
	// clock cycles: 8
	// flags affected: - 0 H C
	const auto hl = gb->cpu.GetHL();
	const auto de = gb->cpu.GetDE();
	gb->cpu.ADDHL(de);
	const auto result = gb->cpu.GetHL();
	
	
	
	printf("LD HL, DE; -> HL(%x), DE(%x), Result(%x) | ", hl, de, result);
	gb->cpu.PrintFlags();
}









void ld_1A(Gameboy* const gb) {
	// LD A, (DE)
	// store value in mem address pointed by DE into A
	// operands: 0
	// clock cycles: 8
	const auto de = gb->cpu.GetDE();
	const auto value = gb->ReadU8(de);
	gb->cpu.SetA(value);
	
	
	printf("LD A, (DE); -> DE(%x), (DE)(%x)\n", de, value);
}






void dec_1B(Gameboy* const) { ASSERT_INSTR_IMPL();  }






void inc_1C(Gameboy* const gb) {
	// INC E
	// operands: 0
	// clock cycles: 4
	// flags affected: Z 0 H -
	const auto e = gb->cpu.GetE();
	const auto result = gb->cpu.INC(e);
	gb->cpu.SetE(result);
	
	
	printf("INC E; -> E(%x), Result(%x) | ", e, result);
	gb->cpu.PrintFlags();
}





void dec_1D(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_1E(Gameboy* const gb)  { ASSERT_INSTR_IMPL(); gb->cpu.AddPC(1); }
void rra_1F(Gameboy* const) { ASSERT_INSTR_IMPL();  }














// 0x20
void jr_20(Gameboy* const gb) {
	// JR NZ, r8
	// jump if Z flags is reset
	// operands: 1
	// clock cycles: 12 if jumps, 8 if not

	const auto pc = gb->cpu.GetPC();
	const auto r8 = gb->ReadS8(pc);
	const uint16_t jump_addr = (pc+r8) + 1;

	if(gb->cpu.GetFlags(CPU::FLAG_Z) == 0)
		gb->cpu.SetPC(jump_addr);
	else 
		gb->cpu.AddPC(1);


	printf("JR NZ, %x\n", jump_addr);
}








void ld_21(Gameboy* const gb) {
	// LD HL, d16
	// load immediate 16 bit value into HL
	// operands: 2
	// clock cycles: 10 or 12
	const auto pc = gb->cpu.GetPC();
	const auto d16 = gb->ReadU16(pc);
	gb->cpu.SetHL(d16);
	gb->cpu.AddPC(2);


	printf("LD HL, %x\n", d16);
}






void ld_22(Gameboy* const gb) {
	// LD (HL+), A / or /  LD (HLI), A / or / LDI (HL), A
	// Put A into memory address HL. Increment HL
	// operands: 0
	// clock cycles: 8
	const auto hl = gb->cpu.GetHL();
	const auto a = gb->cpu.GetA();
	gb->WriteU8(hl, a);
	gb->cpu.SetHL(hl + 1);
	
	
	printf("LD (HL+), A; -> HL(%x), A(%x)\n", hl, a);
}







void inc_23(Gameboy* const gb) {
	// INC HL
	// operands: 0
	// clock cycles: 8
	const auto hl = gb->cpu.GetHL();
	const uint16_t result = hl + 1;
	gb->cpu.SetHL(result);
	

	printf("INC HL; -> HL(%x), Result(%x)\n", hl, result);
}





void inc_24(Gameboy* const){ ASSERT_INSTR_IMPL();  }
void dec_25(Gameboy* const){ ASSERT_INSTR_IMPL();  }
void ld_26(Gameboy* const gb) { ASSERT_INSTR_IMPL(); gb->cpu.AddPC(1); }
void daa_27(Gameboy* const){ ASSERT_INSTR_IMPL();  }






void jr_28(Gameboy* const gb) {
	// JP Z, r8
	// jump if Z flag is set
	// operands: 1
	// clock cycles: if jumps 12, else 8
	const auto pc = gb->cpu.GetPC();
	const auto r8 = gb->ReadS8(pc);
	const auto zero_flag = gb->cpu.GetFlags(CPU::FLAG_Z);
	const uint16_t jump_addr = (pc+r8) + 1;
	
	if(zero_flag)
		gb->cpu.SetPC(jump_addr);
	else
		gb->cpu.AddPC(1);
	
	printf("JP Z, %x\n", jump_addr);
}






void add_29(Gameboy* const) { ASSERT_INSTR_IMPL();  }








void ld_2A(Gameboy* const gb) {
	// LD A, (HL+)
	// store value in address pointed by HL into A, increment HL
	// operands: 0
	// clock cycles: 8
	const auto hl = gb->cpu.GetHL();
	const auto value = gb->ReadU8(hl);
	gb->cpu.SetA(value);
	gb->cpu.SetHL(hl + 1);
	 

	printf("LD A, (HL+); -> HL(%x), (HL)(%x)\n", hl, value);
}










void dec_2B(Gameboy* const gb) {
	// DEC HL
	// decrement register HL by 1
	// operands: 0
	// clock cycles: 8
	const auto hl = gb->cpu.GetHL();
	const uint16_t result = hl - 1;
	gb->cpu.SetHL(result);
	
	
	printf("DEC HL; -> HL(%x), Result(%x)\n", hl, result);
}






void inc_2C(Gameboy* const){ ASSERT_INSTR_IMPL();  }
void dec_2D(Gameboy* const){ ASSERT_INSTR_IMPL();  }
void ld_2E(Gameboy* const gb) { ASSERT_INSTR_IMPL(); gb->cpu.AddPC(1); }



void cpl_2F(Gameboy* const gb) {
	// CPL
	// Complement A register, flip all bits
	// operands: 0
	// clock cycles: 4
	// flags affected: - 1 1 -
	const auto a = gb->cpu.GetA();
	const uint8_t result = ~a;
	gb->cpu.SetA(result);
	gb->cpu.SetF(CPU::FLAG_N | CPU::FLAG_H);
	
	
	printf("CPL ; -> A(%x), Result(%x)\n", a, result);
}










// 0x30
void jr_30(Gameboy* const gb) {
	// JR NC, r8
	// jump if C flag is reset
	// operands: 1
	// clock cycles: 12 if jump 8 if not
	
	const auto carry_flag = gb->cpu.GetFlags(CPU::FLAG_C);
	const auto pc = gb->cpu.GetPC();
	const auto r8 = gb->ReadS8(pc);
	const uint16_t jump_addr = (pc+r8) + 1;
	
	if(!carry_flag)
		gb->cpu.SetPC(jump_addr);
	else
		gb->cpu.AddPC(1);
	
	printf("JR NC, %x\n", jump_addr);
}







void ld_31(Gameboy* const gb) {
	// LD SP, d16
	// loads immediate 16 bits value into SP
	// operands: 2
	// clock cycles: 10 or 12 ?
	const auto pc = gb->cpu.GetPC();
	const auto d16 = gb->ReadU16(pc);
	gb->cpu.SetSP(d16);
	gb->cpu.AddPC(2); 

	printf("LD SP, %x\n", d16);
}







void ld_32(Gameboy* const gb) {
	// LD (HL-), A  / or / LD (HLD), A / or / LDD (HL), A
	// store A into memory pointed by HL, Decrements HL
	// operands: 0
	// clock cycles: 8
	const auto a = gb->cpu.GetA();
	const auto hl = gb->cpu.GetHL();	
	gb->WriteU8(hl, a);
	gb->cpu.SetHL(hl - 1);
	

	printf("LD (HL-), A; -> HL(%x) , A(%x)\n", hl, a);
}





void inc_33(Gameboy* const){ ASSERT_INSTR_IMPL();  }





void inc_34(Gameboy* const gb) {
	// INC (HL)
	// increment value pointed by hl in memory
	// operands: 0
	// clock cycles: 12
	const auto hl = gb->cpu.GetHL();
	const auto value = gb->ReadU8(hl);
	const auto result = gb->cpu.INC(value);
	gb->WriteU8(hl, result);

	printf("INC (HL); -> HL(%X), (HL)(%x), Result(%x) | ", hl, value, result);
	gb->cpu.PrintFlags();
}




void dec_35(Gameboy* const){ ASSERT_INSTR_IMPL();  }





void ld_36(Gameboy* const gb) {
	// LD (HL), d8
	// store d8 into mem address pointed by HL
	// operands: 1
	// clock cycles: 12
	const auto pc = gb->cpu.GetPC();
	const auto hl = gb->cpu.GetHL();
	const auto d8 = gb->ReadU8(pc);
	gb->WriteU8(hl, d8);
	gb->cpu.AddPC(1);
	
	printf("LD (HL), %x; -> HL(%x)\n", d8, hl);
}






void scf_37(Gameboy* const){ ASSERT_INSTR_IMPL();  }






void jr_38(Gameboy* const gb) {
	// JR C, r8
	// jump if C flag is set
	// operands: 1
	// clock cycles: if jumps 12, else 8
	const auto pc = gb->cpu.GetPC();
	const auto r8 = gb->ReadS8(pc);
	const auto carry_flag = gb->cpu.GetFlags(CPU::FLAG_C);
	const uint16_t jump_addr = (pc+r8) + 1;

	if(carry_flag)
		gb->cpu.SetPC(jump_addr);
	else
		gb->cpu.AddPC(1);


	printf("JR C, %x\n", jump_addr);
}





void add_39(Gameboy* const){ ASSERT_INSTR_IMPL();  }
void ld_3A(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void dec_3B(Gameboy* const){ ASSERT_INSTR_IMPL();  }




void inc_3C(Gameboy* const gb) {
	// INC A
	// increment A
	// operands: 0
	// clock cycles: 4
	// flags affected: Z 0 H -
	const auto a = gb->cpu.GetA();
	const auto result = gb->cpu.INC(a);
	gb->cpu.SetA(result);

	printf("INC A; -> A(%x), Result(%x) | ", a, result);
	gb->cpu.PrintFlags();
}




void dec_3D(Gameboy* const gb) { 
	// DEC A
	// decrement A
	// operands: 0
	// clock cycles: 4
	// flags affected: Z 1 H -
	const auto a = gb->cpu.GetA();
	const auto result = gb->cpu.DEC(a);
	gb->cpu.SetA(result);

	printf("DEC A; -> A(%x), Result(%x) | ", a, result);
	gb->cpu.PrintFlags();
}








void ld_3E(Gameboy* const gb) { 
	// LD A, d8
	// loads immediate 8 bit value into A
	// operands: 1
	// clock cycles: 7 or 8 ?
	const auto pc = gb->cpu.GetPC();
	const auto d8 = gb->ReadU8(pc);
	gb->cpu.SetA(d8);
	gb->cpu.AddPC(1);

	printf("LD A, %x\n", d8);
}





void ccf_3F(Gameboy* const){ ASSERT_INSTR_IMPL();  }






// 0x40
void ld_40(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_41(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_42(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_43(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_44(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_45(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_46(Gameboy* const) { ASSERT_INSTR_IMPL();  }








void ld_47(Gameboy* const gb) {
	// LD B, A
	// value in A is stored in B
	// operands: 0
	// clock cycles: 4
	const auto a = gb->cpu.GetA();
	gb->cpu.SetB(a);
	
	
	printf("LD B, A; -> A(%x)\n", a);
}







void ld_48(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_49(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_4A(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_4B(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_4C(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_4D(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_4E(Gameboy* const) { ASSERT_INSTR_IMPL();  }





void ld_4F(Gameboy* const gb) {
	// LD C, A
	// value in A is stored in C
	// operands: 0
	// clock cycles: 4
	const auto a = gb->cpu.GetA();
	gb->cpu.SetC(a);
	
	
	printf("LD C, A; -> A(%x)\n", a);
}









// 0x50
void ld_50(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_51(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_52(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_53(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_54(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_55(Gameboy* const) { ASSERT_INSTR_IMPL();  }








void ld_56(Gameboy* const gb) {
	// LD D, (HL)
	// value in memory address pointed by HL is stored in D
	// operands: 0
	// clock cycles: 8
	const auto hl = gb->cpu.GetHL();
	const auto value = gb->ReadU8(hl);
	gb->cpu.SetD(value); 
	

	printf("LD D, (HL); -> HL(%x), (HL)(%x)\n", hl, value);
}









void ld_57(Gameboy* const gb) {
	// LD D, A
	// the value in A is loaded into D
	// operands: 0
	// clock cycles: 4
	const auto a = gb->cpu.GetA();
	gb->cpu.SetD(a);
	

	printf("LD D, A; -> A(%x)\n", a);
}







void ld_58(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_59(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_5A(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_5B(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_5C(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_5D(Gameboy* const) { ASSERT_INSTR_IMPL();  }







void ld_5E(Gameboy* const gb) {
	// LD E, (HL)
	// value in address pointed by HL is stored into E
	// operands: 0
	// clock cycles: 8
	const auto hl = gb->cpu.GetHL();
	const auto value = gb->ReadU8(hl);
	gb->cpu.SetE(value);
	


	printf("LD E, (HL); -> HL(%x), (HL)(%x)\n", hl, value);
}







void ld_5F(Gameboy* const gb) {
	// LD E, A
	// value in A is stored in E
	// operands: 0
	// clock cycles: 4
	const auto a = gb->cpu.GetA();
	gb->cpu.SetE(a);
	
	
	printf("LD E, A; -> A(%x)\n", a);
}









// 0x60
void ld_60(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_61(Gameboy* const) { ASSERT_INSTR_IMPL();  }








void ld_62(Gameboy* const gb) {
	// LD H, D
	// value in D is stored into H
	// operands: 0
	// clock cycles: 4
	
	const auto d = gb->cpu.GetD();
	gb->cpu.SetH(d);
	

	printf("LD H, D; -> D(%x)\n", d);
}




void ld_63(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_64(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_65(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_66(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_67(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_68(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_69(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_6A(Gameboy* const) { ASSERT_INSTR_IMPL();  }







void ld_6B(Gameboy* const gb) {
	// LD L, E
	// value in E is stored in L
	// operands: 0
	// clock cycles: 4
	const auto e = gb->cpu.GetE();
	gb->cpu.SetL(e);
	

	printf("LD L, E; -> E(%x)\n", e);
}









void ld_6C(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_6D(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_6E(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_6F(Gameboy* const) { ASSERT_INSTR_IMPL();  }






// 0x70
void ld_70(Gameboy* const gb) {
	// LD (HL), B
	// value in B is stored into address pointed by HL
	// operands: 0
	// clock cycles: 8
	const auto hl = gb->cpu.GetHL();
	const auto b = gb->cpu.GetB();
	gb->WriteU8(hl, b);
	

	printf("LD (HL), B; -> HL(%x), B(%x)\n", hl, b);
}











void ld_71(Gameboy* const gb) {
	// LD (HL), C
	// value in C is stored into address pointed by HL
	// operands: 0
	// clock cycles: 8
	const auto hl = gb->cpu.GetHL();
	const auto c  = gb->cpu.GetC();
	gb->WriteU8(hl, c);
	

	printf("LD (HL), C; -> HL(%x), C(%x)\n", hl, c);
}








void ld_72(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_73(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_74(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_75(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void halt_76(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_77(Gameboy* const) { ASSERT_INSTR_IMPL();  }






void ld_78(Gameboy* const gb) {
	// LD A, B
	// store value in B into A
	// operands: 0
	// clock cycles: 4
	const auto b = gb->cpu.GetB();
	gb->cpu.SetA(b);
	
	
	printf("LD A, B; -> B(%x)\n", b);
}







void ld_79(Gameboy* const gb) {
	// LD A, C
	// value in C is stored in A
	// operands: 0
	// clock cycles: 4
	const auto c = gb->cpu.GetC();
	gb->cpu.SetA(c);
	
	
	printf("LD A, C; -> C(%x)\n", c);
}










void ld_7A(Gameboy* const gb) {
	// LD A, D
	// value in D is stored in A
	// operands: 0
	// clock cycles: 4
	const auto d = gb->cpu.GetD();
	gb->cpu.SetA(d);
	
 
	printf("LD A, D; -> D(%x)\n", d);
}







void ld_7B(Gameboy* const) { ASSERT_INSTR_IMPL();  }






void ld_7C(Gameboy* const gb) {
	// LD A, H
	// store value in H into A
	// operands: 0
	// clock cycles: 4
	const auto h = gb->cpu.GetH();
	gb->cpu.SetA(h);
	
	
	printf("LD A, H; -> H(%x)\n", h);
}







void ld_7D(Gameboy* const) { ASSERT_INSTR_IMPL();  }






void ld_7E(Gameboy* const gb) {
	// LD A, (HL)
	// value in mem pointed by HL is stored in A
	// operands: 0
	// clock cycles: 8
	const auto hl = gb->cpu.GetHL();
	const auto value = gb->ReadU8(hl);
	gb->cpu.SetA(value);
	
	printf("LD A, (HL); -> HL(%x), (HL)(%x)\n", hl, value);
}







void ld_7F(Gameboy* const) { ASSERT_INSTR_IMPL();  }









// 0x90
void add_80(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void add_81(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void add_82(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void add_83(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void add_84(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void add_85(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void add_86(Gameboy* const) { ASSERT_INSTR_IMPL();  }






void add_87(Gameboy* const gb) {
	// ADD A, A
	// add A into A
	// operands: 0
	// clock cycles: 4
	// flags affected: Z 0 H C
	const auto a = gb->cpu.GetA();
	const auto result = gb->cpu.ADD(a, a);
	gb->cpu.SetA(result);
	

	printf("ADD A, A; -> A(%x), Result(%x) | ", a, result);
	gb->cpu.PrintFlags();
}







void adc_88(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void adc_89(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void adc_8A(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void adc_8B(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void adc_8C(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void adc_8D(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void adc_8E(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void adc_8F(Gameboy* const) { ASSERT_INSTR_IMPL();  }






// 0x90
void sub_90(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void sub_91(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void sub_92(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void sub_93(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void sub_94(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void sub_95(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void sub_96(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void sub_97(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void sbc_98(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void sbc_99(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void sbc_9A(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void sbc_9B(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void sbc_9C(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void sbc_9D(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void sbc_9E(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void sbc_9F(Gameboy* const) { ASSERT_INSTR_IMPL();  }









// 0xA0
void and_A0(Gameboy* const) { ASSERT_INSTR_IMPL();  }





void and_A1(Gameboy* const gb) {
	// AND B
	// logical AND on B with A, result in A
	// operands: 0
	// clock cycles: 4
	// flags affected: Z 0 1 0
	const auto b = gb->cpu.GetB();
	const auto a = gb->cpu.GetA();
	const auto result = gb->cpu.AND(b, a);
	gb->cpu.SetA(result);
	
	
	printf("AND B; -> B(%x), A(%x), Result(%x) | ", b, a, result);
	gb->cpu.PrintFlags();
}





void and_A2(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void and_A3(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void and_A4(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void and_A5(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void and_A6(Gameboy* const) { ASSERT_INSTR_IMPL();  }






void and_A7(Gameboy* const gb) {
	// AND A
	// Logically AND A with A, result in A.
	// operands: 0
	// clock cycles: 4
	// flags affected: Z 0 1 0
	const auto a = gb->cpu.GetA();
	const auto result = gb->cpu.AND(a, a);
	gb->cpu.SetA(result);
	
	
	printf("AND A; -> A(%x), Result(%x) | ", a, result);
	gb->cpu.PrintFlags();
}







void xor_A8(Gameboy* const) { ASSERT_INSTR_IMPL();  }






void xor_A9(Gameboy* const gb) {
	// XOR C
	// logical XOR on C with A, result in A
	// operands: 0
	// clock cycles: 4
	// flags affected: Z 0 0 0
	const auto a = gb->cpu.GetA();
	const auto c = gb->cpu.GetC();
	const auto result = gb->cpu.XOR(c, a);
	gb->cpu.SetA(result);
	
	
	printf("XOR C; -> C(%x), A(%x), Result(%x) | ", c, a, result);
	gb->cpu.PrintFlags();
}






void xor_AA(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void xor_AB(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void xor_AC(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void xor_AD(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void xor_AE(Gameboy* const) { ASSERT_INSTR_IMPL();  }






void xor_AF(Gameboy* const gb) {
	// XOR A
	// bitwise xor in a with a
	// operands: 0
	// clock cycles: 4
	// flags affected: Z 0 0 0
	const auto a = gb->cpu.GetA();
	const auto result = gb->cpu.XOR(a, a);
	gb->cpu.SetA(result);
	

	printf("XOR A; -> A(%x), Result(%x) | ", a, result);
	gb->cpu.PrintFlags();
}











// 0xB0
void or_B0(Gameboy* const gb) {
	// OR B
	// logical or on B with A, result in A
	// operands: 0
	// clock cycles: 4
	// flags affected: Z 0 0 0
	const auto b = gb->cpu.GetB();
	const auto a = gb->cpu.GetA();
	const auto result = gb->cpu.OR(b, a);
	gb->cpu.SetA(result);
	
	
	printf("OR B; -> B(%x), A(%x), Result(%x) | ", b, a, result);
	gb->cpu.PrintFlags();
}









void or_B1(Gameboy* const gb) {
	// OR C
	// or C with A, result in A
	// operands: 0
	// clock cycles: 4
	// flags affected: Z 0 0 0
	const auto a = gb->cpu.GetA();
	const auto c = gb->cpu.GetC();
	const auto result = gb->cpu.OR(c, a);
	gb->cpu.SetA(result);
	
	
	
	printf("OR C; -> A(%x), C(%x), Result(%x) | ", a, c, result);
	gb->cpu.PrintFlags();
}





void or_B2(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void or_B3(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void or_B4(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void or_B5(Gameboy* const) { ASSERT_INSTR_IMPL();  }








void or_B6(Gameboy* const gb) {
	// OR (HL)
	// value in memory address pointed by HL is or'ed with A, result in A
	// operands: 0
	// clock cycles: 8
	// flags affected: Z 0 0 0
	const auto a = gb->cpu.GetA();
	const auto hl = gb->cpu.GetHL();
	const auto value = gb->ReadU8(hl);
	const auto result = gb->cpu.OR(value, a);
	gb->cpu.SetA(result);
	

	printf("OR (HL); -> HL(%x), (HL)(%x), A(%x), Result(%x) | ", hl, value, a, result);
	gb->cpu.PrintFlags();
}








void or_B7(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void cp_B8(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void cp_B9(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void cp_BA(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void cp_BB(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void cp_BC(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void cp_BD(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void cp_BE(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void cp_BF(Gameboy* const) { ASSERT_INSTR_IMPL();  }









// 0xC0
void ret_C0(Gameboy* const gb) { 
	// RET NZ
	// return if zero flag is reset
	// operands: 0
	// clock cycles: 20 if return, 8 if not
	const auto zero_flag = gb->cpu.GetFlags(CPU::FLAG_Z);
	if (!zero_flag) {
		const auto addr = gb->PopStack16();
		gb->cpu.SetPC(addr);
	}
	else {
		
	}

	printf("RET NZ\n");
}







void pop_C1(Gameboy* const gb) {
	// POP BC
	// operands: 0
	// clock cycles: 12
	const auto value = gb->PopStack16();
	gb->cpu.SetBC(value);
	
	
	printf("POP BC;\n");
}







void jp_C2(Gameboy* const gb)   { ASSERT_INSTR_IMPL(); gb->cpu.AddPC(2); }








void jp_C3(Gameboy* const gb) {
	// JP a16
	// 16 bit address is copied to PC
	// operands: 2
	// clock cycles: 10 or 16 ?
	const auto pc = gb->cpu.GetPC();
	const auto a16 = gb->ReadU16(pc);
	gb->cpu.SetPC(a16);

	printf("JP %x\n", a16);
}






void call_C4(Gameboy* const gb) { ASSERT_INSTR_IMPL(); gb->cpu.AddPC(2); }






void push_C5(Gameboy* const gb) {
	// PUSH BC
	// Push register pair BC onto stack.
	// operands: 0
	// clock cycles: 16
	const auto bc = gb->cpu.GetBC();
	gb->PushStack16(bc);
	
	
	printf("PUSH BC\n");
}







void add_C6(Gameboy* const gb)  { ASSERT_INSTR_IMPL(); gb->cpu.AddPC(1); }
void rst_C7(Gameboy* const)  { ASSERT_INSTR_IMPL();  }







void ret_C8(Gameboy* const gb) {
	// RET Z
	// return if z flag is set
	// operands: 0
	// clock cycles: if jump 20. if not 8
	if(gb->cpu.GetFlags(CPU::FLAG_Z)) {
		const auto address = gb->PopStack16();
		gb->cpu.SetPC(address);
	}
	else {
		
	}
	
	printf("RET Z\n");
}







void ret_C9(Gameboy* const gb) {
	// RET
	// return from subroutine
	// pop 2 bytes from stack and jump to that address
	// operands: 0
	// clock cycles: 16

	const auto address = gb->PopStack16();
	gb->cpu.SetPC(address);

	printf("RET\n");
}






void jp_CA(Gameboy* const gb) {
	// JP Z, a16
	// jump to immediate 16 bit address if flag Z is set
	// operands: 2
	// clock cycles: if jump 16. if not 12
	const auto pc = gb->cpu.GetPC();
	const auto a16 = gb->ReadU16(pc);
	const auto zero_flag = gb->cpu.GetFlags(CPU::FLAG_Z);
	
	if(zero_flag)
		gb->cpu.SetPC(a16);
	else
		gb->cpu.AddPC(2);
	
	printf("JP Z, %x\n", a16);
}







void PREFIX_CB(Gameboy* const gb) { ASSERT_INSTR_IMPL(); gb->cpu.AddPC(2); }
void call_CC(Gameboy* const gb)   { ASSERT_INSTR_IMPL(); gb->cpu.AddPC(2); }








void call_CD(Gameboy* const gb) {
	// CALL a16
	// call subroutine at immediate 16 bits address
	// stack grows backwards ?
	// operands: 2
	// clock cycles: 24
	const auto pc = gb->cpu.GetPC();
	const auto a16 = gb->ReadU16(pc);

	gb->PushStack16(pc + 2);
	gb->cpu.SetPC(a16);

	printf("CALL %x\n", a16);
}






void adc_CE(Gameboy* const gb)  { ASSERT_INSTR_IMPL(); gb->cpu.AddPC(1); }
void rst_CF(Gameboy* const)  { ASSERT_INSTR_IMPL();  }










// 0xD0
void ret_D0(Gameboy* const gb) { 
	// RET NC
	// return if C flag is reset
	// operands: 0
	// clock cycles: 20 if return 8 if not
	const auto carry_flag = gb->cpu.GetFlags(CPU::FLAG_C);

	if(!carry_flag)
		gb->cpu.SetPC(gb->PopStack16());
	else
		


	printf("RET NC;\n");
}









void pop_D1(Gameboy* const gb) {
	// POP DE
	// Pop two bytes off stack into register pair nn.
	// operands: 0
	// clock cycles: 12
	const auto value = gb->PopStack16();
	gb->cpu.SetDE(value);
	
	
	printf("POP DE;\n");
}







void jp_D2(Gameboy* const gb)   { ASSERT_INSTR_IMPL(); gb->cpu.AddPC(2); }
// MISSING D3 ----
void call_D4(Gameboy* const gb) { ASSERT_INSTR_IMPL(); gb->cpu.AddPC(2); }








void push_D5(Gameboy* const gb) {
	// PUSH DE
	// push DE into stack
	// operands: 0
	// clock cycles: 16
	const auto de = gb->cpu.GetDE();
	gb->PushStack16(de);
	
	
	printf("PUSH DE\n");
}








void sub_D6(Gameboy* const gb) { ASSERT_INSTR_IMPL(); gb->cpu.AddPC(1); }
void rst_D7(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ret_D8(Gameboy* const) { ASSERT_INSTR_IMPL();  }


void reti_D9(Gameboy* const) { ASSERT_INSTR_IMPL(); }





void jp_DA(Gameboy* const gb)   { ASSERT_INSTR_IMPL(); gb->cpu.AddPC(2); }
// MISSING DB -----
void call_DC(Gameboy* const gb) { ASSERT_INSTR_IMPL(); gb->cpu.AddPC(2); }
// MISSING DD -----
void sbc_DE(Gameboy* const gb)  { ASSERT_INSTR_IMPL(); gb->cpu.AddPC(1); }
void rst_DF(Gameboy* const)     { ASSERT_INSTR_IMPL();  }









// 0xE0
void ldh_E0(Gameboy* const gb) {
	// LDH (a8), A
	// store value in A into memory address 0xFF00 + a8
	// operands: 1
	// clock cycles: 12
	const auto a = gb->cpu.GetA();
	const auto pc = gb->cpu.GetPC();
	const auto a8 = gb->ReadU8(pc);
	gb->WriteU8(0xFF00 + a8, a);
	gb->cpu.AddPC(1);

	printf("LDH (0xFF00 + %x), A; -> A(%x)\n", a8, a);
}









void pop_E1(Gameboy* const gb) {
	// POP HL
	// pop 2 bytes off stack into register HL
	// operands: 0
	// clock cycles: 12
	const auto value = gb->PopStack16();
	gb->cpu.SetHL(value);
	

	printf("POP HL\n");
}








void ld_E2(Gameboy* const gb) {
	// LD (C), A
	// store value in A into memory address 0xFF00 + C
	// operand length: 0
	// clock cycles: 8
	const auto c = gb->cpu.GetC();
	const auto a = gb->cpu.GetA();
	gb->WriteU8(0xFF00 + c, a);
	printf("LD (C), A; -> C(%x), A(%x)\n", c, a);
}






// MISSING E3 ----
// MISSING E4 ----







void push_E5(Gameboy* const gb) {
	// PUSH HL
	// push hl register into stack
	// operands: 0
	// clock cycles: 16
	const auto hl = gb->cpu.GetHL();
	gb->PushStack16(hl);
	

	printf("PUSH HL\n");
}









void and_E6(Gameboy* const gb) {
	// AND d8
	// logical AND d8 with A, result in A.
	// operands: 1
	// clock cycles: 8
	// flags affected: Z 0 1 0
	const auto a = gb->cpu.GetA();
	const auto pc = gb->cpu.GetPC();
	const auto d8 = gb->ReadU8(pc);
	const auto result = gb->cpu.AND(d8, a);
	gb->cpu.SetA(result);
	gb->cpu.AddPC(1);
	
	printf("AND %x; -> A(%x), Result(%x) | ", d8, a, result);
	gb->cpu.PrintFlags();
}






void rst_E7(Gameboy* const) { ASSERT_INSTR_IMPL();  }










void add_E8(Gameboy* const gb) {
	// ADD SP, operands: 2
	// add immediate signed 8 bit data to SP
	// operands: 1
	// clock cycles: 16
	// flags affected: 0 0 H C
	const auto pc = gb->cpu.GetPC();
	const auto sp = gb->cpu.GetSP();
	const auto r8 = gb->ReadS8(pc);
	const uint32_t result = sp + r8;
	const uint8_t flags_result = CheckH_11th_bit(sp, r8) | CheckC_15th_bit(result);
	gb->cpu.SetF(flags_result);
	
	gb->cpu.SetSP(result & 0xffff);
	gb->cpu.AddPC(1);
	
	printf("ADD SP, %i; -> SP(%x) | ", r8, result);
	gb->cpu.PrintFlags();
}










void jp_E9(Gameboy* const gb) {
	// JP (HL)
	// Jump to address contained in HL
	// operands: 0
	// clock cycles: 4
	const auto hl = gb->cpu.GetHL();
	gb->cpu.SetPC(hl);
	
	printf("JP (HL); HL -> (%x)\n", hl);
}









void ld_EA(Gameboy* const gb) {
	// LD (a16), A
	// store value in A into immediate 16 bits address
	// operands: 2
	// clock cycles: 16
	const auto pc = gb->cpu.GetPC();
	const auto a16 = gb->ReadU16(pc);
	const auto a = gb->cpu.GetA();
	gb->WriteU8(a16, a);
	gb->cpu.AddPC(2);


	printf("LD (%x), A; -> A(%x)\n", a16, a);
}







// MISSING EB -----
// MISSING EC -----
// MISSING ED -----
void xor_EE(Gameboy* const gb) { ASSERT_INSTR_IMPL(); gb->cpu.AddPC(1); }





void rst_EF(Gameboy* const gb) {
	// RST 28H
	// push present address onto stack, and jump to 0x0000 + 0x28
	// operands: 0
	// clock cycles: 16
	const auto pc = gb->cpu.GetPC();
	gb->PushStack16(pc);
	gb->cpu.SetPC(0x0028);
	
	printf("RST 28H\n");
}












// 0XF0
void ldh_F0(Gameboy* const gb) {
	// LDH A, (a8)
	// put content of memory address  (0xFF00 + immediate 8 bit value(a8)) into A
	// operands: 1
	// clock cycles: 12
	const auto pc = gb->cpu.GetPC();
	const auto a8 = gb->ReadU8(pc);
	const uint16_t evaluated_address = 0xFF00 + a8;
	const auto value = gb->ReadU8(evaluated_address);
	gb->cpu.SetA(value);
	gb->cpu.AddPC(1);


	printf("LDH A, (0xff00 + %x); -> A(%x)\n", a8, value);
}








void pop_F1(Gameboy* const gb) {
	// POP AF
	// pop 16 bits from stack into AF
	// operands: 0
	// clock cycles: 12
	// flags affected: all
	const auto value = gb->PopStack16();
	gb->cpu.SetAF(value);
	
	
	printf("POP AF\n");
}







void ld_F2(Gameboy* const gb)  { ASSERT_INSTR_IMPL(); gb->cpu.AddPC(1); }










void di_F3(Gameboy* const) { ASSERT_INSTR_IMPL(); }










// MISSING F4 ----





void push_F5(Gameboy* const gb) {
	// PUSH AF
	// push register AF into stack
	// operands: 0
	// clock cycles: 16
	gb->PushStack16(gb->cpu.GetAF());
	

	printf("PUSH AF\n");
}







void or_F6(Gameboy* const gb)  { ASSERT_INSTR_IMPL(); gb->cpu.AddPC(1); }
void rst_F7(Gameboy* const) { ASSERT_INSTR_IMPL();  }
void ld_F8(Gameboy* const gb)  { ASSERT_INSTR_IMPL(); gb->cpu.AddPC(1); }
void ld_F9(Gameboy* const)  { ASSERT_INSTR_IMPL();  }








void ld_FA(Gameboy* const gb) {
	// LD A, (a16)
	// value in address pointe immediate 16 bit value is stored into A
	// operands: 2
	// clock cycles: 16
	const auto pc = gb->cpu.GetPC();
	const auto a16 = gb->ReadU16(pc);
	const auto value = gb->ReadU8(a16);
	gb->cpu.SetA(value);
	gb->cpu.AddPC(2);

	printf("LD A, (%x); -> A(%x)\n", a16, value);
}









void ei_FB(Gameboy* const) { ASSERT_INSTR_IMPL(); }


// FC MISSING -----
// FD MISSING -----






void cp_FE(Gameboy* const gb) {
	// CP d8
	// compare A with immediate 8 bits value d8, throws alway the result
	// operands: 1
	// clock cycles: 8
	// flags affected: Z 1 H C
	const auto pc = gb->cpu.GetPC();
	const auto a = gb->cpu.GetA();
	const auto d8 = gb->ReadU8(pc);
	const auto result = gb->cpu.SUB(a, d8);
	gb->cpu.AddPC(1);

	printf("CP %x; -> A(%X), Result(%x) | ", d8, a, result);
	gb->cpu.PrintFlags();
}






void rst_FF(Gameboy* const) { ASSERT_INSTR_IMPL();  }




// undefined / unknown opcodes
void unknown(Gameboy* const) {
	// not implemented instruction
	printf("UNDEFINED INSTRUCTION!\n");	
}





const main_instruction_t main_table[256] = {
/*        0        1        2        3        4        5        6        7        8        9        A        B        C        D        E        F      */
/*0*/  nop_00,   ld_01,   ld_02,  inc_03,  inc_04,  dec_05,   ld_06, rlca_07,   ld_08,  add_09,   ld_0A,  dec_0B,  inc_0C,  dec_0D,   ld_0E, rrca_0F,
/*1*/ stop_10,   ld_11,   ld_12,  inc_13,  inc_14,  dec_15,   ld_16,  rla_17,   jr_18,  add_19,   ld_1A,  dec_1B,  inc_1C,  dec_1D,   ld_1E,  rra_1F,
/*2*/   jr_20,   ld_21,   ld_22,  inc_23,  inc_24,  dec_25,   ld_26,  daa_27,   jr_28,  add_29,   ld_2A,  dec_2B,  inc_2C,  dec_2D,   ld_2E,  cpl_2F,
/*3*/   jr_30,   ld_31,   ld_32,  inc_33,  inc_34,  dec_35,   ld_36,  scf_37,   jr_38,  add_39,   ld_3A,  dec_3B,  inc_3C,  dec_3D,   ld_3E,  ccf_3F,
/*4*/   ld_40,   ld_41,   ld_42,   ld_43,   ld_44,   ld_45,   ld_46,   ld_47,   ld_48,   ld_49,   ld_4A,   ld_4B,   ld_4C,   ld_4D,   ld_4E,   ld_4F,
/*5*/   ld_50,   ld_51,   ld_52,   ld_53,   ld_54,   ld_55,   ld_56,   ld_57,   ld_58,   ld_59,   ld_5A,   ld_5B,   ld_5C,   ld_5D,   ld_5E,   ld_5F,
/*6*/   ld_60,   ld_61,   ld_62,   ld_63,   ld_64,   ld_65,   ld_66,   ld_67,   ld_68,   ld_69,   ld_6A,   ld_6B,   ld_6C,   ld_6D,   ld_6E,   ld_6F,
/*7*/   ld_70,   ld_71,   ld_72,   ld_73,   ld_74,   ld_75, halt_76,   ld_77,   ld_78,   ld_79,   ld_7A,   ld_7B,   ld_7C,   ld_7D,   ld_7E,   ld_7F,
/*8*/  add_80,  add_81,  add_82,  add_83,  add_84,  add_85,  add_86,  add_87,  adc_88,  adc_89,  adc_8A,  adc_8B,  adc_8C,  adc_8D,  adc_8E,  adc_8F,
/*9*/  sub_90,  sub_91,  sub_92,  sub_93,  sub_94,  sub_95,  sub_96,  sub_97,  sbc_98,  sbc_99,  sbc_9A,  sbc_9B,  sbc_9C,  sbc_9D,  sbc_9E,  sbc_9F,
/*A*/  and_A0,  and_A1,  and_A2,  and_A3,  and_A4,  and_A5,  and_A6,  and_A7,  xor_A8,  xor_A9,  xor_AA,  xor_AB,  xor_AC,  xor_AD,  xor_AE,  xor_AF,
/*B*/   or_B0,   or_B1,   or_B2,   or_B3,   or_B4,   or_B5,   or_B6,   or_B7,   cp_B8,   cp_B9,   cp_BA,   cp_BB,   cp_BC,   cp_BD,   cp_BE,   cp_BF,
/*C*/  ret_C0,  pop_C1,   jp_C2,   jp_C3, call_C4, push_C5,  add_C6,  rst_C7,  ret_C8,  ret_C9,   jp_CA,PREFIX_CB, call_CC, call_CD, adc_CE,  rst_CF,
/*D*/  ret_D0,  pop_D1,   jp_D2, unknown, call_D4, push_D5,  sub_D6,  rst_D7,  ret_D8, reti_D9,   jp_DA, unknown,  call_DC, unknown, sbc_DE,  rst_DF,
/*E*/  ldh_E0,  pop_E1,   ld_E2, unknown, unknown,  push_E5, and_E6,  rst_E7,  add_E8,   jp_E9,   ld_EA, unknown,  unknown, unknown, xor_EE,  rst_EF,
/*F*/  ldh_F0,  pop_F1,   ld_F2, di_F3,   unknown,  push_F5,  or_F6,  rst_F7,   ld_F8,   ld_F9,   ld_FA,  ei_FB,   unknown, unknown, cp_FE,   rst_FF
};






}

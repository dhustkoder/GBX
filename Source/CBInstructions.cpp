#include "Gameboy.hpp"
#include "Instructions.hpp"

namespace gbx {
        

static uint8_t rlc(const uint8_t value, CPU* const cpu)
{
	// flags effect: Z 0 0 C
	const uint8_t old_bit7 = value & 0x80;
	uint8_t result;
	if (old_bit7) {
		result = (value << 1) | 0x01;
		cpu->f = CheckZ(result) | CPU::Flag_C;
	} else {
		result = value << 1;
		cpu->f = CheckZ(result);
	}

	return result;
}




static uint8_t rrc(const uint8_t value, CPU* const cpu)
{
	// flags effect: Z 0 0 C
	const uint8_t old_bit0 = (value & 0x01);
	uint8_t result;
	if (old_bit0) {
		result = (value >> 1) | 0x80;
		cpu->f = CheckZ(result) | CPU::Flag_C;
	} else {
		result = value >> 1;
		cpu->f =CheckZ(result);
	}

	return result;
}




static uint8_t rl(const uint8_t value, CPU* const cpu)
{
	// flags effect: Z 0 0 C
	const uint8_t old_bit7 = value & 0x80;
	const uint8_t old_carry = cpu->GetFlags(CPU::Flag_C);
	const uint8_t result = old_carry ? (value << 1) | 0x01 : (value << 1);
	if (old_bit7)
		cpu->f = CheckZ(result) | CPU::Flag_C;
	else
		cpu->f = CheckZ(result);

	return result;
}




static uint8_t rr(const uint8_t value, CPU* const cpu)
{
	// flags effect: Z 0 0 C
	const uint8_t old_carry = cpu->GetFlags(CPU::Flag_C);
	const uint8_t old_bit0 = value & 0x01;
	const uint8_t result = old_carry ? ((value >> 1) | 0x80) : (value >> 1);
	if (old_bit0)
		cpu->f = CheckZ(result) | CPU::Flag_C;
	else
		cpu->f = CheckZ(result);

	return result;
}




static uint8_t sla(const uint8_t value, CPU* const cpu)
{
	// flags  effect: Z 0 0 C
	const uint8_t old_bit7 = value & 0x80;
	const uint8_t result = value << 1;
	if (old_bit7)
		cpu->f = CheckZ(result) | CPU::Flag_C;
	else
		cpu->f = CheckZ(result);

	return result;
}



static uint8_t sra(const uint8_t value, CPU* const cpu)
{
	// flags effect: Z 0 0 C
	const uint8_t old_bit0 = value & 0x01;
	const uint8_t result = (value & 0x80) | (value >> 1);
	if (old_bit0)
		cpu->f = CheckZ(result) | CPU::Flag_C;
	else
		cpu->f = CheckZ(result);

	return result;
}




static uint8_t swap(const uint8_t value, CPU* const cpu)
{
	// flags effect: Z 0 0 0
	const uint8_t result = ((value & 0x0f) << 4) | ((value & 0xf0) >> 4);
	cpu->f = CheckZ(result);
	return result;
}




static uint8_t srl(const uint8_t value, CPU* const cpu)
{
	// flags effect: Z 0 0 C
	const uint8_t old_bit0 = value & 0x01;
	const uint8_t result = value >> 1;
	if (old_bit0)
		cpu->f = CheckZ(result) | CPU::Flag_C;
	else
		cpu->f = CheckZ(result);

	return result;
}




static void bit_n(const uint8_t bit, const uint8_t value, CPU* const cpu)
{
	// flags effect: Z 0 1 -
	cpu->f = CheckZ(value & (0x01 << bit)) | CPU::Flag_H | 
	                cpu->GetFlags(CPU::Flag_C);
}



inline void rlc_r(uint8_t* const reg, CPU* const cpu) {
	*reg = rlc(*reg, cpu);
}

inline void rrc_r(uint8_t* const reg, CPU* const cpu) {
	*reg = rrc(*reg, cpu);
}

inline void rl_r(uint8_t* const reg, CPU* const cpu) {
	*reg = rl(*reg, cpu);
}

inline void rr_r(uint8_t* const reg, CPU* const cpu) {
	*reg = rr(*reg, cpu);
}

inline void sla_r(uint8_t* const reg, CPU* const cpu) {
	*reg = sla(*reg, cpu);
}

inline void sra_r(uint8_t* const reg, CPU* const cpu) {
	*reg = sra(*reg, cpu);
}

inline void swap_r(uint8_t* const reg, CPU* const cpu) {
	*reg = swap(*reg, cpu);
}

inline void srl_r(uint8_t* const reg, CPU* const cpu) {
	*reg = srl(*reg, cpu);
}


inline void op_hlp(uint8_t(*op)(uint8_t,CPU*), Gameboy* const gb)
{
	const uint16_t hl = gb->cpu.hl;
	const uint8_t result = op(gb->Read8(hl), &gb->cpu);
	gb->Write8(hl, result);
}



inline void set_r(const uint8_t bit, uint8_t* const reg) {
	*reg = SetBit(bit, *reg);
}


inline void res_r(const uint8_t bit, uint8_t* const reg) {
	*reg = ResBit(bit, *reg);
}


inline void set_hlp(const uint8_t bit, Gameboy* const gb)
{
	const uint16_t hl = gb->cpu.hl;
	const uint8_t result = SetBit(bit, gb->Read8(hl));
	gb->Write8(hl, result);
}


inline void res_hlp(const uint8_t bit, Gameboy* const gb)
{
	const uint16_t hl = gb->cpu.hl;
	const uint8_t result = ResBit(bit, gb->Read8(hl));
	gb->Write8(hl, result);
}



inline void bit_r(const uint8_t bit, const uint8_t reg, CPU* const cpu) {
	bit_n(bit, reg, cpu);
}

inline void bit_hlp(const uint8_t bit, Gameboy* const gb) {
	bit_n(bit, gb->Read8(gb->cpu.hl), &gb->cpu);
}







// CB Instructions Implementation:
// 0x00
void rlc_00(Gameboy* const gb)
{
	// RLC B
	rlc_r(&gb->cpu.b, &gb->cpu);
}

void rlc_01(Gameboy* const gb)
{
	// RLC C
	rlc_r(&gb->cpu.c, &gb->cpu);
}

void rlc_02(Gameboy* const gb)
{
	// RLC D
	rlc_r(&gb->cpu.d, &gb->cpu);
}

void rlc_03(Gameboy* const gb)
{
	// RLC E
	rlc_r(&gb->cpu.e, &gb->cpu);
}

void rlc_04(Gameboy* const gb)
{
	// RLC H
	rlc_r(&gb->cpu.h, &gb->cpu);
}

void rlc_05(Gameboy* const gb)
{
	// RLC L
	rlc_r(&gb->cpu.l, &gb->cpu);
}

void rlc_06(Gameboy* const gb)
{
	// RLC (HL)
	op_hlp(rlc, gb);
}

void rlc_07(Gameboy* const gb)
{
	// RLC A
	rlc_r(&gb->cpu.a, &gb->cpu);
}





void rrc_08(Gameboy* const gb)
{
	// RRC B
	rrc_r(&gb->cpu.b, &gb->cpu);
}

void rrc_09(Gameboy* const gb)
{
	// RRC C
	rrc_r(&gb->cpu.c, &gb->cpu);
}

void rrc_0A(Gameboy* const gb)
{
	// RRC D
	rrc_r(&gb->cpu.d, &gb->cpu);
}

void rrc_0B(Gameboy* const gb)
{
	// RRC E
	rrc_r(&gb->cpu.e, &gb->cpu);
}

void rrc_0C(Gameboy* const gb)
{
	// RRC H
	rrc_r(&gb->cpu.h, &gb->cpu);
}

void rrc_0D(Gameboy* const gb)
{
	// RRC L
	rrc_r(&gb->cpu.l, &gb->cpu);
}

void rrc_0E(Gameboy* const gb)
{
	// RRC (HL)
	op_hlp(rrc, gb);
}

void rrc_0F(Gameboy* const gb)
{
	// RRC A
	rrc_r(&gb->cpu.a, &gb->cpu);
}





// 0x10
void rl_10(Gameboy* const gb)
{
	// RL B
	rl_r(&gb->cpu.b, &gb->cpu);
}


void rl_11(Gameboy* const gb)
{
	// RL C
	rl_r(&gb->cpu.c, &gb->cpu);
}

void rl_12(Gameboy* const gb)
{
	// RL D
	rl_r(&gb->cpu.d, &gb->cpu);
}

void rl_13(Gameboy* const gb)
{
	// RL E
	rl_r(&gb->cpu.e, &gb->cpu);
}

void rl_14(Gameboy* const gb)
{
	// RL H
	rl_r(&gb->cpu.h, &gb->cpu);
}

void rl_15(Gameboy* const gb)
{
	// RL L
	rl_r(&gb->cpu.l, &gb->cpu);
}

void rl_16(Gameboy* const gb)
{
	// RL ( HL )
	op_hlp(rl, gb);
}

void rl_17(Gameboy* const gb)
{
	// RL A
	rl_r(&gb->cpu.a, &gb->cpu);
}





void rr_18(Gameboy* const gb)
{
	// RR B
	rr_r(&gb->cpu.b, &gb->cpu);
}

void rr_19(Gameboy* const gb)
{
	// RR C
	rr_r(&gb->cpu.c, &gb->cpu);
}

void rr_1A(Gameboy* const gb)
{
	// RR D
	rr_r(&gb->cpu.d, &gb->cpu);
}

void rr_1B(Gameboy* const gb)
{
	// RR E
	rr_r(&gb->cpu.e, &gb->cpu);
}

void rr_1C(Gameboy* const gb)
{
	// RR H
	rr_r(&gb->cpu.h, &gb->cpu);
}

void rr_1D(Gameboy* const gb)
{
	// RR L
	rr_r(&gb->cpu.l, &gb->cpu);
}

void rr_1E(Gameboy* const gb)
{
	// RR (HL)
	op_hlp(rr, gb);
}

void rr_1F(Gameboy* const gb)
{
	// RR A
	rr_r(&gb->cpu.a, &gb->cpu);
}






// 0x20
void sla_20(Gameboy* const gb)
{
	// SLA B
	sla_r(&gb->cpu.b, &gb->cpu);
}

void sla_21(Gameboy* const gb)
{
	// SLA C
	sla_r(&gb->cpu.c, &gb->cpu);
}

void sla_22(Gameboy* const gb)
{
	// SLA D
	sla_r(&gb->cpu.d, &gb->cpu);
}

void sla_23(Gameboy* const gb)
{
	// SLA E
	sla_r(&gb->cpu.e, &gb->cpu);
}

void sla_24(Gameboy* const gb)
{
	// SLA H
	sla_r(&gb->cpu.h, &gb->cpu);
}

void sla_25(Gameboy* const gb)
{
	// SLA L
	sla_r(&gb->cpu.l, &gb->cpu);
}

void sla_26(Gameboy* const gb)
{ 
	// SLA (HL)
	op_hlp(sla, gb);
}

void sla_27(Gameboy* const gb)
{
	// SLA A
	sla_r(&gb->cpu.a, &gb->cpu);
}





void sra_28(Gameboy* const gb)
{
	// SRA B
	sra_r(&gb->cpu.b, &gb->cpu);
}


void sra_29(Gameboy* const gb)
{
	// SRA C
	sra_r(&gb->cpu.c, &gb->cpu);
}

void sra_2A(Gameboy* const gb)
{
	// SRA D
	sra_r(&gb->cpu.d, &gb->cpu);
}

void sra_2B(Gameboy* const gb)
{
	// SRA E
	sra_r(&gb->cpu.e, &gb->cpu);
}

void sra_2C(Gameboy* const gb)
{
	// SRA H
	sra_r(&gb->cpu.h, &gb->cpu);
}

void sra_2D(Gameboy* const gb)
{
	// SRA L
	sra_r(&gb->cpu.l, &gb->cpu);
}

void sra_2E(Gameboy* const gb)
{
	// SRA ( HL )
	op_hlp(sra, gb);
}

void sra_2F(Gameboy* const gb)
{
	// SRA A
	sra_r(&gb->cpu.a, &gb->cpu);
}




// 0x30
void swap_30(Gameboy* const gb)
{
	// SWAP B
	swap_r(&gb->cpu.b, &gb->cpu);
}

void swap_31(Gameboy* const gb)
{
	//  SWAP C
	swap_r(&gb->cpu.c, &gb->cpu);
}

void swap_32(Gameboy* const gb)
{
	// SWAP D
	swap_r(&gb->cpu.d, &gb->cpu);
}

void swap_33(Gameboy* const gb)
{
	// SWAP E
	swap_r(&gb->cpu.e, &gb->cpu);
}

void swap_34(Gameboy* const gb)
{
	// SWAP H
	swap_r(&gb->cpu.h, &gb->cpu);
}

void swap_35(Gameboy* const gb)
{
	// SWAP L
	swap_r(&gb->cpu.l, &gb->cpu);
}

void swap_36(Gameboy* const gb)
{
	// SWAP ( HL )
	op_hlp(swap, gb);
}


void swap_37(Gameboy* const gb)
{
	// SWAP A
	swap_r(&gb->cpu.a, &gb->cpu);
}





void srl_38(Gameboy* const gb)
{
	// SRL B
	srl_r(&gb->cpu.b, &gb->cpu);
}

void srl_39(Gameboy* const gb)
{
	// SRL C
	srl_r(&gb->cpu.c, &gb->cpu);
}

void srl_3A(Gameboy* const gb)
{
	// SRL D
	srl_r(&gb->cpu.d, &gb->cpu);
}

void srl_3B(Gameboy* const gb)
{
	// SRL E
	srl_r(&gb->cpu.e, &gb->cpu);
}

void srl_3C(Gameboy* const gb)
{
	// SRL H
	srl_r(&gb->cpu.h, &gb->cpu);
}

void srl_3D(Gameboy* const gb)
{
	// SRL L
	srl_r(&gb->cpu.l, &gb->cpu);
}

void srl_3E(Gameboy* const gb)
{
	// SRL (HL)
	op_hlp(srl, gb);
}

void srl_3F(Gameboy* const gb)
{ 
	// SRL A
	srl_r(&gb->cpu.a, &gb->cpu);
}






// 0x40
void bit_40(Gameboy* const gb)
{ 
	// BIT 0, B
	bit_r(0, gb->cpu.b, &gb->cpu);
}



void bit_41(Gameboy* const gb) 
{ 
	// BIT 0, C
	bit_r(0, gb->cpu.c, &gb->cpu);
}


void bit_42(Gameboy* const gb)
{
	// BIT 0, D
	bit_r(0, gb->cpu.d, &gb->cpu);
}


void bit_43(Gameboy* const gb)
{
	// BIT 0, E
	bit_r(0, gb->cpu.e, &gb->cpu);
}


void bit_44(Gameboy* const gb)
{
	// BIT 0, H
	bit_r(0, gb->cpu.h, &gb->cpu);
}


void bit_45(Gameboy* const gb)
{
	// BIT 0, L
	bit_r(0, gb->cpu.l, &gb->cpu);
}



void bit_46(Gameboy* const gb)
{ 
	// BIT 0, (HL)
	bit_hlp(0, gb);
}



void bit_47(Gameboy* const gb)
{ 
	// BIT 0, A
	bit_r(0, gb->cpu.a, &gb->cpu);
}



void bit_48(Gameboy* const gb)
{ 
	// BIT 1, B
	bit_r(1, gb->cpu.b, &gb->cpu);
}



void bit_49(Gameboy* const gb)
{
	// BIT 1, C
	bit_r(1, gb->cpu.c, &gb->cpu);
}


void bit_4A(Gameboy* const gb)
{
	// BIT 1, D
	bit_r(1, gb->cpu.d, &gb->cpu);
}


void bit_4B(Gameboy* const gb)
{
	// BIT 1, E
	bit_r(1, gb->cpu.e, &gb->cpu);
}


void bit_4C(Gameboy* const gb)
{
	// BIT 1, H
	bit_r(1, gb->cpu.h, &gb->cpu);
}

void bit_4D(Gameboy* const gb)
{
	// BIT 1, L
	bit_r(1, gb->cpu.l, &gb->cpu);
}

void bit_4E(Gameboy* const gb)
{
	// BIT 1 , (HL)
	bit_hlp(1, gb);
}

void bit_4F(Gameboy* const gb)
{ 
	// BIT 1, A
	bit_r(1, gb->cpu.a, &gb->cpu);
}






// 0x50
void bit_50(Gameboy* const gb)
{ 
	// BIT 2, B
	bit_r(2, gb->cpu.b, &gb->cpu);
}


void bit_51(Gameboy* const gb)
{
	// BIT 2, C
	bit_r(2, gb->cpu.c, &gb->cpu);
}


void bit_52(Gameboy* const gb)
{
	// BIT 2, D
	bit_r(2, gb->cpu.d, &gb->cpu);
}


void bit_53(Gameboy* const gb)
{
	// BIT 2, E
	bit_r(2, gb->cpu.e, &gb->cpu);
}

void bit_54(Gameboy* const gb)
{
	// BIT 2, H
	bit_r(2, gb->cpu.h, &gb->cpu);
}

void bit_55(Gameboy* const gb)
{
	// BIT 2, L
	bit_r(2, gb->cpu.l, &gb->cpu);
}

void bit_56(Gameboy* const gb)
{
	// BIT 2, (HL)
	bit_hlp(2, gb);
}


void bit_57(Gameboy* const gb) 
{ 
	// BIT 2, A
	bit_r(2, gb->cpu.a, &gb->cpu);
}


void bit_58(Gameboy* const gb) 
{ 
	// BIT 3, B
	bit_r(3, gb->cpu.b, &gb->cpu);
}


void bit_59(Gameboy* const gb)
{
	// BIT 3, C
	bit_r(3, gb->cpu.c, &gb->cpu);
}


void bit_5A(Gameboy* const gb)
{
	// BIT 3, D
	bit_r(3, gb->cpu.d, &gb->cpu);
}


void bit_5B(Gameboy* const gb)
{
	// BIT 3, E
	bit_r(3, gb->cpu.e, &gb->cpu);
}


void bit_5C(Gameboy* const gb)
{
	// BIT 3, H
	bit_r(3, gb->cpu.h, &gb->cpu);
}


void bit_5D(Gameboy* const gb)
{
	// BIT 3, L
	bit_r(3, gb->cpu.l, &gb->cpu);
}

void bit_5E(Gameboy* const gb)
{
	// BIT 3, (HL)
	bit_hlp(3, gb);
}


void bit_5F(Gameboy* const gb)
{
	// BIT 3, A
	bit_r(3, gb->cpu.a, &gb->cpu);
}




// 0x60
void bit_60(Gameboy* const gb) 
{ 
	// BIT 4, B
	bit_r(4, gb->cpu.b, &gb->cpu);
}



void bit_61(Gameboy* const gb)
{ 
	// BIT 4, C
	bit_r(4, gb->cpu.c, &gb->cpu);
}


void bit_62(Gameboy* const gb)
{
	// BIT 4, D
	bit_r(4, gb->cpu.d, &gb->cpu);
}


void bit_63(Gameboy* const gb)
{
	// BIT 4, E
	bit_r(4, gb->cpu.e, &gb->cpu);
}


void bit_64(Gameboy* const gb)
{
	// BIT 4, H
	bit_r(4, gb->cpu.h, &gb->cpu);
}


void bit_65(Gameboy* const gb)
{
	// BIT 4, L
	bit_r(4, gb->cpu.l, &gb->cpu);
}


void bit_66(Gameboy* const gb)
{
	// BIT 4, (HL)
	bit_hlp(4, gb);
}


void bit_67(Gameboy* const gb)
{
	// BIT 4, A
	bit_r(4, gb->cpu.a, &gb->cpu);
}




void bit_68(Gameboy* const gb)
{ 
	// BIT 5, B
	bit_r(5, gb->cpu.b, &gb->cpu);
}





void bit_69(Gameboy* const gb)
{ 
	// BIT 5, C
	bit_r(5, gb->cpu.c, &gb->cpu);
}


void bit_6A(Gameboy* const gb)
{
	// BIT 5, D
	bit_r(5, gb->cpu.d, &gb->cpu);
}


void bit_6B(Gameboy* const gb)
{
	// BIT 5, E
	bit_r(5, gb->cpu.e, &gb->cpu);
}


void bit_6C(Gameboy* const gb)
{
	// BIT 5, H
	bit_r(5, gb->cpu.h, &gb->cpu);
}


void bit_6D(Gameboy* const gb)
{
	// BIT 5, L
	bit_r(5, gb->cpu.l, &gb->cpu);
}

void bit_6E(Gameboy* const gb)
{
	// BIT 5, (HL)
	bit_hlp(5, gb);
}

void bit_6F(Gameboy* const gb)
{ 
	// BIT 5, A
	bit_r(5, gb->cpu.a, &gb->cpu);
}





// 0x70
void bit_70(Gameboy* const gb) 
{
	// BIT 6, B
	bit_r(6, gb->cpu.b, &gb->cpu);
}



void bit_71(Gameboy* const gb)
{ 
	// BIT 6, C
	bit_r(6, gb->cpu.c, &gb->cpu);
}


void bit_72(Gameboy* const gb)
{
	// BIT 6, D
	bit_r(6, gb->cpu.d, &gb->cpu);
}


void bit_73(Gameboy* const gb)
{
	// BIT 6, E
	bit_r(6, gb->cpu.e, &gb->cpu);
}


void bit_74(Gameboy* const gb)
{
	// BIT 6, H
	bit_r(6, gb->cpu.h, &gb->cpu);
}


void bit_75(Gameboy* const gb)
{
	// BIT 6, L
	bit_r(6, gb->cpu.l, &gb->cpu);
}


void bit_76(Gameboy* const gb) 
{
	// BIT 6, (HL)
	bit_hlp(6, gb);
}



void bit_77(Gameboy* const gb)
{ 
	// BIT 6, A
	bit_r(6, gb->cpu.a, &gb->cpu);
}



void bit_78(Gameboy* const gb) 
{ 
	// BIT 7, B
	bit_r(7, gb->cpu.b, &gb->cpu);
}


void bit_79(Gameboy* const gb) 
{ 
	// BIT 7, C
	bit_r(7, gb->cpu.c, &gb->cpu);
}



void bit_7A(Gameboy* const gb)
{
	// BIT 7, D
	bit_r(7, gb->cpu.d, &gb->cpu);
}


void bit_7B(Gameboy* const gb)
{
	// BIT 7, E
	bit_r(7, gb->cpu.e, &gb->cpu);
}


void bit_7C(Gameboy* const gb)
{
	// BIT 7, H
	bit_r(7, gb->cpu.h, &gb->cpu);
}


void bit_7D(Gameboy* const gb)
{
	// BIT 7, L
	bit_r(7, gb->cpu.l, &gb->cpu);
}



void bit_7E(Gameboy* const gb)
{
	// BIT 7, (HL)
	bit_hlp(7, gb);
}



void bit_7F(Gameboy* const gb)
{ 
	// BIT 7, A
	bit_r(7, gb->cpu.a, &gb->cpu);
}






// 0x80
void res_80(Gameboy* const gb)
{
	// RES 0, B
	res_r(0, &gb->cpu.b);
}


void res_81(Gameboy* const gb)
{
	// RES 0, C 
	res_r(0, &gb->cpu.c);
}


void res_82(Gameboy* const gb)
{
	// RES 0, D
	res_r(0, &gb->cpu.d);
}


void res_83(Gameboy* const gb)
{
	// RES 0, E
	res_r(0, &gb->cpu.e);
}


void res_84(Gameboy* const gb)
{
	// RES 0, H
	res_r(0, &gb->cpu.h);
}


void res_85(Gameboy* const gb)
{
	// RES 0, L
	res_r(0, &gb->cpu.l);
}


void res_86(Gameboy* const gb)
{ 
	// RES 0, (HL)
	res_hlp(0, gb);
}



void res_87(Gameboy* const gb)
{
	// RES 0, A
	res_r(0, &gb->cpu.a);
}


void res_88(Gameboy* const gb)
{
	// RES 1, B
	res_r(1, &gb->cpu.b);
}


void res_89(Gameboy* const gb)
{
	// RES 1, C
	res_r(1, &gb->cpu.c);
}


void res_8A(Gameboy* const gb)
{
	// RES 1, D
	res_r(1, &gb->cpu.d);
}


void res_8B(Gameboy* const gb)
{
	// RES 1, E
	res_r(1, &gb->cpu.e);
}


void res_8C(Gameboy* const gb)
{
	// RES 1, H
	res_r(1, &gb->cpu.h);
}


void res_8D(Gameboy* const gb)
{
	// RES 1, L
	res_r(1, &gb->cpu.l);
}

void res_8E(Gameboy* const gb)
{
	// RES 1, (HL)
	res_hlp(1, gb);
}


void res_8F(Gameboy* const gb)
{
	// RES 1, A
	res_r(1, &gb->cpu.a);
}



// 0x90
void res_90(Gameboy* const gb)
{
	// RES 2, B
	res_r(2, &gb->cpu.b);
}


void res_91(Gameboy* const gb)
{
	// RES 2, C
	res_r(2, &gb->cpu.c);
}


void res_92(Gameboy* const gb)
{
	// RES 2, D
	res_r(2, &gb->cpu.d);
}


void res_93(Gameboy* const gb)
{
	// RES 2, E
	res_r(2, &gb->cpu.e);
}


void res_94(Gameboy* const gb)
{
	// RES 2, H
	res_r(2, &gb->cpu.h);
}


void res_95(Gameboy* const gb)
{
	// RES 2, L
	res_r(2, &gb->cpu.l);
}


void res_96(Gameboy* const gb)
{
	// RES 2, (HL)
	res_hlp(2, gb);
}


void res_97(Gameboy* const gb)
{
	// RES 2, A
	res_r(2, &gb->cpu.a);
}


void res_98(Gameboy* const gb)
{
	// RES 3, B
	res_r(3, &gb->cpu.b);
}


void res_99(Gameboy* const gb)
{
	// RES 3, C
	res_r(3, &gb->cpu.c);
}


void res_9A(Gameboy* const gb)
{
	// RES 3, D
	res_r(3, &gb->cpu.d);
}


void res_9B(Gameboy* const gb)
{
	// RES 3, E
	res_r(3, &gb->cpu.e);
}


void res_9C(Gameboy* const gb)
{
	// RES 3, H
	res_r(3, &gb->cpu.h);
}


void res_9D(Gameboy* const gb)
{
	// RES 3, L
	res_r(3, &gb->cpu.l);
}


void res_9E(Gameboy* const gb) 
{ 
	// RES 3, (HL)
	res_hlp(3, gb);
}


void res_9F(Gameboy* const gb)
{
	// RES 3, A
	res_r(3, &gb->cpu.a);
}



// 0xA0
void res_A0(Gameboy* const gb)
{
	// RES 4, B
	res_r(4, &gb->cpu.b);
}


void res_A1(Gameboy* const gb)
{
	// RES 4, C
	res_r(4, &gb->cpu.c);
}


void res_A2(Gameboy* const gb)
{
	// RES 4, D
	res_r(4, &gb->cpu.d);
}


void res_A3(Gameboy* const gb)
{
	// RES 4, E
	res_r(4, &gb->cpu.e);
}


void res_A4(Gameboy* const gb)
{
	// RES 4, H
	res_r(4, &gb->cpu.h);
}


void res_A5(Gameboy* const gb)
{
	// RES 4, L
	res_r(4, &gb->cpu.l);
}

void res_A6(Gameboy* const gb)
{
	// RES 4, (HL)
	res_hlp(4, gb);
}


void res_A7(Gameboy* const gb)
{
	// RES 4, A
	res_r(4, &gb->cpu.a);
}


void res_A8(Gameboy* const gb)
{
	// RES 5, B
	res_r(5, &gb->cpu.b);
}


void res_A9(Gameboy* const gb)
{
	// RES 5, C
	res_r(5, &gb->cpu.c);
}


void res_AA(Gameboy* const gb)
{
	// RES 5, D
	res_r(5, &gb->cpu.d);
}


void res_AB(Gameboy* const gb)
{
	// RES 5, E
	res_r(5, &gb->cpu.e);
}


void res_AC(Gameboy* const gb)
{
	// RES 5, H
	res_r(5, &gb->cpu.h);
}


void res_AD(Gameboy* const gb)
{
	// RES 5, L
	res_r(5, &gb->cpu.l);
}


void res_AE(Gameboy* const gb)
{
	// RES 5, (HL)
	res_hlp(5, gb);
}


void res_AF(Gameboy* const gb)
{
	// RES 5, A
	res_r(5, &gb->cpu.a);
}




// 0xB0
void res_B0(Gameboy* const gb) 
{
	// RES 6, B
	res_r(6, &gb->cpu.b);
}


void res_B1(Gameboy* const gb)
{
	// RES 6, C
	res_r(6, &gb->cpu.c);
}


void res_B2(Gameboy* const gb)
{
	// RES 6, D
	res_r(6, &gb->cpu.d);
}


void res_B3(Gameboy* const gb)
{
	// RES 6, E
	res_r(6, &gb->cpu.e);
}


void res_B4(Gameboy* const gb)
{
	// RES 6, H
	res_r(6, &gb->cpu.h);
}

void res_B5(Gameboy* const gb)
{
	// RES 6, L
	res_r(6, &gb->cpu.l);
}

void res_B6(Gameboy* const gb)
{
	// RES 6, (HL)
	res_hlp(6, gb);
}

void res_B7(Gameboy* const gb)
{
	// RES 6, A
	res_r(6, &gb->cpu.a);
}


void res_B8(Gameboy* const gb)
{
	// RES 7, B
	res_r(7, &gb->cpu.b);
}


void res_B9(Gameboy* const gb)
{
	// RES 7, C
	res_r(7, &gb->cpu.c);
}


void res_BA(Gameboy* const gb)
{
	// RES 7, D
	res_r(7, &gb->cpu.d);
}


void res_BB(Gameboy* const gb)
{
	// RES 7, E
	res_r(7, &gb->cpu.e);
}


void res_BC(Gameboy* const gb)
{
	// RES 7, H
	res_r(7, &gb->cpu.h);
}


void res_BD(Gameboy* const gb)
{
	// RES 7, L
	res_r(7, &gb->cpu.l);
}


void res_BE(Gameboy* const gb) 
{ 
	// RES 7, (HL)
	res_hlp(7, gb);
}


void res_BF(Gameboy* const gb)
{
	// RES 7, A
	res_r(7, &gb->cpu.a);
}




// 0xC0
void set_C0(Gameboy* const gb)
{
	// SET 0, B
	set_r(0, &gb->cpu.b);
}


void set_C1(Gameboy* const gb)
{
	// SET 0, C
	set_r(0, &gb->cpu.c);
}


void set_C2(Gameboy* const gb)
{
	// SET 0, D
	set_r(0, &gb->cpu.d);
}


void set_C3(Gameboy* const gb)
{
	// SET 0, E
	set_r(0, &gb->cpu.e);
}


void set_C4(Gameboy* const gb)
{
	// SET 0, H
	set_r(0, &gb->cpu.h);
}


void set_C5(Gameboy* const gb)
{
	// SET 0, L
	set_r(0, &gb->cpu.l);
}


void set_C6(Gameboy* const gb)
{
	// SET 0, (HL)
	set_hlp(0, gb);
}


void set_C7(Gameboy* const gb)
{
	// SET 0, A
	set_r(0, &gb->cpu.a);
}


void set_C8(Gameboy* const gb)
{
	// SET 1, B
	set_r(1, &gb->cpu.b);
}


void set_C9(Gameboy* const gb)
{
	// SET 1, C
	set_r(1, &gb->cpu.c);
}

void set_CA(Gameboy* const gb)
{
	// SET 1, D
	set_r(1, &gb->cpu.d);
}


void set_CB(Gameboy* const gb)
{
	// SET 1, E
	set_r(1, &gb->cpu.e);
}


void set_CC(Gameboy* const gb)
{
	// SET 1, H
	set_r(1, &gb->cpu.h);
}


void set_CD(Gameboy* const gb)
{
	// SET 1, L
	set_r(1, &gb->cpu.l);
}


void set_CE(Gameboy* const gb)
{
	// SET 1, (HL)
	set_hlp(1, gb);
}


void set_CF(Gameboy* const gb)
{
	// SET 1, A
	set_r(1, &gb->cpu.a);
}



// 0xD0
void set_D0(Gameboy* const gb) 
{ 
	// SET 2, B
	set_r(2, &gb->cpu.b);
}


void set_D1(Gameboy* const gb)
{
	// SET 2, C
	set_r(2, &gb->cpu.c);
}

void set_D2(Gameboy* const gb)
{
	// SET 2, D
	set_r(2, &gb->cpu.d);
}


void set_D3(Gameboy* const gb)
{
	// SET 2, E
	set_r(2, &gb->cpu.e);
}


void set_D4(Gameboy* const gb)
{
	// SET 2, H
	set_r(2, &gb->cpu.h);
}


void set_D5(Gameboy* const gb)
{
	// SET 2, L
	set_r(2, &gb->cpu.l);
}


void set_D6(Gameboy* const gb)
{
	// SET 2, (HL)
	set_hlp(2, gb);
}


void set_D7(Gameboy* const gb)
{ 
	// SET 2, A
	set_r(2, &gb->cpu.a);
}


void set_D8(Gameboy* const gb)
{ 
	// SET 3, B
	set_r(3, &gb->cpu.b);
}


void set_D9(Gameboy* const gb)
{
	// SET 3, C
	set_r(3, &gb->cpu.c);
}

void set_DA(Gameboy* const gb)
{
	// SET 3, D
	set_r(3, &gb->cpu.d);
}

void set_DB(Gameboy* const gb)
{
	// SET 3, E
	set_r(3, &gb->cpu.e);
}

void set_DC(Gameboy* const gb)
{
	// SET 3, H
	set_r(3, &gb->cpu.h);
}

void set_DD(Gameboy* const gb)
{
	// SET 3, L
	set_r(3, &gb->cpu.l);
}


void set_DE(Gameboy* const gb) 
{ 
	// SET 3, (HL)
	set_hlp(3, gb);
}


void set_DF(Gameboy* const gb)
{
	// SET 3, A
	set_r(3, &gb->cpu.a);
}



// 0xE0
void set_E0(Gameboy* const gb)
{
	// SET 4, B
	set_r(4, &gb->cpu.b);
}


void set_E1(Gameboy* const gb)
{
	// SET 4, C
	set_r(4, &gb->cpu.c);
}


void set_E2(Gameboy* const gb)
{
	// SET 4, D
	set_r(4, &gb->cpu.d);
}


void set_E3(Gameboy* const gb)
{
	// SET 4, E
	set_r(4, &gb->cpu.e);
}


void set_E4(Gameboy* const gb)
{
	// SET 4, H
	set_r(4, &gb->cpu.h);
}

void set_E5(Gameboy* const gb)
{
	// SET 4, L
	set_r(4, &gb->cpu.l);
}

void set_E6(Gameboy* const gb)
{
	// SET 4, (HL)
	set_hlp(4, gb);
}

void set_E7(Gameboy* const gb)
{
	// SET 4, A
	set_r(4, &gb->cpu.a);
}



void set_E8(Gameboy* const gb)
{
	// SET 5, B
	set_r(5, &gb->cpu.b);
}


void set_E9(Gameboy* const gb)
{
	// SET 5, C
	set_r(5, &gb->cpu.c);
}


void set_EA(Gameboy* const gb)
{
	// SET 5, D
	set_r(5, &gb->cpu.d);
}


void set_EB(Gameboy* const gb)
{
	// SET 5, E
	set_r(5, &gb->cpu.e);
}


void set_EC(Gameboy* const gb)
{
	// SET 5, H
	set_r(5, &gb->cpu.h);
}

void set_ED(Gameboy* const gb)
{
	// SET 5, L
	set_r(5, &gb->cpu.l);
}

void set_EE(Gameboy* const gb)
{
	// SET 5, (HL)
	set_hlp(5, gb);
}

void set_EF(Gameboy* const gb)
{
	// SET 5, A
	set_r(5, &gb->cpu.a);
}





// 0xF0
void set_F0(Gameboy* const gb)
{ 
	// SET 6, B
	set_r(6, &gb->cpu.b);
}


void set_F1(Gameboy* const gb)
{
	// SET 6, C
	set_r(6, &gb->cpu.c);
}


void set_F2(Gameboy* const gb)
{
	// SET 6, D
	set_r(6, &gb->cpu.d);
}


void set_F3(Gameboy* const gb)
{
	// SET 6, E
	set_r(6, &gb->cpu.e);
}


void set_F4(Gameboy* const gb)
{
	// SET 6, H
	set_r(6, &gb->cpu.h);
}


void set_F5(Gameboy* const gb)
{
	// SET 6, L
	set_r(6, &gb->cpu.l);
}


void set_F6(Gameboy* const gb)
{
	// SET 6, (HL)
	set_hlp(6, gb);
}


void set_F7(Gameboy* const gb)
{
	// SET 6, A
	set_r(6, &gb->cpu.a);
}


void set_F8(Gameboy* const gb)
{ 
	// SET 7, B
	set_r(7, &gb->cpu.b);
}


void set_F9(Gameboy* const gb)
{
	// SET 7, C
	set_r(7, &gb->cpu.c);
}


void set_FA(Gameboy* const gb)
{
	// SET 7, D
	set_r(7, &gb->cpu.d);
}


void set_FB(Gameboy* const gb)
{
	// SET 7, E
	set_r(7, &gb->cpu.e);
}


void set_FC(Gameboy* const gb)
{
	// SET 7, H
	set_r(7, &gb->cpu.h);
}


void set_FD(Gameboy* const gb)
{
	// SET 7, L
	set_r(7, &gb->cpu.l);
}


void set_FE(Gameboy* const gb)
{ 
	// SET 7, (HL)
	set_hlp(7, gb);
}


void set_FF(Gameboy* const gb)
{
	// SET 7, A
	set_r(7, &gb->cpu.a);
}
    
    




const instruction_table_t cb_instruction[256] = {
/*        0        1        2        3        4        5        6        7        8        9        A        B        C        D        E        F      */
/*0*/  rlc_00,  rlc_01,  rlc_02,  rlc_03,  rlc_04,  rlc_05,  rlc_06,  rlc_07,  rrc_08,  rrc_09,   rrc_0A, rrc_0B,  rrc_0C,  rrc_0D,  rrc_0E,  rrc_0F,
/*1*/   rl_10,   rl_11,   rl_12,   rl_13,   rl_14,   rl_15,   rl_16,   rl_17,   rr_18,   rr_19,   rr_1A,   rr_1B,   rr_1C,   rr_1D,   rr_1E,   rr_1F,
/*2*/  sla_20,  sla_21,  sla_22,  sla_23,  sla_24,  sla_25,  sla_26,  sla_27,  sra_28,  sra_29,   sra_2A,  sra_2B, sra_2C,  sra_2D,  sra_2E,  sra_2F,
/*3*/ swap_30, swap_31, swap_32, swap_33, swap_34, swap_35, swap_36, swap_37,  srl_38,  srl_39,  srl_3A,  srl_3B,  srl_3C,  srl_3D,  srl_3E,  srl_3F,
/*4*/  bit_40,  bit_41,  bit_42,  bit_43,  bit_44,  bit_45,  bit_46,  bit_47,  bit_48,  bit_49,  bit_4A,  bit_4B,  bit_4C,  bit_4D,  bit_4E,  bit_4F,
/*5*/  bit_50,  bit_51,  bit_52,  bit_53,  bit_54,  bit_55,  bit_56,  bit_57,  bit_58,  bit_59,  bit_5A,  bit_5B,  bit_5C,  bit_5D,  bit_5E,  bit_5F,
/*6*/  bit_60,  bit_61,  bit_62,  bit_63,  bit_64,  bit_65,  bit_66,  bit_67,  bit_68,  bit_69,  bit_6A,  bit_6B,  bit_6C,  bit_6D,  bit_6E,  bit_6F,
/*7*/  bit_70,  bit_71,  bit_72,  bit_73,  bit_74,  bit_75,  bit_76,  bit_77,  bit_78,  bit_79,  bit_7A,  bit_7B,  bit_7C,  bit_7D,  bit_7E,  bit_7F,
/*8*/  res_80,  res_81,  res_82,  res_83,  res_84,  res_85,  res_86,  res_87,  res_88,  res_89,  res_8A,  res_8B,  res_8C,  res_8D,  res_8E,  res_8F,
/*9*/  res_90,  res_91,  res_92,  res_93,  res_94,  res_95,  res_96,  res_97,  res_98,  res_99,  res_9A,  res_9B,  res_9C,  res_9D,  res_9E,  res_9F,
/*A*/  res_A0,  res_A1,  res_A2,  res_A3,  res_A4,  res_A5,  res_A6,  res_A7,  res_A8,  res_A9,  res_AA,  res_AB,  res_AC,  res_AD,  res_AE,  res_AF,
/*B*/  res_B0,  res_B1,  res_B2,  res_B3,  res_B4,  res_B5,  res_B6,  res_B7,  res_B8,  res_B9,  res_BA,  res_BB,  res_BC,  res_BD,  res_BE,  res_BF,
/*C*/  set_C0,  set_C1,  set_C2,  set_C3,  set_C4,  set_C5,  set_C6,  set_C7,  set_C8,  set_C9,  set_CA,  set_CB,  set_CC,  set_CD,  set_CE,  set_CF,
/*D*/  set_D0,  set_D1,  set_D2,  set_D3,  set_D4,  set_D5,  set_D6,  set_D7,  set_D8,  set_D9,  set_DA,  set_DB,  set_DC,  set_DD,  set_DE,  set_DF,
/*E*/  set_E0,  set_E1,  set_E2,  set_E3,  set_E4,  set_E5,  set_E6,  set_E7,  set_E8,  set_E9,  set_EA,  set_EB,  set_EC,  set_ED,  set_EE,  set_EF,
/*F*/  set_F0,  set_F1,  set_F2,  set_F3,  set_F4,  set_F5,  set_F6,  set_F7,  set_F8,  set_F9,  set_FA,  set_FB,  set_FC,  set_FD,  set_FE,  set_FF
};




}

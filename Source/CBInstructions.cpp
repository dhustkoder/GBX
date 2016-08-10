#include "Gameboy.hpp"
#include "Instructions.hpp"

namespace gbx {
        

    
    
// note: all CB instructons are 2 bytes long, no need to increment PC
// this is done by the CB instructions caller.



// CB Instructions Implementation:
// 0x00
void rlc_00(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rlc_01(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rlc_02(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rlc_03(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rlc_04(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rlc_05(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rlc_06(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rlc_07(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rrc_08(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rrc_09(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rrc_0A(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rrc_0B(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rrc_0C(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rrc_0D(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rrc_0E(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rrc_0F(Gameboy* const) { ASSERT_INSTR_IMPL(); }


// 0x10
void rl_10(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rl_11(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rl_12(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rl_13(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rl_14(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rl_15(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rl_16(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rl_17(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rr_18(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rr_19(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rr_1A(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rr_1B(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rr_1C(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rr_1D(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rr_1E(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void rr_1F(Gameboy* const) { ASSERT_INSTR_IMPL(); }






// 0x20
void sla_20(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void sla_21(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void sla_22(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void sla_23(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void sla_24(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void sla_25(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void sla_26(Gameboy* const) { ASSERT_INSTR_IMPL(); }





void sla_27(Gameboy* const gb) { 
	// SLA A
	// 2  8
	// Z 0 0 C
	const uint8_t result = gb->cpu.SLA(gb->cpu.GetA());
	gb->cpu.SetA(result);
}





void sra_28(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void sra_29(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void sra_2A(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void sra_2B(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void sra_2C(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void sra_2D(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void sra_2E(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void sra_2F(Gameboy* const) { ASSERT_INSTR_IMPL(); }




// 0x30
void swap_30(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void swap_31(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void swap_32(Gameboy* const) { ASSERT_INSTR_IMPL(); }


void swap_33(Gameboy* const gb)
{
	// SWAP E
	// clock cycles: 8
	// Z 0 0 0
	const uint8_t result = gb->cpu.SWAP(gb->cpu.GetE());
	gb->cpu.SetE(result);
}



void swap_34(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void swap_35(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void swap_36(Gameboy* const) { ASSERT_INSTR_IMPL(); }



void swap_37(Gameboy* const gb)
{
	// SWAP A
	// swap upper & lower bits of A
	// bytes: 2
	// clock cycles: 8
	// flags affected: Z 0 0 0
	const auto result = gb->cpu.SWAP(gb->cpu.GetA());
	gb->cpu.SetA(result);
}








void srl_38(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void srl_39(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void srl_3A(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void srl_3B(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void srl_3C(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void srl_3D(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void srl_3E(Gameboy* const) { ASSERT_INSTR_IMPL(); }

void srl_3F(Gameboy* const gb)
{ 
	// SRL A
	// clock cycles: 8
	// flags affected: Z 0 0 C
	const uint8_t result = gb->cpu.SRL(gb->cpu.GetA());
	gb->cpu.SetA(result);
}






// 0x40
void bit_40(Gameboy* const gb)
{ 
	// BIT 0, B
	// clock cycles: 8
	// Z 0 1 -
	gb->cpu.BIT(0, gb->cpu.GetB());
}



void bit_41(Gameboy* const gb) 
{ 
	// BIT 0, C ( Z 0 1 - )
	gb->cpu.BIT(0, gb->cpu.GetC());
}


void bit_42(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_43(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_44(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_45(Gameboy* const) { ASSERT_INSTR_IMPL(); }



void bit_46(Gameboy* const gb)
{ 
	// BIT 0, (HL) ( Z 0 1 - )
	const uint16_t hl = gb->cpu.GetHL();
	gb->cpu.BIT(0, gb->ReadU8(hl));
}



void bit_47(Gameboy* const gb)
{ 
	// BIT 0, A ( Z 0 1 - )
	gb->cpu.BIT(0, gb->cpu.GetA());
}



void bit_48(Gameboy* const gb)
{ 
	// BIT 1, B ( Z 0 1 - )
	gb->cpu.BIT(1, gb->cpu.GetB());
}



void bit_49(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_4A(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_4B(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_4C(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_4D(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_4E(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_4F(Gameboy* const) { ASSERT_INSTR_IMPL(); }






// 0x50
void bit_50(Gameboy* const gb)
{ 
	// BIT 2, B (  Z 0 1 -  )
	gb->cpu.BIT(2, gb->cpu.GetB());
}


void bit_51(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_52(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_53(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_54(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_55(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_56(Gameboy* const) { ASSERT_INSTR_IMPL(); }


void bit_57(Gameboy* const gb) 
{ 
	// BIT 2, A ( Z 0 1 - )
	gb->cpu.BIT(2, gb->cpu.GetA());
}


void bit_58(Gameboy* const gb) 
{ 
	// BIT 3, B ( Z 0 1 - )
	gb->cpu.BIT(3, gb->cpu.GetB());
}


void bit_59(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_5A(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_5B(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_5C(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_5D(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_5E(Gameboy* const) { ASSERT_INSTR_IMPL(); }


void bit_5F(Gameboy* const gb)
{
	// BIT 3, A
	// clock cycles: 8
	// Z 0 1 -
	gb->cpu.BIT(3, gb->cpu.GetA());
}




// 0x60
void bit_60(Gameboy* const gb) 
{ 
	// BIT 4, B
	// clock cycles: 8
	// flags affected: Z 0 1 - 
	gb->cpu.BIT(4, gb->cpu.GetB());
}



void bit_61(Gameboy* const gb)
{ 
	// BIT 4, C
	// Z 0 1 -
	gb->cpu.BIT(4, gb->cpu.GetC());
}




void bit_62(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_63(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_64(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_65(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_66(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_67(Gameboy* const) { ASSERT_INSTR_IMPL(); }




void bit_68(Gameboy* const gb)
{ 
	// BIT 5, B
	gb->cpu.BIT(5, gb->cpu.GetB());
}





void bit_69(Gameboy* const gb)
{ 
	// BIT 5, C
	gb->cpu.BIT(5, gb->cpu.GetC());
}



void bit_6A(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_6B(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_6C(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_6D(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_6E(Gameboy* const) { ASSERT_INSTR_IMPL(); }

void bit_6F(Gameboy* const gb)
{ 
	// BIT 5, A ( Z 0 1 - )
	gb->cpu.BIT(5, gb->cpu.GetA());
}





// 0x70
void bit_70(Gameboy* const gb) 
{
	// BIT 6, B ( Z 0 1 - )
	gb->cpu.BIT(6, gb->cpu.GetB());
}



void bit_71(Gameboy* const gb)
{ 
	// BIT 6, C ( Z 0 1 - )
	gb->cpu.BIT(6, gb->cpu.GetC());
}


void bit_72(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_73(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_74(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_75(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_76(Gameboy* const) { ASSERT_INSTR_IMPL(); }



void bit_77(Gameboy* const gb)
{ 
	// BIT 6, A ( Z 0 1 - )
	// clock cycles: 8
	// Z 0 1 -
	gb->cpu.BIT(6, gb->cpu.GetA());
}



void bit_78(Gameboy* const gb) 
{ 
	// BIT 7, B ( Z 0 1  - )
	gb->cpu.BIT(7, gb->cpu.GetB());
}


void bit_79(Gameboy* const gb) 
{ 
	// BIT 7, C ( Z 0 1 - )
	gb->cpu.BIT(7, gb->cpu.GetC());
}
void bit_7A(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_7B(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_7C(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void bit_7D(Gameboy* const) { ASSERT_INSTR_IMPL(); }



void bit_7E(Gameboy* const gb)
{
	// BIT 7, (HL)
	// clock cycles: 16
	// flags affected: Z 0 1 -
	const uint8_t value = gb->ReadU8(gb->cpu.GetHL());
	gb->cpu.BIT(7, value);
}



void bit_7F(Gameboy* const gb)
{ 
	// BIT 7, A
	// clock cycles: 8
	// flags affected: Z 0 1 -
	gb->cpu.BIT(7, gb->cpu.GetA());
}






// 0x80
void res_80(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_81(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_82(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_83(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_84(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_85(Gameboy* const) { ASSERT_INSTR_IMPL(); }



void res_86(Gameboy* const gb)
{ 
	// RES 2, (HL)
	// clock cycles: 16
	const uint16_t hl = gb->cpu.GetHL();
	const uint8_t value = gb->ReadU8(hl);
	const uint8_t result = ResBit(2, value);
	gb->WriteU8(hl, result);
}






void res_87(Gameboy* const gb)
{
	// RES 0, A
	// reset bit 0 in register A
	// bytes: 2
	// clock cycles: 8
	const auto a = gb->cpu.GetA();
	const uint8_t result = ResBit(0, a);
	gb->cpu.SetA(result);
}






void res_88(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_89(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_8A(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_8B(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_8C(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_8D(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_8E(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_8F(Gameboy* const) { ASSERT_INSTR_IMPL(); }




// 0x90
void res_90(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_91(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_92(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_93(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_94(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_95(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_96(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_97(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_98(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_99(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_9A(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_9B(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_9C(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_9D(Gameboy* const) { ASSERT_INSTR_IMPL(); }


void res_9E(Gameboy* const gb) 
{ 
	// RES 3, (HL)
	const uint16_t hl = gb->cpu.GetHL();
	const uint8_t result = ResBit(3, gb->ReadU8(hl));
	gb->WriteU8(hl, result);
}


void res_9F(Gameboy* const) { ASSERT_INSTR_IMPL(); }



// 0xA0
void res_A0(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_A1(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_A2(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_A3(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_A4(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_A5(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_A6(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_A7(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_A8(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_A9(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_AA(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_AB(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_AC(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_AD(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_AE(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_AF(Gameboy* const) { ASSERT_INSTR_IMPL(); }




// 0xB0
void res_B0(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_B1(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_B2(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_B3(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_B4(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_B5(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_B6(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_B7(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_B8(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_B9(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_BA(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_BB(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_BC(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void res_BD(Gameboy* const) { ASSERT_INSTR_IMPL(); }


void res_BE(Gameboy* const gb) 
{ 
	// RES 7, (HL)
	const uint16_t hl = gb->cpu.GetHL();
	const uint8_t val = gb->ReadU8(gb->cpu.GetHL());
	gb->WriteU8(hl, ResBit(7, val));
}


void res_BF(Gameboy* const) { ASSERT_INSTR_IMPL(); }




// 0xC0
void set_C0(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_C1(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_C2(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_C3(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_C4(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_C5(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_C6(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_C7(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_C8(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_C9(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_CA(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_CB(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_CC(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_CD(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_CE(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_CF(Gameboy* const) { ASSERT_INSTR_IMPL(); }



// 0xD0
void set_D0(Gameboy* const gb) 
{ 
	// SET 2, B
	gb->cpu.SetB(SetBit(2, gb->cpu.GetB()));
}


void set_D1(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_D2(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_D3(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_D4(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_D5(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_D6(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_D7(Gameboy* const) { ASSERT_INSTR_IMPL(); }

void set_D8(Gameboy* const gb)
{ 
	// SET 3, B
	gb->cpu.SetB(SetBit(3, gb->cpu.GetB()));
}

void set_D9(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_DA(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_DB(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_DC(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_DD(Gameboy* const) { ASSERT_INSTR_IMPL(); }


void set_DE(Gameboy* const gb) 
{ 
	// SET 3, (HL)
	const uint16_t hl = gb->cpu.GetHL();
	const uint8_t result = SetBit(3, gb->ReadU8(hl));
	gb->WriteU8(hl, result);
}


void set_DF(Gameboy* const) { ASSERT_INSTR_IMPL(); }



// 0xE0
void set_E0(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_E1(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_E2(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_E3(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_E4(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_E5(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_E6(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_E7(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_E8(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_E9(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_EA(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_EB(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_EC(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_ED(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_EE(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_EF(Gameboy* const) { ASSERT_INSTR_IMPL(); }



// 0xF0
void set_F0(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_F1(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_F2(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_F3(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_F4(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_F5(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_F6(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_F7(Gameboy* const) { ASSERT_INSTR_IMPL(); }

void set_F8(Gameboy* const gb)
{ 
	// SET 7, B
	gb->cpu.SetB(SetBit(7, gb->cpu.GetB()));
}

void set_F9(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_FA(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_FB(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_FC(Gameboy* const) { ASSERT_INSTR_IMPL(); }
void set_FD(Gameboy* const) { ASSERT_INSTR_IMPL(); }



void set_FE(Gameboy* const)
{ 
	// SET
}


void set_FF(Gameboy* const) { ASSERT_INSTR_IMPL(); }
    
    




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
/*F*/  set_F0,  set_F1,  set_F2,  set_F3,  set_F4,  set_F5,  set_F6,  set_F7,  set_F8,  set_F9,  set_FA,  set_FB,  set_FC,  set_ED,  set_FE,  set_FF
};




}

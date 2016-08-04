#include "CPU.hpp"
#include "Instructions.hpp"

namespace gbx {
        

    
    
// note: all CB instructons are 2 bytes long, no need to increment PC
// this is done by the CB instructions caller.



// CB Instructions Implementation:
// 0x00
void rlc_00(CPU* const) { ASSERT_INSTR_IMPL(); }
void rlc_01(CPU* const) { ASSERT_INSTR_IMPL(); }
void rlc_02(CPU* const) { ASSERT_INSTR_IMPL(); }
void rlc_03(CPU* const) { ASSERT_INSTR_IMPL(); }
void rlc_04(CPU* const) { ASSERT_INSTR_IMPL(); }
void rlc_05(CPU* const) { ASSERT_INSTR_IMPL(); }
void rlc_06(CPU* const) { ASSERT_INSTR_IMPL(); }
void rlc_07(CPU* const) { ASSERT_INSTR_IMPL(); }
void rrc_08(CPU* const) { ASSERT_INSTR_IMPL(); }
void rrc_09(CPU* const) { ASSERT_INSTR_IMPL(); }
void rrc_0A(CPU* const) { ASSERT_INSTR_IMPL(); }
void rrc_0B(CPU* const) { ASSERT_INSTR_IMPL(); }
void rrc_0C(CPU* const) { ASSERT_INSTR_IMPL(); }
void rrc_0D(CPU* const) { ASSERT_INSTR_IMPL(); }
void rrc_0E(CPU* const) { ASSERT_INSTR_IMPL(); }
void rrc_0F(CPU* const) { ASSERT_INSTR_IMPL(); }
// 0x01
void rl_10(CPU* const) { ASSERT_INSTR_IMPL(); }
void rl_11(CPU* const) { ASSERT_INSTR_IMPL(); }
void rl_12(CPU* const) { ASSERT_INSTR_IMPL(); }
void rl_13(CPU* const) { ASSERT_INSTR_IMPL(); }
void rl_14(CPU* const) { ASSERT_INSTR_IMPL(); }
void rl_15(CPU* const) { ASSERT_INSTR_IMPL(); }
void rl_16(CPU* const) { ASSERT_INSTR_IMPL(); }
void rl_17(CPU* const) { ASSERT_INSTR_IMPL(); }
void rr_18(CPU* const) { ASSERT_INSTR_IMPL(); }
void rr_19(CPU* const) { ASSERT_INSTR_IMPL(); }
void rr_1A(CPU* const) { ASSERT_INSTR_IMPL(); }
void rr_1B(CPU* const) { ASSERT_INSTR_IMPL(); }
void rr_1C(CPU* const) { ASSERT_INSTR_IMPL(); }
void rr_1D(CPU* const) { ASSERT_INSTR_IMPL(); }
void rr_1E(CPU* const) { ASSERT_INSTR_IMPL(); }
void rr_1F(CPU* const) { ASSERT_INSTR_IMPL(); }
// 0x02
void sla_20(CPU* const) { ASSERT_INSTR_IMPL(); }
void sla_21(CPU* const) { ASSERT_INSTR_IMPL(); }
void sla_22(CPU* const) { ASSERT_INSTR_IMPL(); }
void sla_23(CPU* const) { ASSERT_INSTR_IMPL(); }
void sla_24(CPU* const) { ASSERT_INSTR_IMPL(); }
void sla_25(CPU* const) { ASSERT_INSTR_IMPL(); }
void sla_26(CPU* const) { ASSERT_INSTR_IMPL(); }
void sla_27(CPU* const) { ASSERT_INSTR_IMPL(); }
void sra_28(CPU* const) { ASSERT_INSTR_IMPL(); }
void sra_29(CPU* const) { ASSERT_INSTR_IMPL(); }
void sra_2A(CPU* const) { ASSERT_INSTR_IMPL(); }
void sra_2B(CPU* const) { ASSERT_INSTR_IMPL(); }
void sra_2C(CPU* const) { ASSERT_INSTR_IMPL(); }
void sra_2D(CPU* const) { ASSERT_INSTR_IMPL(); }
void sra_2E(CPU* const) { ASSERT_INSTR_IMPL(); }
void sra_2F(CPU* const) { ASSERT_INSTR_IMPL(); }
// 0x03
void swap_30(CPU* const) { ASSERT_INSTR_IMPL(); }
void swap_31(CPU* const) { ASSERT_INSTR_IMPL(); }
void swap_32(CPU* const) { ASSERT_INSTR_IMPL(); }
void swap_33(CPU* const) { ASSERT_INSTR_IMPL(); }
void swap_34(CPU* const) { ASSERT_INSTR_IMPL(); }
void swap_35(CPU* const) { ASSERT_INSTR_IMPL(); }
void swap_36(CPU* const) { ASSERT_INSTR_IMPL(); }







void swap_37(CPU* const cpu) {
	// SWAP A
	// swap upper & lower bits of A
	// bytes: 2
	// clock cycles: 8
	// flags affected: Z 0 0 0
	const auto a = cpu->GetA();
	const auto result = cpu->SWAP(a);
	cpu->SetA(result);


	debug_printf("SWAP A; -> A(%x), Result(%x) | ", a, result);
	cpu->PrintFlags();
}








void srl_38(CPU* const) { ASSERT_INSTR_IMPL(); }
void srl_39(CPU* const) { ASSERT_INSTR_IMPL(); }
void srl_3A(CPU* const) { ASSERT_INSTR_IMPL(); }
void srl_3B(CPU* const) { ASSERT_INSTR_IMPL(); }
void srl_3C(CPU* const) { ASSERT_INSTR_IMPL(); }
void srl_3D(CPU* const) { ASSERT_INSTR_IMPL(); }
void srl_3E(CPU* const) { ASSERT_INSTR_IMPL(); }
void srl_3F(CPU* const) { ASSERT_INSTR_IMPL(); }
// 0x04
void bit_40(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_41(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_42(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_43(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_44(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_45(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_46(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_47(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_48(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_49(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_4A(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_4B(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_4C(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_4D(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_4E(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_4F(CPU* const) { ASSERT_INSTR_IMPL(); }
// 0x05
void bit_50(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_51(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_52(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_53(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_54(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_55(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_56(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_57(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_58(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_59(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_5A(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_5B(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_5C(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_5D(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_5E(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_5F(CPU* const) { ASSERT_INSTR_IMPL(); }
// 0x06
void bit_60(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_61(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_62(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_63(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_64(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_65(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_66(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_67(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_68(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_69(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_6A(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_6B(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_6C(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_6D(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_6E(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_6F(CPU* const) { ASSERT_INSTR_IMPL(); }
// 0x07
void bit_70(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_71(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_72(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_73(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_74(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_75(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_76(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_77(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_78(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_79(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_7A(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_7B(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_7C(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_7D(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_7E(CPU* const) { ASSERT_INSTR_IMPL(); }
void bit_7F(CPU* const) { ASSERT_INSTR_IMPL(); }
// 0x08
void res_80(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_81(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_82(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_83(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_84(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_85(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_86(CPU* const) { ASSERT_INSTR_IMPL(); }






void res_87(CPU* const cpu) {
	// RES 0, A
	// reset bit 0 in register A
	// bytes: 2
	// clock cycles: 8
	const auto a = cpu->GetA();
	const uint8_t result = a & 0xfe;
	cpu->SetA(result);

	debug_printf("RES 0, A; -> A(%x), Result(%x)\n", a, result);
}






void res_88(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_89(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_8A(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_8B(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_8C(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_8D(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_8E(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_8F(CPU* const) { ASSERT_INSTR_IMPL(); }
// 0x09
void res_90(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_91(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_92(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_93(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_94(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_95(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_96(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_97(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_98(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_99(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_9A(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_9B(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_9C(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_9D(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_9E(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_9F(CPU* const) { ASSERT_INSTR_IMPL(); }
// 0x0A
void res_A0(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_A1(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_A2(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_A3(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_A4(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_A5(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_A6(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_A7(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_A8(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_A9(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_AA(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_AB(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_AC(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_AD(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_AE(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_AF(CPU* const) { ASSERT_INSTR_IMPL(); }
// 0x0B
void res_B0(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_B1(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_B2(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_B3(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_B4(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_B5(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_B6(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_B7(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_B8(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_B9(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_BA(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_BB(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_BC(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_BD(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_BE(CPU* const) { ASSERT_INSTR_IMPL(); }
void res_BF(CPU* const) { ASSERT_INSTR_IMPL(); }
// 0x0C
void set_C0(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_C1(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_C2(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_C3(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_C4(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_C5(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_C6(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_C7(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_C8(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_C9(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_CA(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_CB(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_CC(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_CD(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_CE(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_CF(CPU* const) { ASSERT_INSTR_IMPL(); }
// 0x0D
void set_D0(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_D1(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_D2(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_D3(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_D4(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_D5(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_D6(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_D7(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_D8(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_D9(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_DA(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_DB(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_DC(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_DD(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_DE(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_DF(CPU* const) { ASSERT_INSTR_IMPL(); }
// 0x0E
void set_E0(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_E1(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_E2(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_E3(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_E4(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_E5(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_E6(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_E7(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_E8(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_E9(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_EA(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_EB(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_EC(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_ED(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_EE(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_EF(CPU* const) { ASSERT_INSTR_IMPL(); }
// 0x0F
void set_F0(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_F1(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_F2(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_F3(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_F4(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_F5(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_F6(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_F7(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_F8(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_F9(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_FA(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_FB(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_FC(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_FD(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_FE(CPU* const) { ASSERT_INSTR_IMPL(); }
void set_FF(CPU* const) { ASSERT_INSTR_IMPL(); }
    
    




const cb_instruction_t cb_table[256] = {
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

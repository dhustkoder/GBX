#ifndef GBX_CPU_HPP_
#define GBX_CPU_HPP_
#include <Utix/Ints.h>

namespace gbx {


struct CPU
{
	enum Flags : uint8_t 
	{
		FLAG_Z = 0x80, 
		FLAG_N = 0x40, 
		FLAG_H = 0x20, 
		FLAG_C = 0x10
	};

	uint8_t GetA() const;
	uint8_t GetF() const;
	uint8_t GetB() const;
	uint8_t GetC() const;
	uint8_t GetD() const;
	uint8_t GetE() const;
	uint8_t GetH() const;
	uint8_t GetL() const;

	uint16_t GetAF() const;
	uint16_t GetBC() const;
	uint16_t GetDE() const;
	uint16_t GetHL() const;


	Flags GetFlags(const CPU::Flags flags) const;

	void PrintFlags() const;
	void PrintRegisters() const;

	void SetA(const uint8_t val);
	void SetF(const uint8_t val);
	void SetB(const uint8_t val);
	void SetC(const uint8_t val);
	void SetD(const uint8_t val);
	void SetE(const uint8_t val);
	void SetH(const uint8_t val);
	void SetL(const uint8_t val);

	void SetAF(const uint16_t val);
	void SetBC(const uint16_t val);
	void SetDE(const uint16_t val);
	void SetHL(const uint16_t val);

	void SetFlags(const CPU::Flags flags);
	void ClearFlags(const CPU::Flags flags);

	uint8_t INC(const uint8_t first);
	uint8_t DEC(const uint8_t first);
	
	uint8_t ADC(const uint8_t first, uint8_t second);
	uint8_t SBC(const uint8_t first, uint8_t second);

	uint8_t ADD(const uint8_t first, const uint8_t second);
	uint8_t SUB(const uint8_t first, const uint8_t second);
	void ADD_HL(const uint16_t reg_pair);
	void CP(const uint8_t value);

	uint8_t OR(const uint8_t first, const uint8_t second);
	uint8_t AND(const uint8_t first, const uint8_t second);
	uint8_t XOR(const uint8_t first, const uint8_t second);
	
	uint8_t RR(const uint8_t value);
	uint8_t RRC(const uint8_t value);
	uint8_t RL(const uint8_t value);
	uint8_t RLC(const uint8_t value);
	uint8_t SLA(const uint8_t value);
	uint8_t SRL(const uint8_t value);
	uint8_t SRA(const uint8_t value);
	uint8_t SWAP(const uint8_t value);
	void BIT(const uint8_t bit, const uint8_t value);


	uint32_t clock;
	uint16_t pc;
	uint16_t sp;

	union {
		struct {
			uint8_t f, a; 
		}ind;
		uint16_t pair;
	}af;

	union {
		struct {
			uint8_t c, b; 
		}ind;
		uint16_t pair;
	}bc;

	union {
		struct {
			uint8_t e, d; 
		}ind;
		uint16_t pair;
	}de;

	union {
		struct {
			uint8_t l, h; 
		}ind;
		uint16_t pair;
	}hl;
	
};




constexpr CPU::Flags operator|(const CPU::Flags f1, const CPU::Flags f2) {
	return static_cast<CPU::Flags>(static_cast<uint8_t>(f1) | static_cast<uint8_t>(f2));
}

constexpr CPU::Flags operator&(const CPU::Flags f1, const CPU::Flags f2) {
	return static_cast<CPU::Flags>(static_cast<uint8_t>(f1) & static_cast<uint8_t>(f2));
}



constexpr CPU::Flags CheckZ(const uint32_t result) {
	return result ? static_cast<CPU::Flags>(0) : CPU::FLAG_Z;
}



constexpr CPU::Flags CheckH_bit3(const uint8_t first, const uint8_t second) {
	return (((first&0x0f) + (second&0x0f)) > 0x0f) ? CPU::FLAG_H : static_cast<CPU::Flags>(0);
}



constexpr CPU::Flags CheckH_bit11(const uint16_t first, const uint16_t second) {
	return (((first&0xf00) + (second&0xf00)) & 0x1000) ? CPU::FLAG_H : static_cast<CPU::Flags>(0);
}



constexpr CPU::Flags CheckC_bit7(const uint16_t result) {
	return (result & 0xff00) ? CPU::FLAG_C : static_cast<CPU::Flags>(0);
}



constexpr CPU::Flags CheckC_bit15(const uint32_t result) {
	return (result & 0xffff0000) ? CPU::FLAG_C : static_cast<CPU::Flags>(0);
}



constexpr CPU::Flags CheckC_borrow(const uint8_t first, const uint8_t second) {
	return first < second ? CPU::FLAG_C : static_cast<CPU::Flags>(0);
}



constexpr CPU::Flags CheckH_borrow(const uint8_t first, const uint8_t second) {
	return (((first&0xf) - (second&0xf)) & 0x10) ? CPU::FLAG_H : static_cast<CPU::Flags>(0);
}






inline uint8_t CPU::GetA() const { return af.ind.a; }
inline uint8_t CPU::GetF() const { return af.ind.f; }
inline uint8_t CPU::GetB() const { return bc.ind.b; }
inline uint8_t CPU::GetC() const { return bc.ind.c; }
inline uint8_t CPU::GetD() const { return de.ind.d; }
inline uint8_t CPU::GetE() const { return de.ind.e; }
inline uint8_t CPU::GetH() const { return hl.ind.h; }
inline uint8_t CPU::GetL() const { return hl.ind.l; }



inline uint16_t CPU::GetAF() const { return af.pair; } 
inline uint16_t CPU::GetBC() const { return bc.pair; }
inline uint16_t CPU::GetDE() const { return de.pair; }
inline uint16_t CPU::GetHL() const { return hl.pair; }

inline CPU::Flags CPU::GetFlags(const CPU::Flags flags) const 
{
	return static_cast<Flags>(GetF() & flags);
}


inline void CPU::SetA(const uint8_t val) { af.ind.a = val; }
inline void CPU::SetF(const uint8_t val) { af.ind.f = val; }
inline void CPU::SetB(const uint8_t val) { bc.ind.b = val; }
inline void CPU::SetC(const uint8_t val) { bc.ind.c = val; }
inline void CPU::SetD(const uint8_t val) { de.ind.d = val; }
inline void CPU::SetE(const uint8_t val) { de.ind.e = val; }
inline void CPU::SetH(const uint8_t val) { hl.ind.h = val; }
inline void CPU::SetL(const uint8_t val) { hl.ind.l = val; }

inline void CPU::SetAF(const uint16_t val) { af.pair = val; } 
inline void CPU::SetBC(const uint16_t val) { bc.pair = val; }
inline void CPU::SetDE(const uint16_t val) { de.pair = val; }
inline void CPU::SetHL(const uint16_t val) { hl.pair = val; }

inline void CPU::SetFlags(const CPU::Flags flags)
{
	SetF(GetF() | flags);
}

inline void CPU::ClearFlags(const CPU::Flags flags)
{
	SetF(GetF() & ~flags);
}
















}
#endif

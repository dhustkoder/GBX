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


	uint16_t GetPC() const;
	uint16_t GetSP() const;
	uint16_t GetAF() const;
	uint16_t GetBC() const;
	uint16_t GetDE() const;
	uint16_t GetHL() const;

	uint32_t GetClock() const;

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

	void SetPC(const uint16_t val);
	void SetSP(const uint16_t val);
	void SetAF(const uint16_t val);
	void SetBC(const uint16_t val);
	void SetDE(const uint16_t val);
	void SetHL(const uint16_t val);
	void SetClock(const uint32_t cycles);

	void AddCycles(const uint8_t cycles);
	void AddPC(const uint16_t val);
	void SetFlags(const CPU::Flags flags);
	void ClearFlags(const CPU::Flags flags);

	void ADDHL(const uint16_t reg_pair);

	uint8_t INC(const uint8_t first);
	uint8_t DEC(const uint8_t first);
	
	uint8_t ADC(uint8_t first, const uint8_t second);
	uint8_t SBC(uint8_t first, const uint8_t second);

	uint8_t ADD(const uint8_t first, const uint8_t second);
	uint8_t SUB(const uint8_t first, const uint8_t second);
	
	
	uint8_t OR(const uint8_t first, const uint8_t second);
	uint8_t AND(const uint8_t first, const uint8_t second);
	uint8_t XOR(const uint8_t first, const uint8_t second);
	
	uint8_t RLC(const uint8_t value);
	uint8_t SLA(const uint8_t value);
	uint8_t SRL(const uint8_t value);
	uint8_t SWAP(const uint8_t value);
	void BIT(const uint8_t bit, const uint8_t value);



private:
	uint32_t clock;
	uint16_t pc;
	uint16_t sp;

	// TODO: check endianess, this is only compatible with little endian
	union {
		struct {
			uint8_t f, a; 
		}bytes;
		uint16_t pair;
	}af;

	union {
		struct {
			uint8_t c, b; 
		}bytes;
		uint16_t pair;
	}bc;

	union {
		struct {
			uint8_t e, d; 
		}bytes;
		uint16_t pair;
	}de;

	union {
		struct {
			uint8_t l, h; 
		}bytes;
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



constexpr CPU::Flags CheckH_3th_bit(const uint8_t first, const uint8_t second) {
	return (((first&0x0f) + (second&0x0f)) & 0x10) ? CPU::FLAG_H : static_cast<CPU::Flags>(0);
}



constexpr CPU::Flags CheckH_11th_bit(const uint16_t first, const uint16_t second) {
	return (((first&0xf00) + (second&0xf00)) & 0x1000) ? CPU::FLAG_H : static_cast<CPU::Flags>(0);
}



constexpr CPU::Flags CheckC_11th_bit(const uint16_t result) {
	return (result & 0xff00) ? CPU::FLAG_C : static_cast<CPU::Flags>(0);
}



constexpr CPU::Flags CheckC_15th_bit(const uint32_t result) {
	return (result & 0xffff0000) ? CPU::FLAG_C : static_cast<CPU::Flags>(0);
}



constexpr CPU::Flags CheckC_borrow(const uint8_t first, const uint8_t second) {
	return first < second ? CPU::FLAG_C : static_cast<CPU::Flags>(0);
}



constexpr CPU::Flags CheckH_borrow(const uint8_t first, const uint8_t second) {
	return (((first&0xf) - (second&0xf)) & 0x10) ? CPU::FLAG_H : static_cast<CPU::Flags>(0);
}






inline uint8_t CPU::GetA() const { return af.bytes.a; }
inline uint8_t CPU::GetF() const { return af.bytes.f; }
inline uint8_t CPU::GetB() const { return bc.bytes.b; }
inline uint8_t CPU::GetC() const { return bc.bytes.c; }
inline uint8_t CPU::GetD() const { return de.bytes.d; }
inline uint8_t CPU::GetE() const { return de.bytes.e; }
inline uint8_t CPU::GetH() const { return hl.bytes.h; }
inline uint8_t CPU::GetL() const { return hl.bytes.l; }


inline uint16_t CPU::GetPC() const { return pc; }
inline uint16_t CPU::GetSP() const { return sp; }
inline uint16_t CPU::GetAF() const { return af.pair; } 
inline uint16_t CPU::GetBC() const { return bc.pair; }
inline uint16_t CPU::GetDE() const { return de.pair; }
inline uint16_t CPU::GetHL() const { return hl.pair; }
inline uint32_t CPU::GetClock() const { return clock; }

inline CPU::Flags CPU::GetFlags(const CPU::Flags flags) const {
	return static_cast<Flags>(GetF() & flags);
}


inline void CPU::SetA(const uint8_t val) { af.bytes.a = val; }
inline void CPU::SetF(const uint8_t val) { af.bytes.f = val; }
inline void CPU::SetB(const uint8_t val) { bc.bytes.b = val; }
inline void CPU::SetC(const uint8_t val) { bc.bytes.c = val; }
inline void CPU::SetD(const uint8_t val) { de.bytes.d = val; }
inline void CPU::SetE(const uint8_t val) { de.bytes.e = val; }
inline void CPU::SetH(const uint8_t val) { hl.bytes.h = val; }
inline void CPU::SetL(const uint8_t val) { hl.bytes.l = val; }


inline void CPU::SetPC(const uint16_t val) { pc = val; }
inline void CPU::SetSP(const uint16_t val) { sp = val; }
inline void CPU::SetAF(const uint16_t val) { af.pair = val; } 
inline void CPU::SetBC(const uint16_t val) { bc.pair = val; }
inline void CPU::SetDE(const uint16_t val) { de.pair = val; }
inline void CPU::SetHL(const uint16_t val) { hl.pair = val; }
inline void CPU::SetClock(const uint32_t cycles) { clock = cycles; }

inline void CPU::SetFlags(const CPU::Flags flags) {
	SetF(GetF() | flags);
}

inline void CPU::ClearFlags(const CPU::Flags flags) {
	SetF(GetF() & ~flags);
}


inline void CPU::AddCycles(const uint8_t value) { clock += value; }
inline void CPU::AddPC(const uint16_t val) { pc += val; }

















}
#endif

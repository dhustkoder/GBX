#ifndef GBX_CPU_HPP_
#define GBX_CPU_HPP_
#include <stdint.h>

namespace gbx {


struct CPU 
{
	enum Flags : uint8_t {
		Flag_Z = 0x80, Flag_N = 0x40, 
		Flag_H = 0x20, Flag_C = 0x10
	};

	Flags GetFlags(Flags flags) const;
	void SetFlags(Flags flags);
	void ClearFlags(Flags flags);

	uint32_t clock;
	uint16_t pc;
	uint16_t sp;

	union {
		struct {
			uint8_t f, a; 
		};
		uint16_t af;
	};

	union {
		struct {
			uint8_t c, b; 
		};
		uint16_t bc;
	};

	union {
		struct {
			uint8_t e, d; 
		};
		uint16_t de;
	};

	union {
		struct {
			uint8_t l, h; 
		};
		uint16_t hl;
	};
	
};




constexpr CPU::Flags operator|(const CPU::Flags f1, const CPU::Flags f2) {
	return static_cast<CPU::Flags>(static_cast<uint8_t>(f1) | static_cast<uint8_t>(f2));
}

constexpr CPU::Flags operator&(const CPU::Flags f1, const CPU::Flags f2) {
	return static_cast<CPU::Flags>(static_cast<uint8_t>(f1) & static_cast<uint8_t>(f2));
}



constexpr CPU::Flags CheckZ(const uint8_t result) {
	return result ? static_cast<CPU::Flags>(0) : CPU::Flag_Z;
}



constexpr CPU::Flags CheckH_bit3(const uint8_t first, const uint16_t second) {
	return (((first&0x0f) + (second&0x0f)) & 0x10) ? CPU::Flag_H : static_cast<CPU::Flags>(0);
}



constexpr CPU::Flags CheckH_bit11(const uint16_t first, const uint16_t second) {
	return (((first&0xf00) + (second&0xf00)) & 0x1000) ? CPU::Flag_H : static_cast<CPU::Flags>(0);
}



constexpr CPU::Flags CheckC_bit7(const uint16_t result) {
	return (result & 0xff00) ? CPU::Flag_C : static_cast<CPU::Flags>(0);
}



constexpr CPU::Flags CheckC_bit15(const uint32_t result) {
	return (result & 0xffff0000) ? CPU::Flag_C : static_cast<CPU::Flags>(0);
}



constexpr CPU::Flags CheckC_borrow(const uint8_t first, const uint16_t second) {
	return first < second ? CPU::Flag_C : static_cast<CPU::Flags>(0);
}



constexpr CPU::Flags CheckH_borrow(const uint8_t first, const uint16_t second) {
	return (((first&0xf) - (second&0xf)) < 0) ? CPU::Flag_H : static_cast<CPU::Flags>(0);
}




inline CPU::Flags CPU::GetFlags(const Flags flags) const 
{
	return static_cast<Flags>(f & flags);
}


inline void CPU::SetFlags(const Flags flags)
{
	f |= flags;
}


inline void CPU::ClearFlags(const Flags flags)
{
	f &= ~flags;
}
















}
#endif

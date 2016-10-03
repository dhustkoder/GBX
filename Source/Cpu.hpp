#ifndef GBX_CPU_HPP_
#define GBX_CPU_HPP_
#include <stdint.h>

namespace gbx {

struct Cpu 
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


constexpr Cpu::Flags operator|(const Cpu::Flags f1, const Cpu::Flags f2) {
	return static_cast<Cpu::Flags>(static_cast<uint8_t>(f1) | static_cast<uint8_t>(f2));
}

constexpr Cpu::Flags operator&(const Cpu::Flags f1, const Cpu::Flags f2) {
	return static_cast<Cpu::Flags>(static_cast<uint8_t>(f1) & static_cast<uint8_t>(f2));
}



constexpr Cpu::Flags fcheck_z(const uint8_t result) {
	return result ? static_cast<Cpu::Flags>(0) : Cpu::Flag_Z;
}



constexpr Cpu::Flags fcheck_h_bit3(const uint8_t first, const uint16_t second) {
	return (((first&0x0f) + (second&0x0f)) & 0x10) ? Cpu::Flag_H : static_cast<Cpu::Flags>(0);
}



constexpr Cpu::Flags fcheck_h_bit11(const uint16_t first, const uint16_t second) {
	return (((first&0xf00) + (second&0xf00)) & 0x1000) ? Cpu::Flag_H : static_cast<Cpu::Flags>(0);
}



constexpr Cpu::Flags fcheck_c_bit7(const uint16_t result) {
	return (result & 0xff00) ? Cpu::Flag_C : static_cast<Cpu::Flags>(0);
}



constexpr Cpu::Flags fcheck_c_bit15(const uint32_t result) {
	return (result & 0xffff0000) ? Cpu::Flag_C : static_cast<Cpu::Flags>(0);
}



constexpr Cpu::Flags fcheck_c_borrow(const uint8_t first, const uint16_t second) {
	return first < second ? Cpu::Flag_C : static_cast<Cpu::Flags>(0);
}



constexpr Cpu::Flags fcheck_h_borrow(const uint8_t first, const uint16_t second) {
	return (((first&0xf) - (second&0xf)) < 0) ? Cpu::Flag_H : static_cast<Cpu::Flags>(0);
}




inline Cpu::Flags Cpu::GetFlags(const Flags flags) const 
{
	return static_cast<Flags>(f & flags);
}


inline void Cpu::SetFlags(const Flags flags)
{
	f |= flags;
}


inline void Cpu::ClearFlags(const Flags flags)
{
	f &= ~flags;
}



} // namespace gbx
#endif


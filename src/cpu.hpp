#ifndef GBX_CPU_HPP_
#define GBX_CPU_HPP_
#include <stdint.h>

namespace gbx {

constexpr const int_fast32_t kCpuFreq = 4194304;

enum CpuFlags : uint8_t {
	kFlagZ = 0x80, kFlagN = 0x40, 
	kFlagH = 0x20, kFlagC = 0x10
};


struct Cpu {
	int32_t clock;
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


constexpr CpuFlags operator|(const CpuFlags f1, const CpuFlags f2) 
{
	return static_cast<CpuFlags>(static_cast<uint8_t>(f1) | static_cast<uint8_t>(f2));
}

constexpr CpuFlags operator&(const CpuFlags f1, const CpuFlags f2) 
{
	return static_cast<CpuFlags>(static_cast<uint8_t>(f1) & static_cast<uint8_t>(f2));
}

constexpr CpuFlags fcheck_z(const uint8_t result) 
{
	return result ? static_cast<CpuFlags>(0) : kFlagZ;
}


constexpr CpuFlags fcheck_h_bit3(const uint8_t first, const uint16_t second) 
{
	return (((first&0x0f) + (second&0x0f)) & 0x10) ? kFlagH : static_cast<CpuFlags>(0);
}


constexpr CpuFlags fcheck_h_bit11(const uint16_t first, const uint16_t second) 
{
	return (((first&0xf00) + (second&0xf00)) & 0x1000) ? kFlagH : static_cast<CpuFlags>(0);
}


constexpr CpuFlags fcheck_c_bit7(const uint16_t result) 
{
	return (result & 0xff00) ? kFlagC : static_cast<CpuFlags>(0);
}


constexpr CpuFlags fcheck_c_bit15(const uint32_t result) 
{
	return (result & 0xffff0000) ? kFlagC : static_cast<CpuFlags>(0);
}


constexpr CpuFlags fcheck_c_borrow(const uint8_t first, const uint16_t second)
{
	return first < second ? kFlagC : static_cast<CpuFlags>(0);
}


constexpr CpuFlags fcheck_h_borrow(const uint8_t first, const uint16_t second) 
{
	return (((first&0xf) - (second&0xf)) < 0) ? kFlagH : static_cast<CpuFlags>(0);
}


inline CpuFlags get_flags(const Cpu& cpu, const CpuFlags flags)
{
	return static_cast<CpuFlags>(cpu.f & flags);
}


inline void set_flags(const CpuFlags flags, Cpu* const cpu)
{
	cpu->f |= flags;
}


inline void clear_flags(const CpuFlags flags, Cpu* const cpu)
{
	cpu->f &= ~flags;
}



} // namespace gbx
#endif


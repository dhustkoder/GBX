#ifndef GBX_PPU_HPP_
#define GBX_PPU_HPP_
#include "common.hpp"
#include "hwstate.hpp"
#include "memory.hpp"

namespace gbx {

enum Color : uint32_t {
	kBlack = 0x00000000,
	kWhite = 0x00FFFFFF,
	kLightGrey = 0x00909090,
	kDarkGrey = 0x00555555
};

enum class PpuMode : uint8_t {
	HBlank = 0x0,
	VBlank = 0x1,
	SearchOAM = 0x2,
	Transfer = 0x3
};

struct Pallete {
	uint8_t value;
	Color colors[4];
};

constexpr const Color kColors[4] { kWhite, kLightGrey, kDarkGrey, kBlack };

struct Ppu {
	int16_t clock;

	union {
		uint8_t value;
		struct {
			uint8_t bg_on     : 1;
			uint8_t obj_on    : 1;
			uint8_t obj_size  : 1;
			uint8_t bg_map    : 1;
			uint8_t tile_data : 1;
			uint8_t win_on    : 1;
			uint8_t win_map   : 1;
			uint8_t lcd_on    : 1;
		};
	} lcdc;

	union {
		uint8_t value;
		struct {
			uint8_t mode               : 2;
			uint8_t coincidence_flag   : 1;
			uint8_t int_on_hblank      : 1;
			uint8_t int_on_vblank      : 1;
			uint8_t int_on_oam         : 1;
			uint8_t int_on_coincidence : 1;
		};
	} stat;

	uint8_t scy;
	uint8_t scx;
	uint8_t wy;
	uint8_t wx;
	uint8_t ly;
	uint8_t lyc;
	Pallete bgp;
	Pallete obp0;
	Pallete obp1;

	static uint32_t screen[144][160];
};


extern void update_ppu(int16_t cycles, const Memory& mem, HWState* hwstate, Ppu* ppu);

inline PpuMode get_ppu_mode(const Ppu& ppu)
{
	return static_cast<PpuMode>(ppu.stat.mode);
}

inline int16_t get_ppu_mode_clock_limit(const PpuMode mode)
{
	constexpr const int16_t limits[] = { 204, 456, 80, 172 };
	return limits[static_cast<size_t>(mode)];
}

inline void set_ppu_mode(const PpuMode mode, Ppu* const ppu, HWState* const hwstate)
{
	if (get_ppu_mode(*ppu) == mode)
		return;

	const auto mode_value = static_cast<uint8_t>(mode);

	if (mode_value < 3 && (ppu->stat.value>>(3 + mode_value))&1)
		request_interrupt(kInterrupts.lcd, hwstate);

	ppu->stat.mode = mode_value;
}

inline void write_pallete(const uint8_t val, Pallete* const pal)
{
	if (pal->value != val) {
		pal->value = val;
		pal->colors[0] = kColors[val&0x03];
		pal->colors[1] = kColors[(val&0x0C)>>2];
		pal->colors[2] = kColors[(val&0x30)>>4];
		pal->colors[3] = kColors[(val&0xC0)>>6];
	}
}


} // namespace gbx
#endif


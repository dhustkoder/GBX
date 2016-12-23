#ifndef GBX_GPU_HPP_
#define GBX_GPU_HPP_
#include "common.hpp"
#include "hwstate.hpp"

namespace gbx {

enum Color : uint32_t {
	kBlack = 0x00000000,
	kWhite = 0x00FFFFFF,
	kLightGrey = 0x00909090,
	kDarkGrey = 0x00555555
};

enum class GpuMode : uint8_t {
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

struct Gpu {
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

private:
	friend const uint32_t* get_screen(const Gpu&, int, int);
	friend uint32_t* get_screen(Gpu*, int, int);
	static uint32_t screen[144][160];
};


inline GpuMode get_gpu_mode(const Gpu& gpu)
{
	return static_cast<GpuMode>(gpu.stat.mode);
}

inline int16_t get_gpu_mode_clock_limit(const GpuMode mode)
{
	constexpr const int16_t limits[] = { 204, 456, 80, 172 };
	return limits[static_cast<size_t>(mode)];
}

inline void set_gpu_mode(const GpuMode mode, Gpu* const gpu, HWState* const hwstate)
{
	if (get_gpu_mode(*gpu) == mode)
		return;

	const auto mode_value = static_cast<uint8_t>(mode);

	if (mode_value < 3 && (gpu->stat.value>>(3 + mode_value))&1)
		request_interrupt(kInterrupts.lcd, hwstate);

	gpu->stat.mode = mode_value;
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


inline const uint32_t* get_screen(const Gpu& /*gpu*/, const int y = 0, const int x = 0)
{
	return &Gpu::screen[y][x];
}

inline uint32_t* get_screen(Gpu* const /*gpu*/, const int y = 0, const int x = 0)
{
	return &Gpu::screen[y][x];
}


} // namespace gbx
#endif


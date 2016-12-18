#ifndef GBX_GPU_HPP_
#define GBX_GPU_HPP_
#include "common.hpp"
#include "hwstate.hpp"

namespace gbx {

struct Memory;

enum class GpuMode : uint8_t {
	HBlank = 0x0, VBlank = 0x1,
	SearchOAM = 0x2, Transfer = 0x3
};


struct Gpu 
{
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
			uint8_t dummy              : 1;
		};
	} stat;

	uint8_t scy;
	uint8_t scx;
	uint8_t wy;
	uint8_t wx;
	uint8_t ly;
	uint8_t lyc;
	uint8_t bgp;
	uint8_t obp0;
	uint8_t obp1;

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


} // namespace gbx
#endif


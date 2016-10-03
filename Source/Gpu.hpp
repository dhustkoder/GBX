#ifndef GBX_GPU_HPP_
#define GBX_GPU_HPP_
#include "Common.hpp"


namespace gbx {

struct HWState;
struct Memory;

struct Gpu 
{
	enum Mode : uint8_t {
		HBlank = 0x0, VBlank = 0x1,
		OAM = 0x2, Transfer = 0x3
	};

	uint16_t clock;
	
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
	}lcdc;

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
	}stat;

	uint8_t scy;
	uint8_t scx;
	uint8_t wy;
	uint8_t wx;
	uint8_t ly;
	uint8_t lyc;
	uint8_t bgp;
	uint8_t obp0;
	uint8_t obp1;
};


extern void draw_graphics(const Gpu& gpu, const Memory& memory, uint32_t(&pixels)[144][160]);


} // namespace gbx
#endif


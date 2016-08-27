#ifndef GBX_GPU_HPP_
#define GBX_GPU_HPP_
#include "Common.hpp"
namespace gbx {






struct GPU
{
	enum class Mode : uint8_t {
		HBLANK = 0x00, VBLANK = 0x01,
		OAM = 0x02, TRANSFER = 0x03
	};

	uint16_t clock;
	
	union {
		uint8_t value;
		struct {
			uint8_t bg_on : 1;
			uint8_t obj_on : 1;
			uint8_t obj_size : 1;
			uint8_t bg_map_select : 1;
			uint8_t tile_data_select : 1;
			uint8_t win_on : 1;
			uint8_t win_map_select : 1;
			uint8_t lcd_on : 1;
		};
	}lcdc;

	union {
		uint8_t value;
		struct {
			Mode mode : 2;
			uint8_t coincidence_flag : 1;
			uint8_t int_on_hblank : 1;
			uint8_t int_on_vblank : 1;
			uint8_t int_on_oam : 1;
			uint8_t int_on_coincidence : 1;
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












}
#endif

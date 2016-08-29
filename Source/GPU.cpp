#include <string.h>
#include "Gameboy.hpp"


namespace gbx {


enum Color : uint32_t
{
	BLACK = 0x00000000,
	WHITE = 0xFFFFFF00,
	LIGHT_GREY = 0x90909000,
	DARK_GREY = 0x55555500,
};


static void hblank(GPU* const gpu, const uint8_t* const vram, uint32_t* const gfx);

static void draw_scanline(const uint8_t* const tile_data,
	const uint8_t* const tile_map,
	const bool unsig_tiles,
	uint32_t* const gfx_line);





void Gameboy::UpdateGPU(const uint8_t cycles)
{
	if (!gpu.lcdc.lcd_on) {
		gpu.clock = 0;
		gpu.ly = 0;
		gpu.stat.mode = GPU::Mode::HBLANK;
		return;
	}

	const auto compare_ly = [this] {
		if (gpu.ly != gpu.lyc) {
			if (gpu.stat.coincidence_flag)
				gpu.stat.coincidence_flag = 0;
		} else {
			gpu.stat.coincidence_flag = 1;
			if (gpu.stat.int_on_coincidence)
				hwstate.RequestInt(INT_LCD_STAT);
		}
	};

	const auto set_mode = [this](const GPU::Mode mode) {
		const auto stat = gpu.stat;
		uint8_t int_on = 0;
		switch (mode) {
		case GPU::Mode::HBLANK: int_on = stat.int_on_hblank; break;
		case GPU::Mode::VBLANK: int_on = stat.int_on_vblank; break;
		case GPU::Mode::OAM: int_on = stat.int_on_oam; break;
		default: break;
		}
		if (int_on)
			hwstate.RequestInt(INT_LCD_STAT);
		gpu.stat.mode = mode;
	};

	gpu.clock += cycles;

	

	switch (gpu.stat.mode) {
	case GPU::Mode::HBLANK:
		if (gpu.clock >= 204) {
			hblank(&gpu, memory.vram, memory.gfx);

			if (gpu.ly != 144) {
				set_mode(GPU::Mode::OAM);
			} else {
				hwstate.RequestInt(INT_VBLANK);
				set_mode(GPU::Mode::VBLANK);
			}

			compare_ly();
			gpu.clock -= 204;
		}

		break;

	case GPU::Mode::VBLANK:
		if (gpu.clock >= 456) {
			++gpu.ly;

			if (gpu.ly > 153) {
				gpu.ly = 0;
				set_mode(GPU::Mode::OAM);
			}

			compare_ly();
			gpu.clock -= 456;
		}

		break;

	case GPU::Mode::OAM:
		if (gpu.clock >= 80) {
			gpu.stat.mode = GPU::Mode::TRANSFER;
			gpu.clock -= 80;
		}

		break;

	case GPU::Mode::TRANSFER:
		if (gpu.clock >= 172) {
			set_mode(GPU::Mode::HBLANK);
			gpu.clock -= 172;
		}

		break;
	}
}






static void hblank(GPU* const gpu, const uint8_t* const vram, uint32_t* const gfx)
{
	const auto lcdc = gpu->lcdc;
	const auto ly = gpu->ly++;
	const bool unsig_tiles = lcdc.tile_data != 0;
	const auto tile_data = unsig_tiles ? vram : &vram[0x1000];
	
	const uint16_t ly_div = ly / 8;
	const uint8_t ly_mod = ly % 8;

	auto gfx_line = &gfx[ly * 160];

	if (lcdc.bg_on) {
		auto data = tile_data;
		auto map = lcdc.bg_map ? &vram[0x1C00] : &vram[0x1800];
		
		const auto scx = gpu->scx;
		const auto scy = gpu->scy;
		map += ((((scy / 8) + ly_div) & 31) * 32) + (scx / 8);
		data += ((scy % 8) + ly_mod) * 2;

		draw_scanline(data, map, unsig_tiles, gfx_line);
	}
}





static void draw_scanline(const uint8_t* const tile_data,
	const uint8_t* const tile_map,
	const bool unsig_tiles,
	uint32_t* const gfx_line)
{
	uint32_t colors[4] = {
		WHITE, LIGHT_GREY, 
		DARK_GREY, BLACK
	};

	for (uint8_t x = 0; x < 20; ++x) {
		const uint16_t data_offset = tile_map[x] * 16;
		uint8_t lsbit;
		uint8_t msbit;
		
		if (unsig_tiles) {
			lsbit = tile_data[data_offset];
			msbit = tile_data[data_offset + 1];
		} else {
			auto sig_offset = static_cast<int16_t>(data_offset);
			lsbit = tile_data[sig_offset];
			msbit = tile_data[sig_offset + 1];
		}
		
		uint8_t xpos = x * 8;
		for (uint8_t pix = 0; pix < 8; ++pix, ++xpos) {
			uint8_t col_num = 0;
			if (lsbit & (0x80 >> pix))
				++col_num;
			if (msbit & (0x80 >> pix))
				col_num += 2;
			
			gfx_line[xpos] = colors[col_num];
		}
	}
}














}

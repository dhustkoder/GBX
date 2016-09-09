#include "Gameboy.hpp"


namespace gbx {


enum Color : uint32_t
{
	BLACK = 0x00000000,
	WHITE = 0xFFFFFF00,
	LIGHT_GREY = 0x90909000,
	DARK_GREY = 0x55555500,
};


static const uint32_t colors[4] = {
	WHITE, LIGHT_GREY, 
	DARK_GREY, BLACK
};

static uint16_t bg_scanlines[144][20];





inline void mode_hblank(const Memory& memory, GPU* gpu, HWState* hwstate);
inline void mode_vblank(GPU* gpu, HWState* hwstate);
inline void mode_oam(GPU* gpu);
inline void mode_transfer(GPU* gpu, HWState* hwstate);
inline void check_gpu_lyc(GPU* gpu, HWState* hwstate);
inline void set_gpu_mode(GPU::Mode mode, GPU* gpu, HWState* hwstate);
static void fill_bg_scanline(const GPU& gpu, const uint8_t* vram);




void Gameboy::UpdateGPU(const uint8_t cycles)
{
	if (!gpu.lcdc.lcd_on) {
		gpu.clock = 0;
		gpu.ly = 0;
		gpu.stat.mode = GPU::Mode::HBLANK;
		return;
	}

	gpu.clock += cycles;
	
	switch (gpu.stat.mode) {
	case GPU::Mode::HBLANK: mode_hblank(memory, &gpu, &hwstate); break;
	case GPU::Mode::VBLANK: mode_vblank(&gpu, &hwstate); break;
	case GPU::Mode::OAM: mode_oam(&gpu); break;
	case GPU::Mode::TRANSFER: mode_transfer(&gpu, &hwstate); break;
	default: break;
	}
}





void mode_hblank(const Memory& memory, GPU* const gpu, HWState* const hwstate)
{
	if (gpu->clock >= 204) {
		fill_bg_scanline(*gpu, memory.vram);
		if (++gpu->ly != 144) {
			set_gpu_mode(GPU::Mode::OAM, gpu, hwstate);
		} else {
			hwstate->RequestInt(INT_VBLANK);
			set_gpu_mode(GPU::Mode::VBLANK, gpu, hwstate);
		}

		check_gpu_lyc(gpu, hwstate);
		gpu->clock -= 204;
	}
}


void mode_vblank(GPU* const gpu, HWState* const hwstate)
{
	if (gpu->clock >= 456) {
		if (++gpu->ly > 153) {
			gpu->ly = 0;
			set_gpu_mode(GPU::Mode::OAM, gpu, hwstate);
		}

		gpu->clock -= 456;
		check_gpu_lyc(gpu, hwstate);
	}
}


void mode_oam(GPU* const gpu)
{
	if (gpu->clock >= 80) {
		gpu->stat.mode = GPU::Mode::TRANSFER;
		gpu->clock -= 80;
	}
}


void mode_transfer(GPU* const gpu, HWState* const hwstate)
{
	if (gpu->clock >= 172) {
		set_gpu_mode(GPU::Mode::HBLANK, gpu, hwstate);
		gpu->clock -= 172;
	}
}



void set_gpu_mode(const GPU::Mode mode, GPU* const gpu, HWState* const hwstate)
{
	const auto stat = gpu->stat;
	uint8_t int_on = 0;
	switch (mode) {
	case GPU::Mode::HBLANK: int_on = stat.int_on_hblank; break;
	case GPU::Mode::VBLANK: int_on = stat.int_on_vblank; break;
	case GPU::Mode::OAM: int_on = stat.int_on_oam; break;
	default: break;
	}
	if (int_on)
		hwstate->RequestInt(INT_LCD_STAT);
	gpu->stat.mode = static_cast<uint8_t>(mode);
};



void check_gpu_lyc(GPU* const gpu, HWState* const hwstate)
{
	if (gpu->ly != gpu->lyc) {
		if (gpu->stat.coincidence_flag)
			gpu->stat.coincidence_flag = 0;
	} else {
		gpu->stat.coincidence_flag = 1;
		if (gpu->stat.int_on_coincidence)
			hwstate->RequestInt(INT_LCD_STAT);
	}
}






void fill_bg_scanline(const GPU& gpu, const uint8_t* const vram)
{
	const auto ly = gpu.ly;
	const auto lcdc = gpu.lcdc;
	const bool unsig_data = lcdc.tile_data != 0;
	const auto tile_data = unsig_data ? vram : vram + 0x1000;

	const auto fill_row =
	[ly, unsig_data](const uint8_t* data, const uint8_t* map, uint8_t mapx) {
		if (unsig_data) {
			for (uint8_t x = 0; x < 20; ++x) {
				const uint16_t addr = map[(x + mapx) & 31] * 16;
				const uint8_t lsb = data[addr];
				const uint8_t msb = data[addr + 1];
				const uint16_t row = (msb << 8) | lsb;
				bg_scanlines[ly][x] = row;
			}
		} else {
			for (uint8_t x = 0; x < 20; ++x) {
				const int8_t id = map[(x + mapx) & 31];
				const int16_t addr = id * 16;
				const uint8_t lsb = *(data + addr);
				const uint8_t msb = *(data + addr + 1);
				const uint16_t row = (msb << 8) | lsb;
				bg_scanlines[ly][x] = row;
			}
		}
	};


	if (lcdc.bg_on &&  (!lcdc.win_on || ly < gpu.wy)) {
		const uint8_t lydiv = ly / 8;
		const uint8_t lymod = ly % 8;
		const uint8_t scxdiv = gpu.scx / 8;
		const uint8_t scydiv = gpu.scy / 8;
		const auto data = &tile_data[lymod * 2];
		auto map = lcdc.bg_map ? vram + 0x1C00 : vram + 0x1800;
		map += ((lydiv + scydiv)&31) * 32;
		fill_row(data, map, scxdiv);
	}

	if (lcdc.win_on && ly >= gpu.wy) {
		const uint8_t wy = gpu.wy;
		const uint8_t wx = gpu.wx - 7;
		if (wy < 144 && wx < 160) {
			const auto map = lcdc.win_map ? vram + 0x1C00 : vram + 0x1800;
			fill_row(tile_data, map, 0);
		}
	}
}






void draw_graphics(const GPU& gpu, uint32_t* const pixels)
{
	const auto bgp = gpu.bgp;
	const uint8_t pallete[4] = {
		static_cast<uint8_t>(bgp&0x03),
		static_cast<uint8_t>((bgp&0x0C) >> 2),
		static_cast<uint8_t>((bgp&0x30) >> 4),
		static_cast<uint8_t>((bgp&0xC0) >> 6)
	};

	for (uint8_t y = 0; y < 144; ++y) {
		uint32_t* const line = &pixels[y * 160];
		for (uint8_t r = 0; r < 20; ++r) {
			const uint8_t xpos = r * 8;
			const uint16_t row = bg_scanlines[y][r];
			for (uint8_t pix = 0; pix < 8; ++pix) {
				uint8_t col = 0;
				if (row & (0x80 >> pix))
					++col;
				if (row & (0x8000 >> pix))
					col += 2;

				line[xpos + pix] = colors[pallete[col]];
			}
		}
	}
}

















}

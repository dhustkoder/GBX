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


uint16_t GPU::bg_scanlines[144][20];


static void fill_bg_scanline(const GPU& gpu, const uint8_t* const vram);



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
			fill_bg_scanline(gpu, memory.vram);

			if (++gpu.ly != 144) {
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





inline void fill_row_unsig(const uint8_t* const data, 
	const uint8_t* const map,
	const uint8_t mapx,
	const uint8_t ly)
{
	for (uint8_t x = 0; x < 20; ++x) {
		const uint16_t addr = map[(x + mapx) & 31] * 16;
		const uint8_t lsb = data[addr];
		const uint8_t msb = data[addr + 1];
		const uint16_t row = (msb << 8) | lsb;
		GPU::bg_scanlines[ly][x] = row;
	}
}

inline void fill_row_sig(const uint8_t* const data, 
	const uint8_t* const map,
	const uint8_t mapx,
	const uint8_t ly)
{
	for (uint8_t x = 0; x < 20; ++x) {
		const int8_t id = map[(x + mapx) & 31];
		const int16_t addr = id * 16;
		const uint8_t lsb = *(data + addr);
		const uint8_t msb = *(data + addr + 1);
		const uint16_t row = (msb << 8) | lsb;
		GPU::bg_scanlines[ly][x] = row;
	}
};


static void fill_bg_scanline(const GPU& gpu, const uint8_t* const vram)
{
	const auto ly = gpu.ly;
	const auto lcdc = gpu.lcdc;
	const bool unsig_data = lcdc.tile_data != 0;
	const auto tile_data = unsig_data ? vram : vram + 0x1000;
	const auto fill_row = unsig_data ? fill_row_unsig : fill_row_sig;

	if (lcdc.bg_on &&  (!lcdc.win_on || ly < gpu.wy)) {
		const uint8_t lydiv = ly / 8;
		const uint8_t lymod = ly % 8;
		const uint8_t scxdiv = gpu.scx / 8;
		const uint8_t scydiv = gpu.scy / 8;
		const auto data = &tile_data[lymod * 2];
		auto map = lcdc.bg_map ? vram + 0x1C00 : vram + 0x1800;
		map += ((lydiv + scydiv)&31) * 32;
		fill_row(data, map, scxdiv, ly);
	}

	if (lcdc.win_on && ly >= gpu.wy) {
		const uint8_t wy = gpu.wy;
		const uint8_t wx = gpu.wx - 7;
		if (wy < 144 && wx < 160) {
			const auto map = lcdc.win_map ? vram + 0x1C00 : vram + 0x1800;
			fill_row(tile_data, map, 0, ly);
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
			const uint16_t row = gpu.bg_scanlines[y][r];
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

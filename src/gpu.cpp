#include <string.h>
#include <stdlib.h>
#include "debug.hpp"
#include "gameboy.hpp"


namespace gbx {

struct Scanline {
	uint32_t* data;
	const Color(&colors)[4];
};

uint32_t Gpu::screen[144][160];


void update_gpu(int16_t cycles, const Memory& mem, HWState* hwstate, Gpu* gpu);
inline void mode_hblank(Gpu* gpu, HWState* hwstate);
inline void mode_vblank(Gpu* gpu, HWState* hwstate);
inline void mode_oam(Gpu* gpu, HWState* hwstate);
inline void mode_transfer(const Memory& mem, Gpu* gpu, HWState* hwstate);
inline void check_gpu_lyc(Gpu* gpu, HWState* hwstate);
static void update_bg_scanline(const Memory& mem, Gpu* gpu);
static void update_win_scanline(const Memory& mem, Gpu* gpu);
static void update_sprite_scanline(const Memory& mem, Gpu* gpu);
static void fill_scanline(int pbeg, int pend, uint16_t row, Scanline* scanline);


void update_gpu(const int16_t cycles, const Memory& mem, HWState* const hwstate, Gpu* const gpu)
{
	if (!gpu->lcdc.lcd_on)
		return;

	const auto mode = get_gpu_mode(*gpu);
	const auto clock_limit = get_gpu_mode_clock_limit(mode);
	gpu->clock += cycles;

	if (gpu->clock >= clock_limit) {
		gpu->clock -= clock_limit;
		switch (mode) {
		case GpuMode::HBlank: mode_hblank(gpu, hwstate); break;
		case GpuMode::VBlank: mode_vblank(gpu, hwstate); break;
		case GpuMode::SearchOAM: mode_oam(gpu, hwstate); break;
		case GpuMode::Transfer: mode_transfer(mem, gpu, hwstate); break;
		default: break;
		}
	}
}


void mode_hblank(Gpu* const gpu, HWState* const hwstate)
{
	if (++gpu->ly < 144) {
		set_gpu_mode(GpuMode::SearchOAM, gpu, hwstate);
	} else {
		request_interrupt(kInterrupts.vblank, hwstate);
		set_gpu_mode(GpuMode::VBlank, gpu, hwstate);
	}
	check_gpu_lyc(gpu, hwstate);
}


void mode_vblank(Gpu* const gpu, HWState* const hwstate)
{
	if (++gpu->ly > 153) {
		gpu->ly = 0;
		set_gpu_mode(GpuMode::SearchOAM, gpu, hwstate);
	}
	check_gpu_lyc(gpu, hwstate);
}


void mode_oam(Gpu* const gpu, HWState* const hwstate)
{
	set_gpu_mode(GpuMode::Transfer, gpu, hwstate);
}


void mode_transfer(const Memory& mem, Gpu* const gpu, HWState* const hwstate)
{
	update_bg_scanline(mem, gpu);
	update_win_scanline(mem, gpu);
	update_sprite_scanline(mem, gpu);
	set_gpu_mode(GpuMode::HBlank, gpu, hwstate);
}


void check_gpu_lyc(Gpu* const gpu, HWState* const hwstate)
{
	if (gpu->ly != gpu->lyc) {
		if (gpu->stat.coincidence_flag)
			gpu->stat.coincidence_flag = 0;
	} else {
		gpu->stat.coincidence_flag = 1;
		if (gpu->stat.int_on_coincidence)
			request_interrupt(kInterrupts.lcd, hwstate);
	}
}


void update_bg_scanline(const Memory& mem, Gpu* const gpu)
{
	const auto lcdc = gpu->lcdc;
	const auto ly = gpu->ly;
	if (!lcdc.bg_on) {
		memset(&gpu->screen[ly][0], 0xFF, sizeof(uint32_t) * 160);
		return;
	} else if (lcdc.win_on && ly >= gpu->wy && gpu->wx <= 7) {
		return;
	}

	const auto scx = gpu->scx;
	const auto scy = gpu->scy;
	const bool unsig_data = lcdc.tile_data != 0;

	const int lydiv = ly >> 3;
	const int lymod = ly & 7;
	const int scxdiv = scx >> 3;
	const int scxmod = scx & 7;
	const int scydiv = scy >> 3;
	const int scymod = scy & 7;
	const int ly_scy_mods = lymod + scymod;
	const int ly_scy_divs = lydiv + scydiv;
	const int data_add = (ly_scy_mods&7) * 2;
	const int map_add = 
	  ((ly_scy_divs + ((ly_scy_mods > 7) ? 1 : 0))&31) * 32;

	const uint8_t* const tile_data =
	  (unsig_data ? &mem.vram[0] : &mem.vram[0x1000]) + data_add;
	const uint8_t* const map =
	  (lcdc.bg_map ? &mem.vram[0x1C00] : &mem.vram[0x1800]) + map_add;

	const auto get_row = 
	[map, scxdiv, unsig_data, tile_data](const int mapx)-> uint16_t {
		const uint8_t id = map[(mapx + scxdiv)&31];
		const int addr =
		  (unsig_data ? id : static_cast<int8_t>(id)) * 16;
		return concat_bytes(tile_data[addr + 1], tile_data[addr]);
	};
	
	Scanline scanline {&gpu->screen[ly][0], gpu->bgp.colors};

	if (scxmod == 0) {
		for (int x = 0; x < 20; ++x)
			fill_scanline(0, 8, get_row(x), &scanline);
	} else {
		fill_scanline(scxmod, 8, get_row(0), &scanline);
		for (int x = 1; x < 20; ++x)
			fill_scanline(0, 8, get_row(x), &scanline);
		fill_scanline(0, scxmod, get_row(20), &scanline);
	}
}

void update_win_scanline(const Memory& mem, Gpu* const gpu)
{
	const auto ly = gpu->ly;
	const auto lcdc = gpu->lcdc;
	const int wy = gpu->wy;
	const int wx = gpu->wx - 7;
	if (!lcdc.win_on || ly < wy || wx >= 160)
		return;

	const int wx_max = max(0, wx);
	const bool unsig_data = lcdc.tile_data != 0;
	const int data_add = ((ly - wy) & 7) * 2;
	const int map_add = (((ly - wy) >> 3)&31) * 32;
	const uint8_t* const tile_data = 
	  (unsig_data ? &mem.vram[0] : &mem.vram[0x1000]) + data_add;
	const uint8_t* const map =
	  (lcdc.win_map ? &mem.vram[0x1C00] : &mem.vram[0x1800]) + map_add;

	const auto get_row = 
	[map, unsig_data, tile_data] (const int mapx) -> uint16_t {
		const uint8_t id = map[mapx&31];
		const int addr =
		  (unsig_data ? id : static_cast<int8_t>(id)) * 16;
		return concat_bytes(tile_data[addr + 1], tile_data[addr]);
	};

	Scanline scanline{&gpu->screen[ly][wx_max], gpu->bgp.colors};

	int xbeg = 0;
	int to_draw = (160 - wx_max);
	if (wx < 0) {
		const int abswx = -wx;
		fill_scanline(abswx, 8, get_row(0), &scanline);
		++xbeg;
		to_draw -= (8 - abswx);
	}

	for (int x = xbeg; to_draw > 0; ++x) {
		const int pend = min(8, to_draw);
		to_draw -= pend;
		fill_scanline(0, pend, get_row(x), &scanline);
	}
}

void update_sprite_scanline(const Memory& mem, Gpu* const gpu)
{
	static_assert((sizeof(mem.oam) % 4) == 0, "");
	const auto lcdc = gpu->lcdc;

	if (!lcdc.obj_on)
		return;

	const auto& bgp = gpu->bgp.colors;
	const auto& obp0 = gpu->obp0.colors;
	const auto& obp1 = gpu->obp1.colors;
	const auto ly = gpu->ly;
	const int yres = lcdc.obj_size ? 16 : 8;

	for (int i = sizeof(mem.oam) - 4; i >= 0; i -= 4) {
		const int ypos = mem.oam[i] - 16;
		const int xpos = mem.oam[i + 1] - 8;
		if (ly < ypos || ly >= (ypos + yres) ||
		    xpos <= -8 || xpos >= 160)
			continue;

		const auto ly_ypos_diff = ly - ypos;
		const auto flags = mem.oam[i + 3];
		const bool priority = (flags&0x80) != 0;
		const bool yflip = (flags&0x40) != 0;
		const bool xflip = (flags&0x20) != 0;
		const auto& pal = (flags&0x10) ? obp1 : obp0;

		const auto get_tile_y = [yflip](const int diff) {
			return (yflip ? (7 - diff) : diff) * 2;
		};

		uint16_t row;
		if (yres == 8) {
			const auto tile = &mem.vram[mem.oam[i + 2] * 16];
			const auto tile_y = get_tile_y(ly_ypos_diff);
			row = concat_bytes(tile[tile_y + 1], tile[tile_y]);
		} else {
			const int pattern = mem.oam[i + 2];
			const uint8_t* tile;
			int tile_y;
			if (ly_ypos_diff < 8) {
				tile = yflip
				  ? &mem.vram[(pattern | 0x01) * 16]
				  : &mem.vram[(pattern & 0xFE) * 16];
				tile_y = get_tile_y(ly_ypos_diff);
			} else {
				tile = yflip 
				  ? &mem.vram[(pattern & 0xFE) * 16]
				  : &mem.vram[(pattern | 0x01) * 16];
				tile_y = get_tile_y(ly_ypos_diff - 8);
			}
			row = concat_bytes(tile[tile_y + 1], tile[tile_y]);
		}

		uint32_t* line;
		int pbeg, pend;
		if (xpos < 0) {
			line = &gpu->screen[ly][0];
			pbeg = -xpos;
			pend = 8;
		} else {
			line = &gpu->screen[ly][xpos];
			pbeg = 0;
			pend = min(160 - xpos, 8);
		}

		for (int p = pbeg; p < pend; ++p, ++line) {
			int color_num = 0;	
			if (!xflip) {
				if (row & (0x80 >> p))
					++color_num;
				if (row & (0x8000 >> p))
					color_num += 2;
			} else {
				if (row & (0x01 << p))
					++color_num;
				if (row & (0x0100 << p))
					color_num += 2;
			}
			if (color_num != 0 && (!priority || *line == bgp[0]))
				*line = pal[color_num];
		}
	}
}

void fill_scanline(const int pbeg, const int pend,
                   const uint16_t row, Scanline* const scanline)
{
	auto& data = scanline->data;
	auto& colors = scanline->colors;
	for (int p = pbeg; p < pend; ++p) {
		int colnum = 0;
		if (row & (0x80 >> p))
			++colnum;
		if (row & (0x8000 >> p))
			colnum += 2;
		*data++ = colors[colnum];
	}
}



} // namespace gbx


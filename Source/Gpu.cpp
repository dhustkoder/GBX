#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "Gameboy.hpp"


namespace gbx {

enum Color : uint32_t {
	Black = 0x00000000,
	White = 0xFFFFFF00,
	LightGrey = 0x90909000,
	DarkGrey = 0x55555500
};

constexpr const Color kColors[4] { White, LightGrey, DarkGrey, Black };
uint32_t Gpu::screen[144][160];

struct Pallete {
	constexpr explicit Pallete(const uint8_t pal)
		: colors{kColors[pal&0x03],
			kColors[(pal&0x0C)>>2],
			kColors[(pal&0x30)>>4],
			kColors[(pal&0xC0)>>6]}
	{
	}

	Color operator[](const int offset) const
	{
		assert(offset >= 0 && offset <= 3);
		return colors[offset];
	}

	const Color colors[4];
};

struct ScanlineFiller {
	constexpr ScanlineFiller(uint32_t* line, Pallete pal)
		: scanline(line), 
		pallete(pal)
	{
	}

	void operator()(const int pbeg, const int pend, const uint16_t row) {
		for (int p = pbeg; p < pend; ++p) {
			uint8_t colnum = 0;
			if (row & (0x80 >> p))
				++colnum;
			if (row & (0x8000 >> p))
				colnum += 2;
			*scanline++ = pallete[colnum];
		}
	}

	uint32_t* scanline;
	const Pallete pallete;
};


void update_gpu(int16_t cycles, const Memory& mem, HWState* hwstate, Gpu* gpu);
inline void mode_hblank(Gpu* gpu, HWState* hwstate);
inline void mode_vblank(Gpu* gpu, HWState* hwstate);
inline void mode_oam(Gpu* gpu);
inline void mode_transfer(const Memory& mem, Gpu* gpu, HWState* hwstate);
inline void check_gpu_lyc(Gpu* gpu, HWState* hwstate);
inline void set_gpu_mode(Gpu::Mode mode, Gpu* gpu, HWState* hwstate);
static void update_bg_scanline(const Gpu& gpu, const Memory& mem);
static void update_win_scanline(const Gpu& gpu, const Memory& mem);
static void update_sprite_scanline(const Gpu& gpu, const Memory& mem);


void update_gpu(const int16_t cycles, const Memory& mem, HWState* const hwstate, Gpu* const gpu)
{
	if (!gpu->lcdc.lcd_on) {
		gpu->clock = 0;
		gpu->ly = 0;
		gpu->stat.mode = Gpu::Mode::VBlank;
		return;
	}

	gpu->clock += cycles;
	
	switch (gpu->stat.mode) {
	case Gpu::Mode::HBlank: mode_hblank(gpu, hwstate); break;
	case Gpu::Mode::VBlank: mode_vblank(gpu, hwstate); break;
	case Gpu::Mode::OAM: mode_oam(gpu); break;
	case Gpu::Mode::Transfer: mode_transfer(mem, gpu, hwstate); break;
	default: break;
	}
}


void mode_hblank(Gpu* const gpu, HWState* const hwstate)
{
	if (gpu->clock >= 204) {
		if (++gpu->ly < 144) {
			set_gpu_mode(Gpu::Mode::OAM, gpu, hwstate);
		} else {
			hwstate->RequestInt(interrupts::vblank);
			set_gpu_mode(Gpu::Mode::VBlank, gpu, hwstate);
		}
		check_gpu_lyc(gpu, hwstate);
		gpu->clock -= 204;
	}
}


void mode_vblank(Gpu* const gpu, HWState* const hwstate)
{
	if (gpu->clock >= 456) {
		if (++gpu->ly > 153) {
			gpu->ly = 0;
			set_gpu_mode(Gpu::Mode::OAM, gpu, hwstate);
		}
		gpu->clock -= 456;
		check_gpu_lyc(gpu, hwstate);
	}
}


void mode_oam(Gpu* const gpu)
{
	if (gpu->clock >= 80) {
		gpu->stat.mode = Gpu::Mode::Transfer;
		gpu->clock -= 80;
	}
}


void mode_transfer(const Memory& mem, Gpu* const gpu, HWState* const hwstate)
{
	if (gpu->clock >= 172) {
		update_bg_scanline(*gpu, mem);
		update_win_scanline(*gpu, mem);
		update_sprite_scanline(*gpu, mem);
		set_gpu_mode(Gpu::Mode::HBlank, gpu, hwstate);
		gpu->clock -= 172;
	}
}


void set_gpu_mode(const Gpu::Mode mode, Gpu* const gpu, HWState* const hwstate)
{
	const auto stat = gpu->stat;
	uint8_t int_on = 0;

	switch (mode) {
	case Gpu::Mode::HBlank: int_on = stat.int_on_hblank; break;
	case Gpu::Mode::VBlank: int_on = stat.int_on_vblank; break;
	case Gpu::Mode::OAM: int_on = stat.int_on_oam; break;
	default: break;
	}

	if (int_on)
		hwstate->RequestInt(interrupts::lcd);

	gpu->stat.mode = static_cast<uint8_t>(mode);
};


void check_gpu_lyc(Gpu* const gpu, HWState* const hwstate)
{
	if (gpu->ly != gpu->lyc) {
		if (gpu->stat.coincidence_flag)
			gpu->stat.coincidence_flag = 0;
	} else {
		gpu->stat.coincidence_flag = 1;
		if (gpu->stat.int_on_coincidence)
			hwstate->RequestInt(interrupts::lcd);
	}
}


void update_bg_scanline(const Gpu& gpu, const Memory& mem)
{
	const auto lcdc = gpu.lcdc;
	const auto ly = gpu.ly;
	auto& screen = Gpu::screen;

	if (!lcdc.bg_on) {
		memset(&screen[ly][0], 0xFF, sizeof(uint32_t) * 160);
		return;
	} else if (lcdc.win_on && ly >= gpu.wy && gpu.wx <= 7) {
		return;
	}

	const auto scx = gpu.scx;
	const auto scy = gpu.scy;
	const bool unsig_data = lcdc.tile_data != 0;
	// >> 3 == / 8
	// & 7 == % 8
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

	const auto tile_data = (unsig_data
		? &mem.vram[0] : &mem.vram[0x1000]) + data_add;
	const auto map = (lcdc.bg_map
		? &mem.vram[0x1C00] : &mem.vram[0x1800]) + map_add;

	const auto get_row = 
	[map, scxdiv, unsig_data, tile_data] (const int mapx) -> uint16_t {
		const uint8_t id = map[(mapx + scxdiv)&31];
		const int addr = (unsig_data 
			? id : static_cast<int8_t>(id)) * 16;
		return (tile_data[addr + 1] << 8) | tile_data[addr];
	};
	
	ScanlineFiller fill_line{&screen[ly][0], Pallete{gpu.bgp}};

	int xbeg = 0;
	if (scxmod) {
		fill_line(scxmod, 8, get_row(0));
		++xbeg;
	}

	for (int x = xbeg; x < 20; ++x)
		fill_line(0, 8, get_row(x));

	if (scxmod)
		fill_line(0, scxmod, get_row(20));

}

void update_win_scanline(const Gpu& gpu, const Memory& mem)
{
	const auto ly = gpu.ly;
	const auto lcdc = gpu.lcdc;
	const int wy = gpu.wy;
	const int wx = gpu.wx - 7;
	auto& screen = Gpu::screen;

	if (!lcdc.win_on || ly < wy || wy >= 144 || wx >= 160)
		return;

	const int wx_max = max(0, wx);
	const bool unsig_data = lcdc.tile_data != 0;
	const int data_add = ((ly - wy) & 7) * 2;
	const int map_add = (((ly - wy) >> 3)&31) * 32;
	const auto tile_data = (unsig_data
		? &mem.vram[0] : &mem.vram[0x1000]) + data_add;
	const auto map = (lcdc.win_map
		? &mem.vram[0x1C00] : &mem.vram[0x1800]) + map_add;

	const auto get_row = 
	[map, unsig_data, tile_data] (const int mapx) -> uint16_t {
		const uint8_t id = map[mapx&31];
		const int addr = (unsig_data ? id : static_cast<int8_t>(id)) * 16;
		return (tile_data[addr + 1] << 8) | tile_data[addr];
	};

	ScanlineFiller fill_line{&screen[ly][wx_max], Pallete{gpu.bgp}};

	int xbeg = 0;
	int to_draw = (160 - wx_max);
	if (wx < 0) {
		const int abswx = -wx;
		fill_line(abswx, 8, get_row(0));
		++xbeg;
		to_draw -= (8 - abswx);
	}

	for (int x = xbeg; to_draw > 0; ++x) {
		const int pend = min(8, to_draw);
		to_draw -= pend;
		fill_line(0, pend, get_row(x));
	}

}

void update_sprite_scanline(const Gpu& gpu, const Memory& mem)
{
	static_assert((sizeof(mem.oam) % 4) == 0, "");
	const auto lcdc = gpu.lcdc;

	if (!lcdc.obj_on)
		return;

	auto& screen = Gpu::screen;
	const Pallete obp0 {gpu.obp0};
	const Pallete obp1 {gpu.obp1};
	const Pallete bgp {gpu.bgp};
	const int yres = gpu.lcdc.obj_size ? 16 : 8;
	const auto ly = gpu.ly;

	for (size_t i = 0; i < sizeof(mem.oam); i += 4) {
		const int ypos = mem.oam[i] - 16;
		const int xpos = mem.oam[i + 1] - 8;
		if (ly < ypos || ly >= (ypos+yres) || xpos <= -8)
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
			row = (tile[tile_y + 1] << 8) | tile[tile_y];
		} else {
			const int pattern = mem.oam[i + 2];
			const uint8_t* tile;
			int tile_y;
			if (ly_ypos_diff < 8) {
				tile = yflip
					? &mem.vram[(pattern|0x01) * 16]
					: &mem.vram[(pattern&0xFE) * 16];
				tile_y = get_tile_y(ly_ypos_diff);
			} else {
				tile = yflip 
					? &mem.vram[(pattern&0xFE) * 16]
					: &mem.vram[(pattern|0x01) * 16]; 
				tile_y = get_tile_y(ly_ypos_diff - 8);
			}
			row = (tile[tile_y + 1] << 8) | tile[tile_y];
		}

		uint32_t* line;
		int pbeg, pend;
		if (xpos < 0) {
			line = &screen[ly][0];
			pbeg = -xpos;
			pend = 8;
		} else {
			line = &screen[ly][xpos];
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



} // namespace gbx


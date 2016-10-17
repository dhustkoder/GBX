#include <string.h>
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
static Color bg_pixels[144][160];

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



extern void update_gpu(uint8_t cycles, const Memory& mem, HWState* hwstate, Gpu* gpu);
inline void mode_hblank(Gpu* gpu, HWState* hwstate);
inline void mode_vblank(Gpu* gpu, HWState* hwstate);
inline void mode_oam(Gpu* gpu);
inline void mode_transfer(const Memory& mem, Gpu* gpu, HWState* hwstate);
inline void check_gpu_lyc(Gpu* gpu, HWState* hwstate);
inline void set_gpu_mode(Gpu::Mode mode, Gpu* gpu, HWState* hwstate);
static void update_bg_scanline(const Gpu& gpu, const Memory& mem);
static void update_win_scanline(const Gpu& gpu, const Memory& mem);
static void draw_scanlines(const Gpu& gpu, uint32_t(&pixels)[144][160]);
static void draw_sprites(const Gpu& gpu, const Memory& memory, uint32_t(&pixels)[144][160]);
inline void draw_sprite_row(uint16_t row, int xpos, int xlimit, bool xflip, bool priority,
                             const Pallete& pal, const Pallete& bgpal, uint32_t* line);


void update_gpu(const uint8_t cycles, const Memory& mem, HWState* const hwstate, Gpu* const gpu)
{
	if (!gpu->lcdc.lcd_on) {
		gpu->clock = 0;
		gpu->ly = 0;
		gpu->stat.mode = Gpu::Mode::HBlank;
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
	if (!lcdc.bg_on)
		return;

	const auto ly = gpu.ly;
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
	const int map_add = ((ly_scy_divs + ((ly_scy_mods > 7) ? 1 : 0))&31) * 32;

	const auto tile_data = (unsig_data
		? &mem.vram[0] : &mem.vram[0x1000]) + data_add;
	const auto map = (lcdc.bg_map
		? &mem.vram[0x1C00] : &mem.vram[0x1800]) + map_add;

	const Pallete pal {gpu.bgp};
	Color* line = &bg_pixels[ly][0];
	auto fill_line = [line, &pal]
	(const int pbeg, const int pend, const uint16_t row) mutable {
		for (int p = pbeg; p < pend; ++p) {
			uint8_t colnum = 0;
			if (row & (0x80 >> p))
				++colnum;
			if (row & (0x8000 >> p))
				colnum += 2;
			*line++ = pal[colnum];
		}
	};

	const auto get_row = 
	[map, scxdiv, unsig_data, tile_data] (const int mapx) -> uint16_t {
		const uint8_t id = map[(mapx + scxdiv)&31];
		const int addr = (unsig_data ? id : static_cast<int8_t>(id)) * 16;
		return (tile_data[addr + 1] << 8) | tile_data[addr];
	};
	
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
	const uint8_t wy = gpu.wy;
	const uint8_t wx = gpu.wx - 7;
	if (!lcdc.win_on || ly < wy || wy >= 144 || wx >= 160)
		return;

	const bool unsig_data = lcdc.tile_data != 0;
	const int data_add = ((ly - wy) & 7) * 2;
	const int map_add = (((ly - wy) >> 3)&31) * 32;
	const auto tile_data = (unsig_data
		? &mem.vram[0] : &mem.vram[0x1000]) + data_add;
	const auto map = (lcdc.win_map
		? &mem.vram[0x1C00] : &mem.vram[0x1800]) + map_add;

	const Pallete pal {gpu.bgp};
	Color* line = &bg_pixels[ly][wx];
	auto fill_line = [line, &pal]
	(const int pbeg, const int pend, const uint16_t row) mutable {
		for (int p = pbeg; p < pend; ++p) {
			uint8_t colnum = 0;
			if (row & (0x80 >> p))
				++colnum;
			if (row & (0x8000 >> p))
				colnum += 2;
			*line++ = pal[colnum];
		}
	};

	const auto get_row = 
	[map, unsig_data, tile_data] (const int mapx) -> uint16_t {
		const uint8_t id = map[mapx&31];
		const int addr = (unsig_data ? id : static_cast<int8_t>(id)) * 16;
		return (tile_data[addr + 1] << 8) | tile_data[addr];
	};

	const auto min = [](int x, int y) { return x > y ? y : x; };
	for (int r = 0, to_draw = (160 - wx); to_draw > 0; ++r) {
		const int pend = min(8, to_draw);
		to_draw -= pend;
		fill_line(0, pend, get_row(r));
	}

}

void draw_graphics(const Gpu& gpu, const Memory& memory, uint32_t(&pixels)[144][160])
{
	draw_scanlines(gpu, pixels);
	draw_sprites(gpu, memory, pixels);
}


void draw_scanlines(const Gpu& /*gpu*/, uint32_t(&pixels)[144][160])
{
	memcpy(pixels, bg_pixels, sizeof(uint32_t) * 144 * 160);
}


void draw_sprites(const Gpu& gpu, const Memory& memory, uint32_t(&pixels)[144][160])
{
	const Pallete pal0{gpu.obp0};
	const Pallete pal1{gpu.obp1};
	const Pallete bgpal{gpu.bgp};
	const int yres = !gpu.lcdc.obj_size ? 8 : 16;
	const auto min = [](const int x, const int y) { return x < y ? x : y; };
	const auto& oam = memory.oam;
	static_assert((sizeof(oam) % 4) == 0, "");

	for (size_t i = 0; i < sizeof(oam); i += 4) {
		const int ypos = oam[i] - 16;
		const int xpos = oam[i + 1] - 8;

		if (ypos <= -yres || xpos <= -8)
			continue;
		
		const uint8_t pattern = oam[i + 2];
		uint8_t sprite[32];
		if (yres == 8) {
			memcpy(&sprite[0], &memory.vram[pattern * 16], 16);
		} else {
			const int first = (pattern & 0xFE) * 16;
			const int second = (pattern | 0x01) * 16;
			memcpy(&sprite[0], &memory.vram[first], 16);
			memcpy(&sprite[16], &memory.vram[second], 16);
		}

		const uint8_t flags = oam[i + 3];
		const bool priority = (flags & 0x80) != 0;
		const bool yflip = (flags & 0x40) != 0;
		const bool xflip = (flags & 0x20) != 0;
		const Pallete& pal = (flags & 0x10) ? pal1 : pal0;
		const int ylimit = min(144 - ypos, yres);
		const int xlimit = min(160 - xpos, 8);

		if (!yflip) {
			for (int y = 0; y < ylimit; ++y) {
				const int yoffset = ypos + y;
				if (yoffset < 0)
					continue;
				const uint16_t row = (sprite[y*2 + 1] << 8) | sprite[y*2];
				draw_sprite_row(row, xpos, xlimit, xflip,
						priority, pal, bgpal,
						&pixels[yoffset][0]);
			}
		} else {
			for (int y = ylimit-1; y >= 0; --y) {
				const int yoffset = ypos + (ylimit - (y+1));
				if (yoffset < 0)
					break;
				const uint16_t row = (sprite[y*2 + 1] << 8) | sprite[y*2];
				draw_sprite_row(row, xpos, xlimit, xflip,
						priority,pal, bgpal, 
						&pixels[yoffset][0]);
			}
		}
	}
}




void draw_sprite_row(const uint16_t row, const int xpos, const int xlimit, 
		const bool xflip, const bool priority, const Pallete& pal,
		const Pallete& bgpal, uint32_t* const line)
{
	const auto has_priority = [&bgpal, priority, line](const int xoffset) {
		return !priority || (bgpal[0] == line[xoffset]); 
	};

	if (!xflip) {
		for (int p = 0; p < xlimit; ++p) {
			const int xoffset = p + xpos;
			if (xoffset < 0)
				continue;
			uint8_t colornum = 0;
			if (row & (0x80 >> p))
				++colornum;
			if (row & (0x8000 >> p))
				colornum += 2;
			if (colornum != 0 && has_priority(xoffset))
				line[xoffset] = pal[colornum];
		}
	} else {
		for (int p = 0; p < xlimit; ++p) {
			const int xoffset = p + xpos;
			if (xoffset < 0)
				continue;
			uint8_t colornum = 0;
			if (row & (0x01 << p))
				++colornum;
			if (row & (0x0100 << p))
				colornum += 2;
			if (colornum != 0 && has_priority(xoffset))
				line[xoffset] = pal[colornum];
		}
	}
}



} // namespace gbx


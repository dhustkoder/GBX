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

constexpr const Color kColors[4] = { White, LightGrey, DarkGrey, Black };
static uint16_t bg_scanlines[144][20];

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
inline void mode_hblank(const Memory& memory, Gpu* gpu, HWState* hwstate);
inline void mode_vblank(Gpu* gpu, HWState* hwstate);
inline void mode_oam(Gpu* gpu);
inline void mode_transfer(Gpu* gpu, HWState* hwstate);
inline void check_gpu_lyc(Gpu* gpu, HWState* hwstate);
inline void set_gpu_mode(Gpu::Mode mode, Gpu* gpu, HWState* hwstate);
static void fill_bg_scanline(const Gpu& gpu, const Memory& mem);
static void draw_bg_scanlines(const Gpu& gpu, uint32_t(&pixels)[144][160]);
static void draw_sprites(const Gpu& gpu, const Memory& memory, uint32_t(&pixels)[144][160]);
inline void draw_bg_row(uint16_t row, int length, const Pallete& pal, uint32_t* line);
inline void draw_sprite_row(uint16_t row, int xpos, int xlimit, bool xflip, const Pallete& spritepal,
                            const Pallete& bgpal, uint32_t* line);


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
	case Gpu::Mode::HBlank: mode_hblank(mem, gpu, hwstate); break;
	case Gpu::Mode::VBlank: mode_vblank(gpu, hwstate); break;
	case Gpu::Mode::OAM: mode_oam(gpu); break;
	case Gpu::Mode::Transfer: mode_transfer(gpu, hwstate); break;
	default: break;
	}
}


void mode_hblank(const Memory& mem, Gpu* const gpu, HWState* const hwstate)
{
	if (gpu->clock >= 204) {
		fill_bg_scanline(*gpu, mem);
		if (++gpu->ly != 144) {
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


void mode_transfer(Gpu* const gpu, HWState* const hwstate)
{
	if (gpu->clock >= 172) {
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


void fill_bg_scanline(const Gpu& gpu, const Memory& mem)
{
	const auto ly = gpu.ly;
	const auto lcdc = gpu.lcdc;
	const bool unsig_data = lcdc.tile_data != 0;
	const auto tile_data = unsig_data ? &mem.vram[0] : &mem.vram[0x1000];

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

	if (lcdc.bg_on && (!lcdc.win_on || ly < gpu.wy)) {
		const uint8_t lydiv = ly / 8;
		const uint8_t lymod = ly % 8;
		const uint8_t scxdiv = gpu.scx / 8;
		const uint8_t scydiv = gpu.scy / 8;
		const auto data = &tile_data[lymod * 2];
		auto map = lcdc.bg_map ? &mem.vram[0x1C00] : &mem.vram[0x1800];
		map += ((lydiv + scydiv)&31) * 32;
		fill_row(data, map, scxdiv);
	}

	if (lcdc.win_on && ly >= gpu.wy) {
		const uint8_t wy = gpu.wy;
		const uint8_t wx = gpu.wx - 7;
		if (wy < 144 && wx < 160) {
			auto map = lcdc.win_map ? &mem.vram[0x1C00] : &mem.vram[0x1800];
			fill_row(tile_data, map, 0);
		}
	}
}


void draw_graphics(const Gpu& gpu, const Memory& memory, uint32_t(&pixels)[144][160])
{
	draw_bg_scanlines(gpu, pixels);
	draw_sprites(gpu, memory, pixels);
}


void draw_bg_scanlines(const Gpu& gpu, uint32_t(&pixels)[144][160])
{
	const Pallete pallete{gpu.bgp};
	for (int y = 0; y < 144; ++y) {
		for (int x = 0; x < 20; ++x) {
			draw_bg_row(bg_scanlines[y][x], 8, 
			            pallete, &pixels[y][x*8]);
		}
	}
}


void draw_sprites(const Gpu& gpu, const Memory& memory, uint32_t(&pixels)[144][160])
{
	// TODO: add priority flag, check bgpal
	
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
			memcpy(&sprite[0], &memory.vram[(pattern & 0xFE) * 16], 16);
			memcpy(&sprite[16], &memory.vram[(pattern | 0x01) * 16], 16);
		}

		const uint8_t flags = oam[i + 3];
		const bool yflip = (flags & 0x40) != 0;
		const bool xflip = (flags & 0x20) != 0;
		const Pallete* const pal = (flags & 0x01) ? &pal1 : &pal0;
		const int ylimit = min(144 - ypos, yres);
		const int xlimit = min(160 - xpos, 8);

		if (!yflip) {
			for (int y = 0; y < ylimit; ++y) {
				const int yoffset = ypos + y;
				if (yoffset < 0)
					continue;
				const uint16_t row = (sprite[y*2 + 1] << 8) | sprite[y*2];
				draw_sprite_row(row, xpos, xlimit, xflip, *pal, 
						bgpal, &pixels[yoffset][0]);
			}
		} else {
			for (int y = ylimit-1; y >= 0; --y) {
				const int yoffset = ypos + y;
				if (yoffset < 0)
					break;
				const uint16_t row = (sprite[y*2 + 1] << 8) | sprite[y*2];
				draw_sprite_row(row, xpos, xlimit, xflip, *pal, 
						bgpal, &pixels[yoffset][0]);
			}
		}
	}
}


void draw_bg_row(const uint16_t row, const int length, const Pallete& pal, uint32_t* const line)
{
	for (int p = 0; p < length; ++p) {
		uint8_t color_num = 0;
		if (row & (0x80 >> p))
			++color_num;
		if (row & (0x8000 >> p))
			color_num += 2;
		line[p] = pal[color_num];
	}
}


void draw_sprite_row(const uint16_t row, const int xpos, const int xlimit, const bool xflip,
		const Pallete& pal, const Pallete& /*bgpal*/, uint32_t* line)
{
	if (!xflip) {
		for (int p = 0; p < xlimit; ++p) {
			const int xoffset = p + xpos;
			if (xoffset < 0)
				continue;
			uint8_t color_num = 0;
			if (row & (0x80 >> p))
				++color_num;
			if (row & (0x8000 >> p))
				color_num += 2;
			if (color_num != 0)
				line[xoffset] = pal[color_num];
		}
	} else {
		for (int p = 0; p < xlimit; ++p) {
			const int xoffset = p + xpos;
			if (xoffset < 0)
				continue;
			uint8_t color_num = 0;
			if (row & (0x01 << p))
				++color_num;
			if (row & (0x0100 << p))
				color_num += 2;
			if (color_num != 0)
				line[xoffset] = pal[color_num];
		}
	}
}



} // namespace gbx


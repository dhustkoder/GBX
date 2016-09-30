#include "Gameboy.hpp"


namespace gbx {

enum Color : uint32_t {
	Black = 0x00000000,
	White = 0xFFFFFF00,
	LightGrey = 0x90909000,
	DarkGrey = 0x55555500,
};

struct Pallete {
	constexpr explicit Pallete(const uint8_t pal)
		: colnums{static_cast<uint8_t>((pal&0x03)),
		          static_cast<uint8_t>((pal&0x0C)>>2),
		          static_cast<uint8_t>((pal&0x30)>>4),
		          static_cast<uint8_t>((pal&0xC0)>>6)} 
	{
	}
	constexpr Color operator[](const uint8_t idx) const { return colors[colnums[idx]]; }
	static constexpr const Color colors[4] = { White, LightGrey, DarkGrey, Black };
	const uint8_t colnums[4];
};

constexpr const Color Pallete::colors[4];
static uint16_t bg_scanlines[144][20];



extern void update_gpu(uint8_t cycles, const Memory& mem, HWState* hwstate, GPU* gpu);
inline void mode_hblank(const Memory& memory, GPU* gpu, HWState* hwstate);
inline void mode_vblank(GPU* gpu, HWState* hwstate);
inline void mode_oam(GPU* gpu);
inline void mode_transfer(GPU* gpu, HWState* hwstate);
inline void check_gpu_lyc(GPU* gpu, HWState* hwstate);
inline void set_gpu_mode(GPU::Mode mode, GPU* gpu, HWState* hwstate);
static void fill_bg_scanline(const GPU& gpu, const Memory& mem);
static void draw_bg_scanlines(const GPU& gpu, uint32_t(&pixels)[144][160]);
static void draw_sprites(const GPU& gpu, const Memory& memory, uint32_t(&pixels)[144][160]);



void update_gpu(const uint8_t cycles, const Memory& mem, HWState* const hwstate, GPU* const gpu)
{
	if (!gpu->lcdc.lcd_on) {
		gpu->clock = 0;
		gpu->ly = 0;
		gpu->stat.mode = GPU::Mode::HBlank;
		return;
	}

	gpu->clock += cycles;
	
	switch (gpu->stat.mode) {
	case GPU::Mode::HBlank: mode_hblank(mem, gpu, hwstate); break;
	case GPU::Mode::VBlank: mode_vblank(gpu, hwstate); break;
	case GPU::Mode::OAM: mode_oam(gpu); break;
	case GPU::Mode::Transfer: mode_transfer(gpu, hwstate); break;
	default: break;
	}
}


void mode_hblank(const Memory& mem, GPU* const gpu, HWState* const hwstate)
{
	if (gpu->clock >= 204) {
		fill_bg_scanline(*gpu, mem);
		if (++gpu->ly != 144) {
			set_gpu_mode(GPU::Mode::OAM, gpu, hwstate);
		} else {
			hwstate->RequestInt(Interrupts::vblank);
			set_gpu_mode(GPU::Mode::VBlank, gpu, hwstate);
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
		gpu->stat.mode = GPU::Mode::Transfer;
		gpu->clock -= 80;
	}
}


void mode_transfer(GPU* const gpu, HWState* const hwstate)
{
	if (gpu->clock >= 172) {
		set_gpu_mode(GPU::Mode::HBlank, gpu, hwstate);
		gpu->clock -= 172;
	}
}


void set_gpu_mode(const GPU::Mode mode, GPU* const gpu, HWState* const hwstate)
{
	const auto stat = gpu->stat;
	uint8_t int_on = 0;

	switch (mode) {
	case GPU::Mode::HBlank: int_on = stat.int_on_hblank; break;
	case GPU::Mode::VBlank: int_on = stat.int_on_vblank; break;
	case GPU::Mode::OAM: int_on = stat.int_on_oam; break;
	default: break;
	}

	if (int_on)
		hwstate->RequestInt(Interrupts::lcd);

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
			hwstate->RequestInt(Interrupts::lcd);
	}
}


void fill_bg_scanline(const GPU& gpu, const Memory& mem)
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


void draw_graphics(const GPU& gpu, const Memory& memory, uint32_t(&pixels)[144][160])
{
	draw_bg_scanlines(gpu, pixels);
	draw_sprites(gpu, memory, pixels);
}


void draw_bg_scanlines(const GPU& gpu, uint32_t(&pixels)[144][160])
{
	const Pallete pallete{gpu.bgp};

	for (uint8_t y = 0; y < 144; ++y) {
		uint32_t* const line = &pixels[y][0];
		for (uint8_t r = 0; r < 20; ++r) {
			const uint8_t xpos = r * 8;
			const uint16_t row = bg_scanlines[y][r];
			for (uint8_t pix = 0; pix < 8; ++pix) {
				uint8_t colnum = 0;
				if (row & (0x80 >> pix))
					++colnum;
				if (row & (0x8000 >> pix))
					colnum += 2;

				line[xpos + pix] = pallete[colnum];
			}
		}
	}
}


void draw_sprites(const GPU& gpu, const Memory& memory, uint32_t(&pixels)[144][160])
{
	// TODO: check sprite flags for pallete/priority/flips

	const Pallete pallete0{gpu.obp0};
	//const Pallete pallete1{gpu.obp1};

	const auto limit = [](const uint8_t len) { return len > 8 ? 8 : len; };
	const auto& oam = memory.oam;

	for (uint8_t i = 0; i < sizeof(oam); i += 4) {
		const uint8_t ypos = oam[i] - 16;
		const uint8_t xpos = oam[i + 1] - 8;

		if (ypos >= 144 || xpos >= 160)
			continue;

		const uint8_t* const tile = &memory.vram[oam[i + 2] * 16];
		//const uint8_t flags = oam[i + 3]; sprite flags, unused right now
		const uint8_t rlimit = limit(144 - ypos);
		const uint8_t plimit = limit(160 - xpos);

		for (uint8_t r = 0; r < rlimit; ++r) {
			const uint16_t row = (tile[r*2 + 1] << 8) | tile[r*2];
			uint32_t* const line = &pixels[ypos+r][0];
			for (uint8_t p = 0; p < plimit; ++p) {
				uint8_t colnum = 0;
				if (row & (0x80 >> p))
					++colnum;
				if (row & (0x8000 >> p))
					colnum += 2;
				line[xpos + p] = pallete0[colnum];
			}
		}
	}
}



} // namespace gbx


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL.h>
#include <Utix/ScopeExit.h>
#include "Gameboy.hpp"

namespace {

static bool InitSDL();
static void QuitSDL();
static void UpdateKey(const uint8_t val, const SDL_Scancode kcode, gbx::Keys* const keys);
static void UpdateGraphics(const gbx::Gameboy& gb);

}




int main(int argc, char** argv)
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s <rom>\n", argv[0]);
		return EXIT_FAILURE;
	}

	gbx::Gameboy* const gameboy = gbx::create_gameboy();
	
	if (!gameboy)
		return EXIT_FAILURE;

	const auto gameboy_guard = utix::MakeScopeExit([=] {
		gbx::destroy_gameboy(gameboy);
	});

	if (!gameboy->LoadRom(argv[1]))
		return EXIT_FAILURE;


	if (!InitSDL())
		return EXIT_FAILURE;


	const auto sdl_guard = utix::MakeScopeExit([] {
		QuitSDL();
	});

	SDL_Event event;

	while (true) {

		if (SDL_PollEvent(&event)) {
			auto etype = event.type;
			if (etype == SDL_QUIT)
				break;
			else if (etype == SDL_KEYDOWN || etype == SDL_KEYUP)
				UpdateKey(etype == SDL_KEYDOWN ? 0 : 1,
				          event.key.keysym.scancode, &gameboy->keys);
		}

		do {
			gameboy->Step();
			gameboy->UpdateGPU();
			gameboy->UpdateInterrupts();
		} while (gameboy->cpu.GetClock() < 71000);

		gameboy->cpu.SetClock(0);
		UpdateGraphics(*gameboy);
		SDL_Delay(10);
	}

	return EXIT_SUCCESS;
}









namespace {


enum Color : Uint32
{
	BLACK = 0x00000000,
	WHITE = 0xFFFFFF00,
	LIGHT_GREY = 0xD0D0D000,
	DARK_GREY = 0xA0A0A000,
};


struct Tile
{
	uint8_t data[16];
};


struct SpriteAttributes
{
	uint8_t ypos;
	uint8_t xpos;
	uint8_t pattern;
	uint8_t attr;
};



constexpr const int WIN_WIDTH = 160;
constexpr const int WIN_HEIGHT = 144;
constexpr const int PITCH = WIN_WIDTH * sizeof(Uint32);
constexpr const int GFX_BUFFER_SIZE = WIN_WIDTH * WIN_HEIGHT;

static SDL_Window* window;
static SDL_Texture* texture;
static SDL_Renderer* renderer;
static Uint32 gfx_buffer[GFX_BUFFER_SIZE] = { 0 };


static void DrawSprites(const gbx::Gameboy& gb);
static void DrawTile(const Tile& tile, const uint8_t bgp, const size_t win_x, const size_t win_y);
inline void DrawTileMap(const Tile* tiles, const uint8_t* tile_map, const uint8_t bgp, 
                         const uint8_t screen_x, const uint8_t screen_y);



static void UpdateGraphics(const gbx::Gameboy& gb)
{
	const uint8_t lcdc = gb.gpu.control;
	const uint8_t bgp = gb.gpu.bgp;
	const bool tile_data_bit = gbx::GetBit(lcdc, 4);
	const Tile* const tiles = tile_data_bit ? reinterpret_cast<const Tile*>(gb.memory.vram)
	                                        : reinterpret_cast<const Tile*>(gb.memory.vram + 0x800);
	SDL_RenderClear(renderer);

	if (gbx::GetBit(lcdc, 0)) {
		const uint8_t scy = gb.gpu.scy;
		const uint8_t scx = gb.gpu.scx;
		const uint8_t* tile_map = gbx::GetBit(lcdc, 3) ? gb.memory.vram + 0x1C00 : gb.memory.vram + 0x1800;
		DrawTileMap(tiles, tile_map, bgp, scx, scy);
	}

	if (gbx::GetBit(lcdc, 5)) {
		const uint8_t wy = gb.gpu.wy;
		const uint8_t wx = gb.gpu.wx - 7;
		const uint8_t* tile_map = gbx::GetBit(lcdc, 6) ? gb.memory.vram + 0x1C00 : gb.memory.vram + 0x1800;
		DrawTileMap(tiles, tile_map, bgp, wx, wy);
	}

	if (gbx::GetBit(lcdc, 1))
		DrawSprites(gb);

	SDL_UpdateTexture(texture, nullptr, gfx_buffer, PITCH);
	SDL_RenderCopy(renderer, texture, nullptr, nullptr);
	SDL_RenderPresent(renderer);
}




static void DrawSprites(const gbx::Gameboy& gb)
{
	const auto bgp = gb.gpu.bgp;
	const Tile* tiles = reinterpret_cast<const Tile*>(gb.memory.vram);
	const SpriteAttributes* sprites_attr = reinterpret_cast<const SpriteAttributes*>(gb.memory.oam);

	for (uint8_t i = 0; i < 40; ++i) {
		const uint8_t pattern = sprites_attr[i].pattern;
		if (pattern != 0) {
			const uint8_t ypos = sprites_attr[i].ypos - 16;
			const uint8_t xpos = sprites_attr[i].xpos - 8;
			if (xpos < 160 && ypos < 144)
				DrawTile(tiles[pattern], bgp, xpos, ypos);
		}
	}
}




inline void DrawTileMap(const Tile* const tiles, const uint8_t* const tile_map, const uint8_t bgp, 
                         const uint8_t screen_x, const uint8_t screen_y) 
{
	for (uint8_t y = 0; y < 18; ++y)
		for (uint8_t x = 0; x < 20; ++x)
			DrawTile(tiles[tile_map[(y + screen_x) * 32 + (x + screen_y)]], bgp, x * 8, y * 8);
}







static void DrawTile(const Tile& tile, const uint8_t bgp, const size_t win_x, const size_t win_y)
{

	const auto set_gfx = [](const Color pixel, size_t x, size_t y) {
		gfx_buffer[(y*WIN_WIDTH) + x] = pixel;
	};

	const auto get_color = [](const uint8_t val) -> Color {
		switch (val) {
		case 0x00: return WHITE;
		case 0x01: return LIGHT_GREY;
		case 0x02: return DARK_GREY;
		default: return BLACK;
		};
	};

	const auto get_pixel = [=](const Tile& tile, size_t x, size_t y) -> Color {
		const bool up_bit = (tile.data[y] & (0x80 >> x)) != 0;
		const bool down_bit = (tile.data[y + 1] & (0x80 >> x)) != 0;
		if (up_bit && down_bit)
			return get_color(bgp >> 6);
		else if (up_bit && !down_bit)
			return get_color((bgp & 0x30) >> 4);
		else if (!up_bit && down_bit)
			return get_color((bgp & 0x0C) >> 2);
		return get_color((bgp & 0x03));
	};

	for (size_t tile_y = 0; tile_y < 8; ++tile_y) {
		for (size_t tile_x = 0; tile_x < 8; ++tile_x) {
			const auto pixel = get_pixel(tile, tile_x, tile_y * 2);
			set_gfx(pixel, win_x + tile_x, win_y + tile_y);
		}
	}
}











static void UpdateKey(const uint8_t val, const SDL_Scancode kcode, gbx::Keys* const keys)
{
	switch (kcode) {
	case SDL_SCANCODE_Z: keys->bit.a = val; break;
	case SDL_SCANCODE_X: keys->bit.b = val; break;
	case SDL_SCANCODE_C: keys->bit.select = val; break;
	case SDL_SCANCODE_V: keys->bit.start = val; break;
	case SDL_SCANCODE_RIGHT: keys->bit.right = val; break;
	case SDL_SCANCODE_LEFT: keys->bit.left = val; break;
	case SDL_SCANCODE_UP: keys->bit.up = val; break;
	case SDL_SCANCODE_DOWN: keys->bit.down = val; break;
	}
}















static bool InitSDL()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "failed to init SDL2: %s\n", SDL_GetError());
		return false;
	}

	window = SDL_CreateWindow("SDL2",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WIN_WIDTH * 2, WIN_HEIGHT * 2, 0);

	if (!window) {
		fprintf(stderr, "failed to create SDL_Window: %s\n", SDL_GetError());
		goto free_sdl;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	if (!renderer) {
		fprintf(stderr, "failed to create SDL_Renderer: %s\n", SDL_GetError());
		goto free_window;
	}


	texture = SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_TARGET,
		WIN_WIDTH, WIN_HEIGHT);

	if (!texture) {
		fprintf(stderr, "failed to create SDL_Texture: %s\n", SDL_GetError());
		goto free_renderer;
	}

	return true;

free_renderer:
	SDL_DestroyRenderer(renderer);
free_window:
	SDL_DestroyWindow(window);
free_sdl:
	SDL_Quit();

	return false;
}






static void QuitSDL()
{
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}













}

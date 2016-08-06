#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL.h>
#include <Utix/ScopeExit.h>
#include "Gameboy.hpp"

namespace {

static bool init_sdl();
static void quit_sdl();
static void update_graphics(gbx::Gameboy* const gb);

static SDL_Event event;

}




int main(int argc, char** argv)
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s <rom>\n", argv[0]);
		return EXIT_FAILURE;
	}

	gbx::Gameboy* gameboy = gbx::create_gameboy();
	
	if (!gameboy)
		return EXIT_FAILURE;

	const auto gameboy_guard = utix::MakeScopeExit([=] {
		gbx::destroy_gameboy(gameboy);
	});

	if (!gameboy->LoadRom(argv[1]))
		return EXIT_FAILURE;


	if (!init_sdl())
		return EXIT_FAILURE;


	const auto sdl_guard = utix::MakeScopeExit([] {
		quit_sdl();
	});


	bool need_update = true;
	while (true) {

		if (SDL_PollEvent(&event))
			if (event.type == SDL_QUIT)
				break;

		if (need_update) {
			gameboy->Step();
			gameboy->UpdateGPU();
			gameboy->UpdateInterrupts();

			if (gameboy->cpu.GetPC() == 0x2800) {
				update_graphics(gameboy);
				need_update = false;
			}
		}
	}

	return EXIT_SUCCESS;
}









namespace {

struct Tile
{
	uint8_t data[16];
};


constexpr const int WIN_WIDTH = 160;
constexpr const int WIN_HEIGHT = 144;
constexpr const int PITCH = WIN_WIDTH * sizeof(Uint32);
constexpr const int GFX_BUFFER_SIZE = WIN_WIDTH * WIN_HEIGHT;

static SDL_Window* window;
static SDL_Texture* texture;
static SDL_Renderer* renderer;
static Uint32 gfx_buffer[GFX_BUFFER_SIZE] = { 0 };





static void draw_tile(const Tile& tile, size_t win_x, size_t win_y)
{
	// TODO: endiannes check
	enum Color : Uint32
	{
		BLACK = 0x00000000,
		WHITE = 0xFFFFFF00,
		LIGHT_GREY = 0xD3D3D300,
		DARK_GREY = 0xA9A9A900,
	};

	const auto check_bit = [](const uint8_t byte, size_t right_shift) {
		return (byte & (0x80 >> right_shift)) != 0;
	};

	const auto set_pixel = [](const Uint32 value, size_t x, size_t y) {
		gfx_buffer[(y*WIN_WIDTH) + x] = value;
	};

	const auto get_color = [=](const Tile& tile, size_t line, size_t pixel) {
		const bool bit_1 = check_bit(tile.data[line * 2], pixel);
		const bool bit_2 = check_bit(tile.data[line * 2 + 1], pixel);
		return bit_1 ? bit_2 ? BLACK : DARK_GREY
                             : bit_2 ? LIGHT_GREY : WHITE;
	};

	for (size_t tile_y = 0; tile_y < 8; ++tile_y) {
		for (size_t tile_x = 0; tile_x < 8; ++tile_x) {
			const auto color = get_color(tile, tile_y, tile_x);
			set_pixel(color, win_x + tile_x, win_y + tile_y);
		}
	}
}







static void update_graphics(gbx::Gameboy* const gb)
{
	const Tile* tiles = reinterpret_cast<Tile*>(gb->memory.vram);

	for (size_t y = 0; y < 18; ++y)
		for (size_t x = 0; x < 20; ++x)
			draw_tile(*tiles++, x * 8, y * 8);

	SDL_RenderClear(renderer);
	SDL_UpdateTexture(texture, nullptr, gfx_buffer, PITCH);
	SDL_RenderCopy(renderer, texture, nullptr, nullptr);
	SDL_RenderPresent(renderer);
}








static bool init_sdl()
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






static void quit_sdl()
{
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}













}

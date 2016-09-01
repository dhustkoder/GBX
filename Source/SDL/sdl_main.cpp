#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <Utix/ScopeExit.h>
#include "Gameboy.hpp"



namespace {

static bool InitSDL();
static void QuitSDL();
static void UpdateKey(gbx::KeyState state, SDL_Scancode keycode, gbx::Keys* keys);
static void RenderGraphics(const gbx::GPU& gpu);


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

	constexpr const auto ESCAPE = SDL_SCANCODE_ESCAPE;
	SDL_Event event;
	Uint32 last_ticks = 0;
	size_t itr = 0;
	
	while (true) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.scancode == ESCAPE)
					goto SDL_QUIT_EVENT;

				UpdateKey(gbx::KEYDOWN,
					event.key.keysym.scancode,
					&gameboy->keys);
				break;
			case SDL_KEYUP:
				UpdateKey(gbx::KEYUP,
					event.key.keysym.scancode,
					&gameboy->keys);
				break;
			case SDL_QUIT:
				goto SDL_QUIT_EVENT;
			default:
				break;
			}
		}

		gameboy->Run(69905);
		
		if (gameboy->gpu.stat.mode != gbx::GPU::Mode::VBLANK)
			RenderGraphics(gameboy->gpu);
		
		//SDL_Delay(15);

		const auto ticks = SDL_GetTicks();
		if (ticks > (last_ticks + 1000)) {
			printf("%zu\n", itr);
			itr = 0;
			last_ticks = ticks;
		}

		++itr;
	}


SDL_QUIT_EVENT:


	return EXIT_SUCCESS;
}









namespace {

constexpr const int WIN_WIDTH = 160;
constexpr const int WIN_HEIGHT = 144;

static SDL_Window* window;
static SDL_Texture* texture;
static SDL_Renderer* renderer;



static void RenderGraphics(const gbx::GPU& gpu)
{
	const auto lcdc = gpu.lcdc;

	SDL_RenderClear(renderer);
	
	if (lcdc.lcd_on) {
		int pitch;
		void* pixels;
		if (SDL_LockTexture(texture, nullptr, &pixels, &pitch) == 0) {
			draw_bg_scanlines(gpu, static_cast<uint32_t*>(pixels));
			SDL_UnlockTexture(texture);
			SDL_RenderCopy(renderer, texture, nullptr, nullptr);
		} else {
			fprintf(stderr, "failed to lock texture: %s\n", SDL_GetError());
		}
	}

	SDL_RenderPresent(renderer);
}





static void UpdateKey(gbx::KeyState state, SDL_Scancode keycode, gbx::Keys* keys)
{
	switch (keycode) {
	case SDL_SCANCODE_Z: keys->pad.a = state; break;
	case SDL_SCANCODE_X: keys->pad.b = state; break;
	case SDL_SCANCODE_C: keys->pad.select = state; break;
	case SDL_SCANCODE_V: keys->pad.start = state; break;
	case SDL_SCANCODE_RIGHT: keys->pad.right = state; break;
	case SDL_SCANCODE_LEFT: keys->pad.left = state; break;
	case SDL_SCANCODE_UP: keys->pad.up = state; break;
	case SDL_SCANCODE_DOWN: keys->pad.down = state; break;
	default: break;
	}
}




static bool InitSDL()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "failed to init SDL2: %s\n", SDL_GetError());
		return false;
	}

	window = SDL_CreateWindow("GBX",
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
		SDL_TEXTUREACCESS_STREAMING,
		WIN_WIDTH, WIN_HEIGHT);

	if (!texture) {
		fprintf(stderr, "failed to create SDL_Texture: %s\n", SDL_GetError());
		goto free_renderer;
	}

	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);
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

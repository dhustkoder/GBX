#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "Gameboy.hpp"


namespace {

static bool init_sdl();
static void quit_sdl();
static void update_key(gbx::Keys::State state, SDL_Scancode keycode, gbx::Keys* keys);
static void render_graphics(gbx::Gameboy* gb);

}


int main(int argc, char** argv)
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s <rom>\n", argv[0]);
		return EXIT_FAILURE;
	}

	const gbx::owner<gbx::Gameboy*> gameboy = gbx::create_gameboy(argv[1]);
	
	if (gameboy == nullptr)
		return EXIT_FAILURE;

	const auto gameboy_guard = gbx::finally([=]{
		gbx::destroy_gameboy(gameboy); 
	});

	if (!init_sdl())
		return EXIT_FAILURE;

	const auto sdl_guard = gbx::finally([]{
		quit_sdl();
	});

	SDL_Event event;
	Uint32 last_ticks = 0;
	size_t itr = 0;
	
	while (true) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
					goto break_loop;
				} else {
					update_key(gbx::Keys::State::Down,
					           event.key.keysym.scancode,
					           &gameboy->keys);
				}
				break;
			case SDL_KEYUP:
				update_key(gbx::Keys::State::Up,
				           event.key.keysym.scancode,
				           &gameboy->keys);
				break;
			case SDL_QUIT:
				goto break_loop;
			default:
				break;
			}
		}

		gameboy->Run(69905);
		
		if (gameboy->gpu.stat.mode != gbx::Gpu::Mode::VBlank)
			render_graphics(gameboy);
		
		//SDL_Delay(15);

		const auto ticks = SDL_GetTicks();
		if (ticks > (last_ticks + 1000)) {
			printf("%zu\n", itr);
			itr = 0;
			last_ticks = ticks;
		}

		++itr;
	}

break_loop:


	return EXIT_SUCCESS;
}


namespace {

constexpr const int WinWidth = 160;
constexpr const int WinHeight = 144;

static gbx::owner<SDL_Window*> window = nullptr;
static gbx::owner<SDL_Texture*> texture = nullptr;
static gbx::owner<SDL_Renderer*> renderer = nullptr;


void render_graphics(gbx::Gameboy* const gb)
{
	const auto lcdc = gb->gpu.lcdc;

	SDL_RenderClear(renderer);
	
	if (lcdc.lcd_on) {
		int pitch;
		void* pixels;
		if (SDL_LockTexture(texture, nullptr, &pixels, &pitch) == 0) {
			using ArrPtr = uint32_t(*)[WinHeight][WinWidth];
			draw_graphics(gb->gpu, gb->memory, *static_cast<ArrPtr>(pixels));
			SDL_UnlockTexture(texture);
			SDL_RenderCopy(renderer, texture, nullptr, nullptr);
		} else {
			fprintf(stderr, "failed to lock texture: %s\n", SDL_GetError());
		}
	}

	SDL_RenderPresent(renderer);
}


void update_key(gbx::Keys::State state, SDL_Scancode keycode, gbx::Keys* keys)
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


bool init_sdl()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "failed to init SDL2: %s\n", SDL_GetError());
		return false;
	}

	window = SDL_CreateWindow("GBX",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WinWidth * 2, WinHeight * 2, 0);

	if (window == nullptr) {
		fprintf(stderr, "failed to create SDL_Window: %s\n", SDL_GetError());
		goto free_sdl;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	if (renderer == nullptr) {
		fprintf(stderr, "failed to create SDL_Renderer: %s\n", SDL_GetError());
		goto free_window;
	}


	texture = SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_STREAMING,
		WinWidth, WinHeight);

	if (texture == nullptr) {
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


void quit_sdl()
{
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}





} // anonymous namespace


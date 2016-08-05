#include <stdio.h>
#include <SDL2/SDL.h>
#include <Utix/ScopeExit.h>
#include "Gameboy.hpp"


bool init_sdl();
void quit_sdl();
void update_graphics(gbx::Gameboy* const gb);

constexpr const int WIN_WIDTH = 160;
constexpr const int WIN_HEIGHT = 144;
constexpr const int PITCH = WIN_WIDTH * sizeof(Uint32);
constexpr const int GFX_BUFFER_SIZE = WIN_WIDTH * WIN_HEIGHT;


static SDL_Event event;
static SDL_Window* window;
static SDL_Texture* texture;
static SDL_Renderer* renderer;
static Uint32 gfx_buffer[GFX_BUFFER_SIZE] = { 0 };

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

	
	while (true) {
		SDL_PollEvent(&event);
		if(event.type == SDL_QUIT)
			break;

		SDL_RenderClear(renderer);

		gameboy->Step();
		gameboy->UpdateGPU();
		gameboy->UpdateInterrupts();
		update_graphics(gameboy);

		SDL_UpdateTexture(texture, nullptr, gfx_buffer, PITCH);
		SDL_RenderCopy(renderer, texture, nullptr, nullptr);
		SDL_RenderPresent(renderer);
	}


	return EXIT_SUCCESS;
}








void update_graphics(gbx::Gameboy*)
{



}














bool init_sdl()
{

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "failed to init SDL2: %s\n", SDL_GetError());
		return false;
	}

	window = SDL_CreateWindow("SDL2", 
	                          SDL_WINDOWPOS_CENTERED, 
	                          SDL_WINDOWPOS_CENTERED, 
	                          160, 144, 0);
	if (!window) {
		fprintf(stderr, "failed to create SDL_Window: %s\n", SDL_GetError());
		goto free_sdl;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	if (!renderer) {
		fprintf(stderr, "failed to create SDL_Renderer: %s\n", SDL_GetError());
		goto free_window;
	}


	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 160, 144);

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






void quit_sdl()
{
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}



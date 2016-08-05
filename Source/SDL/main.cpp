#include <stdio.h>
#include <SDL2/SDL.h>



bool init_sdl();
void quit_sdl();

struct SDL_Quit_Guard { 
	~SDL_Quit_Guard() { quit_sdl(); }
};

static SDL_Event event;
static SDL_Window* window;
static SDL_Texture* texture;
static SDL_Renderer* renderer;


int main(int argc, char**)
{
	int pitch = 160 * sizeof(Uint32);

	if(!init_sdl())
		return EXIT_FAILURE;

	const SDL_Quit_Guard sdl_quit_guard{};

	constexpr const int buffer_size = 160 * 144;

	printf("%i\n", pitch);
	Uint32 buffer[buffer_size] = { 0 };
	buffer[buffer_size/2 + 160/2-1] = 0xff00ff00;

	while (1) {
		SDL_PollEvent(&event);
		if(event.type == SDL_QUIT)
			break;

		SDL_RenderClear(renderer);
		SDL_UpdateTexture(texture, nullptr, buffer, pitch);
		SDL_RenderCopy(renderer, texture, nullptr, nullptr);
		SDL_RenderPresent(renderer);
	}


	return EXIT_SUCCESS;
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
		return false;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	if(!renderer) {
		fprintf(stderr, "failed to create SDL_Renderer: %s\n", SDL_GetError());
		goto free_window;
	}


	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 160, 144);

	if(!texture) {
		fprintf(stderr, "failed to create SDL_Texture: %s\n", SDL_GetError());
		goto free_renderer;
	}

	return true;

free_renderer:
	SDL_DestroyRenderer(renderer);
free_window:
	SDL_DestroyWindow(window);

	return false;
}






void quit_sdl()
{
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}



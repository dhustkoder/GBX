#include <stdio.h>
#include <SDL2/SDL.h>

















int main(int argc, char**)
{
	if( SDL_InitSubSystem( SDL_INIT_VIDEO ) ) {
		fprintf(stderr, "failed to initialize SDL2: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}
	

	SDL_Window* window = SDL_CreateWindow("Chip8 - SdlRender", 
	                                      SDL_WINDOWPOS_CENTERED, 
	                                      SDL_WINDOWPOS_CENTERED, 
	                                      160, 144, 0);
	if (!window) {
		fprintf(stderr, "failed to create SDL_Window: %s", SDL_GetError());
		return EXIT_FAILURE;
	}


	SDL_Surface* screenSurface = SDL_GetWindowSurface( window );


	SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, 0xFF, 0xFF, 0xFF ) );


	SDL_UpdateWindowSurface( window );


	SDL_Delay( 2000 );

	SDL_DestroyWindow(window);

	return EXIT_SUCCESS;
}




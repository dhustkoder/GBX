#ifndef GBX_VIDEO_HPP_
#define GBX_VIDEO_HPP_
#include <stdint.h>
#include "SDL.h"


inline void render_graphics(const uint32_t* const pixels, const uint_fast32_t len)
{
	extern SDL_Texture* texture;
	extern SDL_Renderer* renderer;

	int pitch;
	void* dest;
	if (SDL_LockTexture(texture, nullptr, &dest, &pitch) == 0) {
		memcpy(dest, pixels, len);
		SDL_UnlockTexture(texture);
		SDL_RenderCopy(renderer, texture, nullptr, nullptr);
		SDL_RenderPresent(renderer);
	} else {
		const char* const err = SDL_GetError();
		fprintf(stderr, "failed to lock texture: %s\n", err);
	}
}


#endif

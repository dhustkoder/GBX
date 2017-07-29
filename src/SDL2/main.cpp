#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"
#include "SDL_audio.h"
#include "input.hpp"
#include "video.hpp"
#include "audio.hpp"
#include "gbx.hpp"


static bool init_sdl();
static void quit_sdl();


constexpr const int kWinWidth = 160;
constexpr const int kWinHeight = 144;

static SDL_Window* window = nullptr;
static SDL_Texture* texture = nullptr;
static SDL_Renderer* renderer = nullptr;
static SDL_AudioDeviceID audio_device = 0;
static SDL_Event events;


int main(int argc, char** argv)
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s <rom>\n", argv[0]);
		return EXIT_FAILURE;
	}

	if (!init_sdl())
		return EXIT_FAILURE;

	const int exitcode = gbx::gbx_main(argc, argv);

	quit_sdl();
	return exitcode;
}


bool process_inputs(gbx::Gameboy* const gb)
{
	constexpr const uint32_t keycodes[8] {
		SDL_SCANCODE_Z, SDL_SCANCODE_X, 
		SDL_SCANCODE_C, SDL_SCANCODE_V,
		SDL_SCANCODE_RIGHT, SDL_SCANCODE_LEFT, 
		SDL_SCANCODE_UP, SDL_SCANCODE_DOWN
	};

	const auto update_key = [gb] (const gbx::KeyState state, const uint32_t keycode) {
		gbx::update_joypad(keycodes, keycode, state, &gb->hwstate, &gb->joypad);
	};

	while (SDL_PollEvent(&events)) {
		switch (events.type) {
		case SDL_KEYDOWN:
			update_key(gbx::KeyState::Down, events.key.keysym.scancode);
			break;
		case SDL_KEYUP:
			update_key(gbx::KeyState::Up, events.key.keysym.scancode);
			break;

		case SDL_QUIT:
			return false;
		default:
			break;
		}
	}

	return true;
}


void render_graphics(const uint32_t* const pixels, const uint_fast32_t len)
{
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

void queue_sound_buffer(const uint8_t* const buffer, const uint_fast32_t len)
{
	while (SDL_GetQueuedAudioSize(audio_device) > len)
		SDL_Delay(1);

	if (SDL_QueueAudio(audio_device, buffer, len) != 0)
		fprintf(stderr, "Failed to queue audio: %s\n", SDL_GetError());
}


bool init_sdl()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		fprintf(stderr, "failed to init SDL2: %s\n", SDL_GetError());
		return false;
	}

	window = SDL_CreateWindow("GBX",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		kWinWidth * 2, kWinHeight * 2, SDL_WINDOW_RESIZABLE);

	if (window == nullptr) {
		fprintf(stderr, "failed to create SDL_Window: %s\n",
		        SDL_GetError());
		goto free_sdl;
	}

	renderer = SDL_CreateRenderer(window, -1,
	  SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (renderer == nullptr) {
		fprintf(stderr, "failed to create SDL_Renderer: %s\n",
		        SDL_GetError());
		goto free_window;
	}


	texture = SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		kWinWidth, kWinHeight);

	if (texture == nullptr) {
		fprintf(stderr, "failed to create SDL_Texture: %s\n",
		        SDL_GetError());
		goto free_renderer;
	}

	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);


	SDL_AudioSpec want;
	SDL_zero(want);
	want.freq = 44100;
	want.format = AUDIO_S16SYS;
	want.channels = 1;
	want.samples = 1024;

	if ((audio_device = SDL_OpenAudioDevice(nullptr, 0, &want, nullptr, 0)) == 0) {
		fprintf(stderr, "Failed to open audio device: %s\n", SDL_GetError());
		goto free_texture;
	}

	SDL_PauseAudioDevice(audio_device, 0);
	return true;

free_texture:
	SDL_DestroyTexture(texture);
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


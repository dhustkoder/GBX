#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"
#include "common.hpp"
#include "gameboy.hpp"

namespace {

static bool init_sdl(bool enable_joystick);
static void quit_sdl();
static bool update_events(SDL_Event* events, gbx::Gameboy* gb);
static void render_graphics(gbx::Gameboy* gb);

}


int main(int argc, char** argv)
{
	static_assert(sizeof(Uint32) == sizeof(uint32_t), "");
	static_assert(sizeof(Uint16) == sizeof(uint16_t), "");
	static_assert(sizeof(Uint8) == sizeof(uint8_t), "");
	static_assert(sizeof(Sint32) == sizeof(int32_t), "");
	static_assert(sizeof(Sint16) == sizeof(int16_t), "");
	static_assert(sizeof(Sint8) == sizeof(int8_t), "");

	if (argc < 2) {
		fprintf(stderr, "usage: %s <rom>\n", argv[0]);
		return EXIT_FAILURE;
	}

	gbx::owner<gbx::Gameboy* const> gameboy = gbx::create_gameboy(argv[1]);
	
	if (gameboy == nullptr)
		return EXIT_FAILURE;

	const auto gameboy_guard = gbx::finally([=]{
		gbx::destroy_gameboy(gameboy); 
	});

	if (!init_sdl(argc > 2 && strcmp(argv[2], "-joy") == 0))
		return EXIT_FAILURE;

	const auto sdl_guard = gbx::finally([]{
		quit_sdl();
	});

	SDL_Event events;
	uint32_t last_ticks = 0;
	int fps = 0;

	while (update_events(&events, gameboy)) {
		gbx::run_for(70224, gameboy);
		render_graphics(gameboy);
		++fps;
		const auto ticks = SDL_GetTicks();
		if (ticks >= (last_ticks + 1000)) {
			printf("FPS: %d\r", fps);
			fflush(stdout);
			last_ticks = ticks;
			fps = 0;
		}
	}

	return EXIT_SUCCESS;
}


namespace {

constexpr const int kWinWidth = 160;
constexpr const int kWinHeight = 144;
constexpr const int16_t kJoyaxisDeadZone = INT16_MAX - 3000;

static struct Joyaxis {
	uint32_t kcode_high;
	uint32_t kcode_low;
	int16_t value;
	uint8_t id;
} joyaxis[2];

static uint32_t keycodes[8] {
	SDL_SCANCODE_Z, SDL_SCANCODE_X, 
	SDL_SCANCODE_C, SDL_SCANCODE_V,
	SDL_SCANCODE_RIGHT, SDL_SCANCODE_LEFT, 
	SDL_SCANCODE_UP, SDL_SCANCODE_DOWN
};

static gbx::owner<SDL_Joystick*> joystick = nullptr;
static gbx::owner<SDL_Window*> window = nullptr;
static gbx::owner<SDL_Texture*> texture = nullptr;
static gbx::owner<SDL_Renderer*> renderer = nullptr;


bool update_events(SDL_Event* const events, gbx::Gameboy* const gb)
{
	using State = gbx::Joypad::KeyState;
	constexpr const auto escape = SDL_SCANCODE_ESCAPE;

	const auto update_key =
	[gb] (const State state, const uint32_t keycode) {
		gbx::update_joypad(keycodes, keycode, state,
		                   &gb->hwstate, &gb->joypad);
	};

	while (SDL_PollEvent(events)) {
		switch (events->type) {
		case SDL_KEYDOWN:
			if (events->key.keysym.scancode != escape) {
				update_key(State::Down,
				           events->key.keysym.scancode);
			} else {
				return false;
			}
			break;
		case SDL_KEYUP: 
			update_key(State::Up, events->key.keysym.scancode);
			break;
		case SDL_JOYBUTTONDOWN: 
			update_key(State::Down, events->jbutton.button);
			break;
		case SDL_JOYBUTTONUP:
			update_key(State::Up, events->jbutton.button);
			break;
		case SDL_JOYAXISMOTION: {
			const auto axis_id = events->jaxis.axis;
			const auto axis_value = events->jaxis.value;
			auto& jaxis = axis_id == joyaxis[0].id 
			  ? joyaxis[0] : joyaxis[1];

			if (jaxis.value != axis_value) {
				const auto state = 
				  abs(axis_value) > kJoyaxisDeadZone 
				  ? State::Down : State::Up;
				const auto value = state == State::Down
				  ? axis_value : jaxis.value;
				const auto kcode = value > 0 
				  ? jaxis.kcode_high : jaxis.kcode_low;

				update_key(state, kcode);
				jaxis.value = axis_value;
			}
			break;
		}
		case SDL_QUIT:
			return false;
		default:
			break;
		}
	}

	return true;
}


void render_graphics(gbx::Gameboy* const gb)
{
	if (gb->gpu.lcdc.lcd_on) {
		int pitch;
		void* pixels;
		if (SDL_LockTexture(texture, nullptr, &pixels, &pitch) == 0) {
			const uint32_t* const screen = &gbx::Gpu::screen[0][0];
			const size_t size = sizeof(gbx::Gpu::screen);
			memcpy(pixels, screen, size);
			SDL_UnlockTexture(texture);
			SDL_RenderCopy(renderer, texture, nullptr, nullptr);
		} else {
			const char* const err = SDL_GetError();
			fprintf(stderr, "failed to lock texture: %s\n", err);
		}
	} else {
		SDL_RenderClear(renderer);
	}
	
	SDL_RenderPresent(renderer);
}


bool setup_joystick()
{
	const int num_devices = SDL_NumJoysticks();
	if (num_devices < 0) {
		fprintf(stderr, "error: %s\n", SDL_GetError());
		return false;
	} else if (num_devices == 0) {
		fprintf(stderr, "error: no joystick found.\n");
		return false;
	}

	joystick = SDL_JoystickOpen(0);
	auto joystick_guard = gbx::finally([] {
		SDL_JoystickClose(joystick);
	});

	if (joystick == nullptr) {
		fprintf(stderr, "error: %s\n", SDL_GetError());
		return false;
	}

	constexpr const char* const keywords[] {
		"A", "B", "SELECT", "START",
		"RIGHT", "LEFT", "UP", "DOWN"
	};

	const auto is_quit_event = [](const SDL_Event& ev) {
		if (ev.type == SDL_QUIT)
			return true;
		else if (ev.type == SDL_KEYDOWN)
			return ev.key.keysym.scancode == SDL_SCANCODE_ESCAPE;
		return false;
	};

	SDL_Event ev;
	uint32_t prev_button = UINT32_MAX;
	uint8_t prev_axis_id = UINT8_MAX;

	for (int i = 0; i < 8; ++i) {
		printf("PRESS KEY FOR %s: ", keywords[i]);
		fflush(stdout);

		while (true) {
			if (!SDL_PollEvent(&ev))
				continue;

			if (is_quit_event(ev)) {
				putchar('\n');
				return false;
			} else if (ev.type == SDL_JOYBUTTONDOWN &&
			     ev.jbutton.button != prev_button) {
				prev_button = keycodes[i] = ev.jbutton.button;
				printf("%d\n", prev_button);
				break;
			} else if (i > 3 && ev.type == SDL_JOYAXISMOTION) {
				constexpr const uint32_t scancodes[] {
					SDL_SCANCODE_RIGHT, SDL_SCANCODE_LEFT,
					SDL_SCANCODE_UP, SDL_SCANCODE_DOWN
				};

				const auto axis_id = ev.jaxis.axis;
				const auto axis_value = ev.jaxis.value;

				if (abs(axis_value) < kJoyaxisDeadZone ||
				     axis_id == prev_axis_id)
					continue;

				if (i == 5 || i == 7)
					--i;

				auto& jaxis = i < 6 ? joyaxis[0] : joyaxis[1];
				jaxis.id = axis_id;
				if (axis_value > 0) {
					jaxis.kcode_high = scancodes[i - 4];
					jaxis.kcode_low = scancodes[i - 3];
				} else {
					jaxis.kcode_low = scancodes[i - 4];
					jaxis.kcode_high = scancodes[i - 3];
				}
				++i;

				prev_axis_id = axis_id;
				printf("axis %u\n", axis_id);
				break;
			}
		}
	}

	joystick_guard.Abort();
	return true;
}

bool init_sdl(const bool enable_joystick)
{
	const auto joystick_flag = enable_joystick ? SDL_INIT_JOYSTICK : 0;
	if (SDL_Init(SDL_INIT_VIDEO | joystick_flag) != 0) {
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
		fprintf(stderr, "failed to create SDL_Texture: %s\n", SDL_GetError());
		goto free_renderer;
	}

	if (enable_joystick && !setup_joystick())
		goto free_texture;

	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);
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
	SDL_JoystickClose(joystick);
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}


} // anonymous namespace


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "Gameboy.hpp"


namespace {

static bool init_sdl(bool enable_joystick);
static void quit_sdl();
static void update_key(gbx::Keys::State state, Uint32 keycode, gbx::Keys* keys);
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

	if (!init_sdl(argc > 2 && strcmp(argv[2], "-joy") == 0))
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

			case SDL_JOYBUTTONDOWN:
				update_key(gbx::Keys::State::Down,
				           event.jbutton.button,
					   &gameboy->keys);
				break;

			case SDL_JOYBUTTONUP:
				update_key(gbx::Keys::State::Up,
				           event.jbutton.button,
					   &gameboy->keys);
				break;

			case SDL_JOYAXISMOTION: {
				gbx::Keys::State state;
				Uint32 keycode;
				Uint32 codes[2];
				const auto value = event.jaxis.value;
				const auto axis = event.jaxis.axis;

				if (value > -30000 && value < 30000)
					state = gbx::Keys::State::Up;
				else
					state = gbx::Keys::State::Down;

				if (axis == 0) {
					codes[0] = SDL_SCANCODE_LEFT;
					codes[1] = SDL_SCANCODE_RIGHT;
				} else if (axis == 1) {
					codes[0] = SDL_SCANCODE_UP;
					codes[1] = SDL_SCANCODE_DOWN;
				} else {
					break;
				}
				
				if (state == gbx::Keys::State::Down) {
					keycode = codes[value > 0 ? 1 : 0];
					update_key(state, keycode,
					            &gameboy->keys);
				} else {
					update_key(state, codes[0],
					           &gameboy->keys);
					update_key(state, codes[1],
					           &gameboy->keys);
				}

				break;
			}

			case SDL_QUIT:
				goto break_loop;
			default:
				break;
			}
		}

		gameboy->Run(70224);
		render_graphics(gameboy);
		SDL_Delay(15);

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

Uint32 input_keys[8] {
	SDL_SCANCODE_Z, SDL_SCANCODE_X, 
	SDL_SCANCODE_C, SDL_SCANCODE_V,
	SDL_SCANCODE_RIGHT, SDL_SCANCODE_LEFT, 
	SDL_SCANCODE_UP, SDL_SCANCODE_DOWN
};

static gbx::owner<SDL_Joystick*> joystick = nullptr;
static gbx::owner<SDL_Window*> window = nullptr;
static gbx::owner<SDL_Texture*> texture = nullptr;
static gbx::owner<SDL_Renderer*> renderer = nullptr;
static bool setup_joystick();

void render_graphics(gbx::Gameboy* const gb)
{
	SDL_RenderClear(renderer);

	if (gb->gpu.lcdc.lcd_on) {
		int pitch;
		void* pixels;
		if (SDL_LockTexture(texture, nullptr, &pixels, &pitch) == 0) {
			using ArrPtr = uint32_t(*)[WinHeight][WinWidth];
			draw_graphics(gb->gpu, gb->memory,
			              *static_cast<ArrPtr>(pixels));
			SDL_UnlockTexture(texture);
			SDL_RenderCopy(renderer, texture, nullptr, nullptr);
		} else {
			fprintf(stderr, "failed to lock texture: %s\n", SDL_GetError());
		}
	}

	SDL_RenderPresent(renderer);
}


void update_key(const gbx::Keys::State state, const Uint32 keycode,
                 gbx::Keys* const keys)
{
	int key_index = 0;
	for (const auto key : input_keys) {
		if (key == keycode)
			break;
		++key_index;
	}

	switch (key_index) {
	case 0: keys->pad.a = state; break;
	case 1: keys->pad.b = state; break;
	case 2: keys->pad.select = state; break;
	case 3: keys->pad.start = state; break;
	case 4: keys->pad.right = state; break;
	case 5: keys->pad.left = state; break;
	case 6: keys->pad.up = state; break;
	case 7: keys->pad.down = state; break;
	default: break;
	}
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
		WinWidth * 2, WinHeight * 2, SDL_WINDOW_RESIZABLE);

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

	if (enable_joystick && !setup_joystick()) {
		goto free_texture;
	}

	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);
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

	if (joystick == nullptr) {
		fprintf(stderr, "error: %s\n", SDL_GetError());
		return false;
	}

	const char* const keywords[] {
		"A", "B", "SELECT", "START"
	};

	SDL_Event ev;
	Uint32 keycode = -1;

	for (int i = 0; i < 4; ++i) {
		printf("press key for %s: ", keywords[i]);
		fflush(stdout);
		
		while (true) {
			SDL_PollEvent(&ev);
			if (ev.type == SDL_JOYBUTTONDOWN
			    && ev.jbutton.button != keycode) {
				keycode = ev.jbutton.button;
				break;
			}
		}

		input_keys[i] = keycode;
		printf("%d\n", keycode);
	}

	return true;
}



} // anonymous namespace


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
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

	SDL_Event events;
	uint32_t last_ticks = 0;
	int fps = 0;
	
	while (update_events(&events, gameboy)) {	
		gameboy->Run(70224);
		render_graphics(gameboy);
		SDL_Delay(15);
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

constexpr const int WinWidth = 160;
constexpr const int WinHeight = 144;

uint32_t keycodes[8] {
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
	const auto& scancode = events->key.keysym.scancode;
	const auto& jbutton = events->jbutton;
	
	const auto update_key =
	[gb] (const State state, const uint32_t keycode) {
		gbx::update_joypad(keycodes, keycode, state,
		                   &gb->hwstate, &gb->joypad);
	};

	while (SDL_PollEvent(events)) {
		switch (events->type) {
		case SDL_KEYDOWN:
			if (scancode != SDL_SCANCODE_ESCAPE)
				update_key(State::Down, scancode);
			else
				return false;
			break;
		case SDL_KEYUP:
			update_key(State::Up, scancode);
			break;
		case SDL_JOYBUTTONDOWN:
			update_key(State::Down, jbutton.button);
			break;
		case SDL_JOYBUTTONUP:
			update_key(State::Up, jbutton.button);
			break;
		case SDL_JOYAXISMOTION: {
			State state;
			Uint32 keycode;
			Uint32 codes[2];
			const auto value = events->jaxis.value;
			const auto axis = events->jaxis.axis;

			if (value > -30000 && value < 30000)
				state = State::Up;
			else
				state = State::Down;

			if (axis == 0) {
				codes[0] = SDL_SCANCODE_LEFT;
				codes[1] = SDL_SCANCODE_RIGHT;
			} else if (axis == 1) {
				codes[0] = SDL_SCANCODE_UP;
				codes[1] = SDL_SCANCODE_DOWN;
			}

			if (state == State::Down) {
				keycode = codes[value > 0 ? 1 : 0];
				update_key(state, keycode);
			} else {
				update_key(state, codes[0]);
				update_key(state, codes[1]);
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
	const auto lcdc = gb->gpu.lcdc;
	if (lcdc.lcd_on) {
		int pitch;
		void* pixels;
		if (SDL_LockTexture(texture, nullptr, &pixels, &pitch) == 0) {
			memcpy(pixels, gbx::Gpu::screen,
			        sizeof(uint32_t) * 144 * 160);
			SDL_UnlockTexture(texture);
			SDL_RenderCopy(renderer, texture, nullptr, nullptr);
		} else {
			fprintf(stderr, "failed to lock texture: %s\n",
			        SDL_GetError());
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

	if (joystick == nullptr) {
		fprintf(stderr, "error: %s\n", SDL_GetError());
		return false;
	}

	const char* const keywords[]{
		"A", "B", "SELECT", "START"
	};

	SDL_Event ev;
	Uint32 keycode = 255;

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

		keycodes[i] = keycode;
		printf("%d\n", keycode);
	}

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
		WinWidth * 2, WinHeight * 2, SDL_WINDOW_RESIZABLE);

	if (window == nullptr) {
		fprintf(stderr, "failed to create SDL_Window: %s\n",
			SDL_GetError());
		goto free_sdl;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	if (renderer == nullptr) {
		fprintf(stderr, "failed to create SDL_Renderer: %s\n",
		        SDL_GetError());
		goto free_window;
	}


	texture = SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		WinWidth, WinHeight);

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


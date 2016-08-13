#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <Utix/Assert.h>
#include <Utix/ScopeExit.h>
#include "Gameboy.hpp"



namespace {

static bool InitSDL();
static void QuitSDL();
static void UpdateKey(gbx::KeyState state, SDL_Scancode keycode, gbx::Keys* keys);
void RenderGraphics(const gbx::GPU& gpu, const gbx::Memory& memory);


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

	SDL_Event event;
	Uint32 last_ticks = 0;
	size_t itr = 0;

	while (true) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
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
		if (gameboy->gpu.GetMode() != gbx::GPU::Mode::VBLANK)
			RenderGraphics(gameboy->gpu, gameboy->memory);
		SDL_Delay(15);

		const auto ticks = SDL_GetTicks();
		if (ticks > (last_ticks + 1000)) {
			printf("ITR: %zu\n", itr);
			itr = 0;
			last_ticks = ticks;
		}

		++itr;
	}


SDL_QUIT_EVENT:


	return EXIT_SUCCESS;
}









namespace {

// TODO: endian checks

enum Color : Uint32
{
	BLACK = 0x00000000,
	WHITE = 0xFFFFFF00,
	LIGHT_GREY = 0x90909000,
	DARK_GREY = 0x55555500,
};


struct ColorNumber
{
	uint8_t upper;
	uint8_t down;
};

struct TileMap
{
	uint8_t data[32][32];
};

struct SpriteAttr
{
	uint8_t ypos;
	uint8_t xpos;
	uint8_t id;
	uint8_t flags;
};


struct Tile
{
	uint8_t data[8][2];
};

struct Sprite
{
	uint8_t data[8][2];
};



constexpr const int WIN_WIDTH = 160;
constexpr const int WIN_HEIGHT = 144;

static SDL_Window* window;
static SDL_Texture* texture;
static SDL_Renderer* renderer;
static Uint32* gfx_buffer;

static void DrawOBJ(const gbx::GPU& gpu, const gbx::Memory& memory);
static void DrawTileMap(const Tile* tiles, const TileMap* map, uint8_t pallete, bool unsigned_map);
static void DrawTile(const Tile& tile, uint8_t pallete, uint8_t x, uint8_t y);
static void DrawSprite(const Sprite& sprite, const SpriteAttr& attr, const gbx::GPU& gpu);
static Color SolvePallete(ColorNumber color_number, uint8_t bit, uint8_t pallete);
inline ColorNumber SolveColorNumber(const Tile& tile, uint8_t row);
inline ColorNumber SolveColorNumber(const Sprite& tile, uint8_t row);
inline Color CheckPixel(uint8_t x, uint8_t y);
inline void DrawPixel(Color pixel, uint8_t x, uint8_t y);





void RenderGraphics(const gbx::GPU& gpu, const gbx::Memory& memory)
{
	using gbx::GPU;

	const uint8_t lcdc = gpu.lcdc;

	SDL_RenderClear(renderer);


	int pitch;
	if (SDL_LockTexture(texture, nullptr, (void**)&gfx_buffer, &pitch) != 0) {
		fprintf(stderr, "failed to lock texture: %s\n", SDL_GetError());
		return;
	}

	const uint8_t bgp = gpu.bgp;
	const bool unsigned_tiles = (lcdc & GPU::BG_WIN_TILE_DATA_SELECT) != 0;
	auto tile_data = unsigned_tiles ? reinterpret_cast<const Tile*>(memory.vram)
	                                : reinterpret_cast<const Tile*>(memory.vram + 0x1000);


	if (lcdc & GPU::BG_ON_OFF) {
		const bool tile_map_select = (lcdc & GPU::BG_TILE_MAP_SELECT) != 0;
		auto tile_map = tile_map_select ? reinterpret_cast<const TileMap*>(memory.vram + 0x1C00)
		                                : reinterpret_cast<const TileMap*>(memory.vram + 0x1800);
		DrawTileMap(tile_data, tile_map, bgp, unsigned_tiles);
	}

	if (lcdc & GPU::WIN_ON_OFF) {
		const bool tile_map_select = (lcdc & GPU::WIN_TILE_MAP_SELECT) != 0;
		auto tile_map = tile_map_select ? reinterpret_cast<const TileMap*>(memory.vram + 0x1C00)
		                                : reinterpret_cast<const TileMap*>(memory.vram + 0x1800);
		DrawTileMap(tile_data, tile_map, bgp, unsigned_tiles);
	}

	if (lcdc & GPU::OBJ_ON_OFF)
		DrawOBJ(gpu, memory);


	SDL_UnlockTexture(texture);
	SDL_RenderCopy(renderer, texture, nullptr, nullptr);
	SDL_RenderPresent(renderer);
}


static void DrawOBJ(const gbx::GPU& gpu, const gbx::Memory& memory)
{
	auto sprite_attr = reinterpret_cast<const SpriteAttr*>(memory.oam);
	auto sprites = reinterpret_cast<const Sprite*>(memory.vram);

	for (uint8_t i = 0; i < 40; ++i) {
		const auto id = sprite_attr[i].id;
		DrawSprite(sprites[id], sprite_attr[i], gpu);
	}
}




static void DrawTileMap(const Tile* tiles, const TileMap* map, uint8_t pallete, bool unsigned_map)
{
	if (unsigned_map) {
		for (uint8_t y = 0; y < 18; ++y) {
			for (uint8_t x = 0; x < 20; ++x) {
				const auto tile_id = map->data[y][x];
				DrawTile(tiles[tile_id], pallete, x * 8, y * 8);
			}
		}
	}
	else {
		for (uint8_t y = 0; y < 18; ++y) {
			for (uint8_t x = 0; x < 20; ++x) {
				const auto tile_id = static_cast<int8_t>(map->data[y][x]);
				DrawTile(tiles[tile_id], pallete, x * 8, y * 8);
			}
		}
	}
}







static void DrawTile(const Tile& tile, uint8_t pallete, uint8_t x, uint8_t y)
{
	for (uint8_t row = 0; row < 8; ++row) {
		const auto color_number = SolveColorNumber(tile, row);
		for (uint8_t bit = 0; bit < 8; ++bit) {
			const auto pixel = SolvePallete(color_number, bit, pallete);
			DrawPixel(pixel, x + bit, y + row);
		}
	}
}




// need to implement XFLIP and YFLIP
static void DrawSprite(const Sprite& sprite, const SpriteAttr& attr, const gbx::GPU& gpu)
{

	const uint8_t xpos = attr.xpos - 8;
	const uint8_t ypos = attr.ypos - 16;
	
	if (xpos >= WIN_WIDTH && ypos >= WIN_HEIGHT)
		return;

	
	const uint8_t attrflags = attr.flags;
	const uint8_t pallete = 0xfc & ( gbx::TestBit(4, attrflags) ? gpu.obp1 : gpu.obp0 );
	
	const bool priority = gbx::TestBit(7, attrflags) != 0;

	ASSERT_MSG(!gbx::TestBit(6, attr.flags), "NEED YFLIP");
	ASSERT_MSG(!gbx::TestBit(5, attr.flags), "NEED XFLIP");

	for (uint8_t row = 0; row < 8; ++row) {
		const uint8_t abs_ypos = ypos + row;
		
		if (abs_ypos >= WIN_HEIGHT)
			break;

		const auto color_number = SolveColorNumber(sprite, row);
		
		for (uint8_t bit = 0; bit < 8; ++bit) {
			const uint8_t abs_xpos = xpos + bit;
			if (abs_xpos >= WIN_WIDTH)
				break;
			else if (priority && CheckPixel(abs_xpos, abs_ypos) != WHITE)
					continue;

			const auto pixel = SolvePallete(color_number, bit, pallete);
			if (pixel != WHITE)
				DrawPixel(pixel, abs_xpos, abs_ypos);
		}
	}
}







static Color SolvePallete(const ColorNumber color_number, uint8_t bit, uint8_t pallete)
{
	const auto get_color = [=](uint8_t pallete_value) {
		switch (pallete_value) {
		case 0x00: return WHITE;
		case 0x01: return LIGHT_GREY;
		case 0x02: return DARK_GREY;
		default: return BLACK;
		};
	};

	const uint8_t upperbit = (color_number.upper & (0x80 >> bit)) ? 1 : 0;
	const uint8_t downbit = (color_number.down & (0x80 >> bit)) ? 1 : 0;
	const uint8_t value = (upperbit << 1) | downbit;

	switch (value) {
	case 0x00: return get_color(pallete & 0x03);
	case 0x01: return get_color((pallete & 0x0C) >> 2);
	case 0x02: return get_color((pallete & 0x30) >> 4);
	default: return get_color((pallete & 0xC0) >> 6);
	}
}





inline ColorNumber SolveColorNumber(const Tile& tile, uint8_t row)
{
	return { tile.data[row][1], tile.data[row][0] };
}

inline ColorNumber SolveColorNumber(const Sprite& sprite, uint8_t row)
{
	return { sprite.data[row][1],  sprite.data[row][0] };
}


inline Color CheckPixel(uint8_t x, uint8_t y)
{
	return static_cast<Color>(gfx_buffer[(y * WIN_WIDTH) + x]);
}



inline void DrawPixel(Color pixel, uint8_t x, uint8_t y)
{
	gfx_buffer[(y * WIN_WIDTH) + x] = pixel;
}




static void UpdateKey(gbx::KeyState state, SDL_Scancode keycode, gbx::Keys* keys)
{
	switch (keycode) {
	case SDL_SCANCODE_Z: keys->pad.bit.a = state; break;
	case SDL_SCANCODE_X: keys->pad.bit.b = state; break;
	case SDL_SCANCODE_C: keys->pad.bit.select = state; break;
	case SDL_SCANCODE_V: keys->pad.bit.start = state; break;
	case SDL_SCANCODE_RIGHT: keys->pad.bit.right = state; break;
	case SDL_SCANCODE_LEFT: keys->pad.bit.left = state; break;
	case SDL_SCANCODE_UP: keys->pad.bit.up = state; break;
	case SDL_SCANCODE_DOWN: keys->pad.bit.down = state; break;
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

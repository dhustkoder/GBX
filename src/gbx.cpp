#include <stdio.h>
#include <stdlib.h>
#include "video.hpp"
#include "input.hpp"
#include "gameboy.hpp"

namespace gbx {


int gbx_main(int argc, char** argv)
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s [rom]\n", argv[0]);
		return EXIT_FAILURE;
	}

	Gameboy* const gb = create_gameboy(argv[1]);

	while (process_inputs(gb)) {
		run_for(70224, gb);
		render_graphics((uint32_t*)gb->ppu.screen, sizeof(gb->ppu.screen));
	}

	destroy_gameboy(gb);
	return EXIT_SUCCESS;
}



} // namespace gbx


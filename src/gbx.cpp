#include <stdio.h>
#include <stdlib.h>
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
	if (gb == nullptr)
		return EXIT_FAILURE;

	while (process_inputs(gb))
		run_for(70224, gb);

	destroy_gameboy(gb);
	return EXIT_SUCCESS;
}



} // namespace gbx


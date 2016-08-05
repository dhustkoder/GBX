#include <stdio.h>
#include <stdlib.h>
#include <Utix/Assert.h>
#include <Utix/ScopeExit.h>
#include "Gameboy.hpp"




int main(int argc, char** argv)
{
	if(argc < 2) {
		fprintf(stderr, "usage: %s <rom>\n", argv[0]);
		return EXIT_FAILURE;
	}

	gbx::Gameboy* gameboy = gbx::create_gameboy();
	if(!gameboy)
		return EXIT_FAILURE;
	
	const auto gameboy_guard = utix::MakeScopeExit([=] { 
		gbx::destroy_gameboy(gameboy); 
	});

	if(!gameboy->LoadRom(argv[1]))
		return EXIT_FAILURE;


	while(true) {
		gameboy->Step();
		gameboy->UpdateGPU();
		gameboy->UpdateInterrupts();
	}

	return EXIT_SUCCESS;
}
















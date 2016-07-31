#include <stdio.h>
#include <stdlib.h>
#include "Cartridge.hpp"









int main(int argc, char** argv)
{
	if(argc < 2) {
		fprintf(stderr, "usage: %s <rom>\n", argv[0]);
		return EXIT_FAILURE;
	}

	gbx::CartridgeInfo cinfo;

	if(!gbx::FillCartridgeInfo(argv[1], &cinfo))
		return EXIT_FAILURE;
	

	printf("CARTRIDGE INFO\n"    \
	       "INTERNAL NAME: %s\n" \
	       "TYPE: %u\n"          \
	       "SYSTEM: %u\n"        \
	       "SIZE: %zu\n",
	       cinfo.internal_name, (unsigned)cinfo.type, 
	       (unsigned)cinfo.system, cinfo.size);


	return EXIT_SUCCESS;
}
















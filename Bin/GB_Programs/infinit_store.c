


#define ADDR(x) ((unsigned char*)x)

void main(void) {

	
	volatile unsigned char* const ptr = ADDR(0x3031);
//	volatile unsigned char* const ptr2 = ADDR(0x7117);

	while(1) {
		*ptr = 0x66;
//		*ptr2 = 0x99;
	}

}

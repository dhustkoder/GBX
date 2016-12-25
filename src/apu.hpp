#ifndef GBX_APU_HPP_
#define GBX_APU_HPP_
#include <stdint.h>

namespace gbx {



struct SoundChannel1 {
	union {
		uint8_t value;
		struct {
			uint8_t shift  : 3;
			uint8_t volume : 1;
			uint8_t time   : 3;
		};
	} nr10;

	union {
		uint8_t value;
		struct {
			uint8_t length  : 6;
			uint8_t pattern : 2;
		};
	} nr11;

	union {
		uint8_t value;
		struct {
			uint8_t number    : 3;
			uint8_t direction : 1;
			uint8_t volume    : 4;
		};
	} nr12;

	union {
		uint8_t value;
		uint8_t freqlower;
	} nr13;

	union {
		uint8_t value;
		struct {
			uint8_t freqhigher : 3;
			uint8_t dummy      : 3;
			uint8_t counter    : 1;
			uint8_t initial    : 1;
		};
	} nr14;
};


struct SoundChannel2 {
	union {
		uint8_t value;
		struct {
			uint8_t length  : 6;
			uint8_t pattern : 2;
		};
	} nr21;

	union {
		uint8_t value;
		struct {
			uint8_t number    : 3;
			uint8_t direction : 1;
			uint8_t volume    : 4;
		};
	} nr22;


	union {
		uint8_t value;
		uint8_t freqlower;
	} nr23;

	union {
		uint8_t value;
		struct {
			uint8_t freqhigher : 3;
			uint8_t dummy      : 3;
			uint8_t counter    : 1;
			uint8_t initial    : 1;
		};
	} nr24;

};

struct SoundChannel3 {
	union {
		uint8_t value;
		struct {
			uint8_t dummy : 7;
			uint8_t off   : 1;
		};
	} nr30;

	union {
		uint8_t value;
		uint8_t length;
	} nr31;

	union {
		uint8_t value;
		struct {
			uint8_t dummy : 5;
			uint8_t level : 2;
		};
	} nr32;

	union {
		uint8_t value;
		uint8_t freqlower;
	} nr33;

	union {
		uint8_t value;
		struct {
			uint8_t freqhigher : 3;
			uint8_t dummy      : 3;
			uint8_t counter    : 1;
			uint8_t initial    : 1;
		};
	} nr34;
};



struct SoundChannel4 {
	union {
		uint8_t value;
		struct {
			uint8_t length : 6;
		};
	} nr41;

	union {
		uint8_t value;
		struct {
			uint8_t number    : 3;
			uint8_t direction : 1;
			uint8_t volume    : 4;
		};
	} nr42;

	union {
		uint8_t value;
		struct {
			uint8_t ratio_freq   : 3;
			uint8_t counter      : 1;
			uint8_t shift_freq   : 4;
		};
	} nr43;

	union {
		uint8_t value;
		struct {
			uint8_t dummy   : 6;
			uint8_t counter : 1;
			uint8_t initial : 1;
		};
	} nr44;
};


struct SoundControl {
	union {
		uint8_t value;
		struct {
			uint8_t so1_volume : 3;
			uint8_t so1_vin    : 1;
			uint8_t so2_volume : 3;
			uint8_t so2_vin    : 1;
		};
	} nr50;


	union {
		uint8_t value;
		struct {
			uint8_t so1_out1 : 1;
			uint8_t so1_out2 : 1;
			uint8_t so1_out3 : 1;
			uint8_t so1_out4 : 1;
			uint8_t so2_out1 : 1;
			uint8_t so2_out2 : 1;
			uint8_t so2_out3 : 1;
			uint8_t so2_out4 : 1;
		};
	} nr51;


	union {
		uint8_t value;
		struct {
			uint8_t sound1_on : 1;
			uint8_t sound2_on : 1;
			uint8_t sound3_on : 1;
			uint8_t sound4_on : 1;
			uint8_t dummy     : 3;
			uint8_t all       : 1;
		};
	} nr52;
};

struct Apu {
	SoundChannel1 ch1;
	SoundChannel2 ch2;
	SoundChannel3 ch3;
	SoundChannel4 ch4;
	SoundControl ctl;
	uint8_t wave_pattern_ram[16];
};



} // namespace gbx
#endif

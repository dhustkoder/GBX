#ifndef GBX_APU_HPP_
#define GBX_APU_HPP_
#include <stdint.h>
#include <string.h>

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

	int16_t timer;
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

	int16_t timer;
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

	int16_t timer;
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

	int16_t timer;
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
			uint8_t ch1_on : 1;
			uint8_t ch2_on : 1;
			uint8_t ch3_on : 1;
			uint8_t ch4_on : 1;
			uint8_t dummy  : 3;
			uint8_t master : 1;
		};
	} nr52;
};

struct Apu {
	SoundChannel1 ch1;
	SoundChannel2 ch2;
	SoundChannel3 ch3;
	SoundChannel4 ch4;
	SoundControl ctl;
	uint8_t wave_ram[16];
};


inline void write_nr11(const uint8_t value, Apu* const apu)
{
	auto& ch = apu->ch1;
	ch.nr11.value = value;
	const auto length = ch.nr11.length;
	ch.nr11.length = length != 0 ? 64 - length : 63;
	apu->ctl.nr52.ch1_on = 1;
}

inline void write_nr21(const uint8_t value, Apu* const apu)
{
	auto& ch = apu->ch2;
	ch.nr21.value = value;
	const auto length = ch.nr21.length;
	ch.nr21.length = length != 0 ? 64 - length : 63;
	apu->ctl.nr52.ch2_on = 1;
}

inline void write_nr31(const uint8_t value, Apu* const apu)
{
	auto& ch = apu->ch3;
	ch.nr31.value = value;
	const auto length = ch.nr31.length;
	ch.nr31.length = 
	  length != 0 ? static_cast<uint8_t>(256 - length) : 255;
	apu->ctl.nr52.ch3_on = 1;
}

inline void write_nr41(const uint8_t value, Apu* const apu)
{
	auto& ch = apu->ch4;
	ch.nr41.value = value;
	const auto length = ch.nr41.length;
	ch.nr41.length = length != 0 ? 64 - length : 63;
	apu->ctl.nr52.ch4_on = 1;
}


} // namespace gbx
#endif

#ifndef GBX_APU_HPP_
#define GBX_APU_HPP_
#include <string.h>
#include "common.hpp"
#include "cpu.hpp"

namespace gbx {

constexpr const int_fast32_t kApuFrameCntTicks = kCpuFreq / 512;

struct Apu {

	struct Square {
		int16_t freq_load;
		int16_t freq_cnt;
		int16_t len_cnt;
		int8_t env_cnt;
		int8_t duty_pos;
		int8_t volume;
		uint8_t out;
		bool trigger;
		bool len_enabled;
		bool enabled;

		union {
			uint8_t reg1raw;
			struct {
				const uint8_t len_load  : 6;
				const uint8_t duty_mode : 2;
			};
		};

		union {
			uint8_t reg2raw;
			struct {
				const uint8_t env_period_load : 3;
				const uint8_t env_add         : 1;
				const uint8_t volume_load     : 4;
			};
		};
	};

	struct Square1 : Square {
		int16_t sweep_period_cnt;
		int16_t freq_shadow;
		bool sweep_enabled;

		union {
			uint8_t reg0raw;
			struct {
				const uint8_t sweep_shift        : 3;
				const uint8_t sweep_negate       : 1;
				const uint8_t sweep_period_load  : 3;
			};
		};

	} square1;

	Square square2;

	union {
		uint8_t nr51raw;
		struct {
			const uint8_t s1t1 : 1;
			const uint8_t s2t1 : 1;
			const uint8_t s3t1 : 1;
			const uint8_t s4t1 : 1;
			const uint8_t s1t2 : 1;
			const uint8_t s2t2 : 1;
			const uint8_t s3t2 : 1;
			const uint8_t s4t2 : 1;
		};
	};


	int16_t frame_cnt;
	int8_t frame_step;
	bool power;
};



extern void update_apu(int16_t cycles, Apu* apu);


inline void tick_length(Apu* const apu)
{
	const auto tick_square_len = [](Apu::Square* const s) {
		if (s->len_enabled && s->len_cnt > 0 && --s->len_cnt == 0)
			s->enabled = false;
	};

	tick_square_len(&apu->square1);
	tick_square_len(&apu->square2);
}


inline uint16_t apu_eval_sweep_freq(Apu* const apu)
{
	Apu::Square1& s = apu->square1;
	uint16_t freq = s.freq_shadow >> s.sweep_shift;
	if (s.sweep_negate)
		freq = s.freq_shadow - freq;
	else
		freq = s.freq_shadow + freq;

	if (freq > 0x7FF)
		s.enabled = false;

	return freq;
}

inline uint8_t read_apu_register(const Apu& apu, const uint16_t addr)
{
	const auto rsquare_reg4 = [](const Apu::Square& s) -> uint8_t {
		return s.len_enabled<<6;
	};

	switch (addr) {
	case 0xFF10: return apu.square1.reg0raw;
	case 0xFF11: return apu.square1.reg1raw;
	case 0xFF12: return apu.square1.reg2raw;
	case 0xFF14: return rsquare_reg4(apu.square1);
	case 0xFF16: return apu.square2.reg1raw;
	case 0xFF17: return apu.square2.reg2raw;
	case 0xFF19: return rsquare_reg4(apu.square2);
	case 0xFF25: return apu.nr51raw;
	case 0xFF26:
		return (apu.power<<7)                 |
		       ((apu.square2.len_cnt > 0)<<1) |
		       (apu.square1.len_cnt > 0);
	}

	return 0;
}

inline void write_apu_register(const uint16_t addr, const uint8_t val, Apu* const apu)
{
	const auto wsquare_reg0 = [&](Apu::Square1* const s) {
		s->reg0raw = val;
	};

	const auto wsquare_reg1 = [&](Apu::Square* const s) {
		s->reg1raw = val;
		s->len_cnt = 64 - s->len_load;
	};

	const auto wsquare_reg2 = [&](Apu::Square* const s) {
		s->reg2raw = val;
	};

	const auto wsquare_reg3 = [&](Apu::Square* const s) {
		s->freq_load = (s->freq_load&0x0700)|val;
	};

	const auto wsquare_reg4 = [&](const int ch) {
		Apu::Square& s = ch == 1 ? apu->square1 : apu->square2;

		s.freq_load = (s.freq_load&0x00FF)|((val&0x07)<<8);
		s.len_enabled = (val&0x40) != 0;
		s.trigger = (val&0x80) != 0;

		if (s.trigger) {
			s.enabled = true;

			if (s.len_cnt == 0)
				s.len_cnt = 64;

			s.freq_cnt = (2048 - s.freq_load) * 4;
			s.volume = s.volume_load;

			if (ch == 1) {
				Apu::Square1& s1 = apu->square1;
				s1.freq_shadow = s1.freq_load;
				s1.sweep_period_cnt = s1.sweep_period_load;

				if (s1.sweep_period_cnt == 0)
					s1.sweep_period_cnt = 8;

				s1.freq_shadow = s1.freq_load;
				s1.sweep_enabled = s1.sweep_period_cnt > 0 || s1.sweep_shift > 0;
				if (s1.sweep_shift > 0)
					apu_eval_sweep_freq(apu);
			}
		}
	};

	if (!apu->power && addr != 0xFF26)
		return;

	switch (addr) {
	case 0xFF10: wsquare_reg0(&apu->square1); break;
	case 0xFF11: wsquare_reg1(&apu->square1); break;
	case 0xFF12: wsquare_reg2(&apu->square1); break;
	case 0xFF13: wsquare_reg3(&apu->square1); break;
	case 0xFF14: wsquare_reg4(1); break;
	case 0xFF16: wsquare_reg1(&apu->square2); break;
	case 0xFF17: wsquare_reg2(&apu->square2); break;
	case 0xFF18: wsquare_reg3(&apu->square2); break;
	case 0xFF19: wsquare_reg4(2); break;
	case 0xFF25: 
		apu->nr51raw = val;
		break;
	case 0xFF26:
		apu->power = (val&0x80) != 0;
		if (!apu->power) {
			memset(apu, 0, sizeof(*apu));
			apu->frame_cnt = kApuFrameCntTicks;
		}
		break;
	}
}



} // namespace gbx
#endif

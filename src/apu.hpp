#ifndef GBX_APU_HPP_
#define GBX_APU_HPP_
#include <string.h>
#include "common.hpp"
#include "cpu.hpp"

namespace gbx {

constexpr const int_fast32_t kApuFrameCntTicks = kCpuFreq / 512;


constexpr uint8_t kNoiseDivisors[] = {
	8, 16, 32, 48, 64, 80, 96, 112
};


struct Apu {

	struct Square {
		int16_t freq_load;
		int16_t freq_cnt;
		int16_t len_cnt;
		int8_t env_cnt;
		int8_t duty_pos;
		int8_t volume;
		uint8_t out;
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


	struct Wave {
		uint8_t len_load;        // nr31 $FF1B
		int16_t len_cnt;
		int16_t freq_load;       // nr33 low nr34 high $FF1F $FF1E
		int16_t freq_cnt;
		uint8_t output_level;    // nr32 $FF1C
		uint8_t volume;
		uint8_t out;
		uint8_t pos;
		bool enabled;            // nr30 $FF1A
		bool len_enabled;        // nr34 $FF1E
		uint8_t pattern_ram[16]; // $FF30 - $FF3F
	} wave;


	struct Noise {
		uint16_t lfsr;
		int16_t freq_cnt;
		int16_t len_cnt;
		int16_t env_cnt;
		int16_t len_load;  // nr41 $FF20
		uint8_t volume;
		uint8_t out;
		bool len_enabled;  // nr44 $FF23
		bool enabled;

		union {
			uint8_t nr42raw; // nr42 $FF21
			struct {
				uint8_t env_period_load : 3;
				uint8_t env_add         : 1;
				uint8_t volume_load     : 4;
			};
		};

		union {
			uint8_t nr43raw; // nr43 $FF22
			struct {
				uint8_t divisor_code     : 3;
				uint8_t width_mode       : 1;
				uint8_t clock_shift      : 4;
			};
		};


	} noise;

	union {
		uint8_t nr50raw;
		struct {
			const uint8_t rvol : 3;
			const uint8_t vren : 1;
			const uint8_t lvol : 3;
			const uint8_t vlen : 1;
		};
	};


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

	Apu::Wave& wave = apu->wave;
	if (wave.len_enabled && wave.len_cnt > 0 && --wave.len_cnt == 0)
		wave.enabled = false;

	Apu::Noise& noise = apu->noise;
	if (noise.len_enabled && noise.len_cnt > 0 && --noise.len_cnt == 0)
		noise.enabled = false;
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
	if (addr >= 0xFF30 && addr <= 0xFF3F)
		return apu.wave.pattern_ram[addr - 0xFF30];

	switch (addr) {
	case 0xFF10: return apu.square1.reg0raw;
	case 0xFF11: return apu.square1.reg1raw;
	case 0xFF12: return apu.square1.reg2raw;
	case 0xFF14: return apu.square1.len_enabled<<6;
	case 0xFF16: return apu.square2.reg1raw;
	case 0xFF17: return apu.square2.reg2raw;
	case 0xFF19: return apu.square2.len_enabled<<6;
	case 0xFF1A: return apu.wave.enabled;
	case 0xFF1B: return apu.wave.len_load;
	case 0xFF1C: return apu.wave.output_level<<5;
	case 0xFF1E: return apu.wave.len_enabled<<6;
	case 0xFF20: return apu.noise.len_load;
	case 0xFF21: return apu.noise.nr42raw;
	case 0xFF22: return apu.noise.nr43raw;
	case 0xFF23: return apu.noise.len_enabled<<6;
	case 0xFF24: return apu.nr50raw;
	case 0xFF25: return apu.nr51raw;
	case 0xFF26:
		return (apu.power<<7)               |
		       ((!!apu.noise.len_cnt)<<3)   |
		       ((!!apu.wave.len_cnt)<<2)    |
		       ((!!apu.square2.len_cnt)<<1) |
		       (!!apu.square1.len_cnt);
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
		s->env_cnt = s->env_period_load;
		s->volume = s->volume_load;
	};

	const auto wsquare_reg3 = [&](Apu::Square* const s) {
		s->freq_load = (s->freq_load&0x0700)|val;
	};

	const auto wsquare_reg4 = [&](const int ch) {
		Apu::Square& s = ch == 1 ? apu->square1 : apu->square2;

		s.freq_load = (s.freq_load&0x00FF)|((val&0x07)<<8);
		s.len_enabled = (val&0x40) != 0;
		const bool trigger = (val&0x80) != 0;

		if (trigger) {
			s.enabled = true;

			if (s.len_cnt == 0)
				s.len_cnt = 64;

			s.freq_cnt = (2048 - s.freq_load) * 4;
			s.volume = s.volume_load;
			s.env_cnt = s.env_period_load;

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


	if (addr >= 0xFF30 && addr <= 0xFF3F) {
		apu->wave.pattern_ram[addr - 0xFF30] = val;
		return;
	}

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
	case 0xFF1A: apu->wave.enabled = (val&0x80) != 0; break;
	case 0xFF1B:
		apu->wave.len_load = val;
		apu->wave.len_cnt = (256 - apu->wave.len_load);
		break;
	case 0xFF1C: apu->wave.output_level = (val&0x60)>>5; break;
	case 0xFF1D: apu->wave.freq_load = (apu->wave.freq_load&0x0700)|val; break;
	case 0xFF1E:
		apu->wave.freq_load = (apu->wave.freq_load&0x00FF)|((val&0x7)<<8);
		apu->wave.len_enabled = (val&0x40) != 0;
		if ((val&0x80) != 0) {
			apu->wave.enabled = true;
			if (apu->wave.len_cnt == 0)
				apu->wave.len_cnt = 256;
			apu->wave.freq_cnt = (2048 - apu->wave.freq_load) * 2;
			apu->wave.pos = 0;
		}
		break;

	case 0xFF20:
		apu->noise.len_load = val&0x3F;
		apu->noise.len_cnt = (64 - apu->noise.len_load);
		break;
	case 0xFF21:
		apu->noise.nr42raw = val;
		apu->noise.env_cnt = apu->noise.env_period_load;
		apu->noise.volume = apu->noise.volume_load;
		break;
	case 0xFF22: apu->noise.nr43raw = val; break;
	case 0xFF23:
		apu->noise.len_enabled = (val&0x40) != 0;
		if (val&0x80) {
			apu->noise.enabled = true;
			if (apu->noise.len_cnt == 0)
				apu->noise.len_cnt = 64;
			apu->noise.freq_cnt = kNoiseDivisors[apu->noise.divisor_code]<<apu->noise.clock_shift;
			apu->noise.env_cnt = apu->noise.env_period_load;
			apu->noise.volume = apu->noise.volume_load;
			apu->noise.lfsr = 0x7FFF;
		}
		break;
	case 0xFF24:
		apu->nr50raw = val;
		break;
	case 0xFF25: 
		apu->nr51raw = val;
		break;
	case 0xFF26:
		apu->power = (val&0x80) != 0;
		if (!apu->power) {
			memset((void*)apu, 0, sizeof(*apu));
			apu->frame_cnt = kApuFrameCntTicks;
		}
		break;
	}
}



} // namespace gbx
#endif

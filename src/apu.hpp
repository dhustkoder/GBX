#ifndef GBX_APU_HPP_
#define GBX_APU_HPP_
#include "common.hpp"
#include "cpu.hpp"

namespace gbx {

constexpr const int_fast32_t kApuFrameCntTicks = kCpuFreq / 512;

struct Apu {

	struct Square {
		int16_t freq;
		int16_t freq_cnt;
		int16_t len_cnt;
		int8_t duty_pos;
		uint8_t out;
		bool trigger;
		bool len_enabled;
		bool enabled;

		union {
			uint8_t val;
			struct {
				uint8_t len    : 6;
				uint8_t duty   : 2;
			};
		} reg1;

		union {
			uint8_t val;
			struct {
				uint8_t period  : 3;
				uint8_t env_add : 1;
				uint8_t vol     : 4;
			};
		} reg2;

	};

	struct Square1 : Square {
		int16_t sweep_cnt;
		int16_t freq_shadow;
		bool sweep_enabled;
		union {
			uint8_t val;
			struct {
				uint8_t shift        : 3;
				uint8_t negate       : 1;
				uint8_t sweep_period : 3;
				uint8_t              : 1;
			};
		} reg0;
	} square1;

	Square square2;

	union {
		uint8_t val;
		struct {
			uint8_t s1t1 : 1;
			uint8_t s2t1 : 1;
			uint8_t s3t1 : 1;
			uint8_t s4t1 : 1;
			uint8_t s1t2 : 1;
			uint8_t s2t2 : 1;
			uint8_t s3t2 : 1;
			uint8_t s4t2 : 1;
		};
	} nr51;


	int16_t frame_cnt;
	int8_t frame_step;
	bool power;
};



extern void update_apu(int16_t cycles, Apu* apu);


inline uint16_t apu_eval_sweep_freq(Apu* const apu)
{
	Apu::Square1& s = apu->square1;
	uint16_t freq = s.freq_shadow >> s.reg0.shift;
	if (s.reg0.negate)
		freq = s.freq_shadow - freq;
	else
		freq = s.freq_shadow + freq;

	if (freq > 2047)
		s.enabled = false;

	return freq;
}


} // namespace gbx
#endif

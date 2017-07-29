#ifndef GBX_APU_HPP_
#define GBX_APU_HPP_
#include "common.hpp"


namespace gbx {



struct Apu {
	struct Square {
		int16_t freq;
		int16_t freq_cnt;
		int8_t duty_pos;
		uint8_t out;
		bool trigger;
		bool len_enabled;

		union {
			uint8_t val;
			struct {
				uint8_t length : 6;
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
};



extern void update_apu(int16_t cycles, Apu* apu);


} // namespace gbx
#endif

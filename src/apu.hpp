#ifndef GBX_APU_HPP_
#define GBX_APU_HPP_

namespace gbx {



struct Apu {
	struct {
		union {
			uint8_t val;
			struct {
				uint8_t shift        : 3;
				uint8_t negate       : 1;
				uint8_t sweep_period : 3;
				uint8_t              : 1;
			};
		} reg0;
	} square[2];
};


} // namespace gbx
#endif

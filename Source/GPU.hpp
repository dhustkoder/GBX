#ifndef GBX_GPU_HPP_
#define GBX_GPU_HPP_
#include "Common.hpp"
namespace gbx {






struct GPU
{
	enum class Mode
	{
		HBLANK = 0x00,
		VBLANK = 0x01,
		OAM = 0x02,
		TRANSFER = 0x03
	};

	Mode GetMode() const;
	void SetMode(const Mode mode);

	uint16_t clock;
	uint8_t control;
	uint8_t status;
	uint8_t scy;
	uint8_t scx;
	uint8_t wy;
	uint8_t wx;
	uint8_t ly;
	uint8_t lyc;
	uint8_t bgp;
};






inline GPU::Mode GPU::GetMode() const 
{
	return static_cast<Mode>(status & 0x03);
}



inline void GPU::SetMode(const Mode mode) 
{
	status &= 0xfc; // clean mode
	status |= static_cast<uint8_t>(mode);
}






}
#endif

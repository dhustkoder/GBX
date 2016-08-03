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


	enum ControlMask : uint8_t
	{
		LCD_ON_OFF = 0x80,
		WIN_TILE_MAP_SELECT = 0x40,
		WIN_ON_OFF = 0x20,
		BG_WIN_TILE_MAP_SELECT = 0x10,
		BG_TILE_MAP_SELECT = 0x08,
		OBJ_SIZE = 0x04,
		OBJ_ON_OFF = 0x02,
		BG_ON_OFF = 0x01
	};

	Mode GetMode() const;
	void SetMode(Mode mode);

	uint16_t clock;
	uint8_t control;
	uint8_t status;
	uint8_t scanline;
};






inline GPU::Mode GPU::GetMode() const {
	return static_cast<Mode>(status & 0x03);
}


inline void GPU::SetMode(const Mode mode) 
{
	status &= 0xfc; // clean mode
	status |= static_cast<uint8_t>(mode);
}










}
#endif
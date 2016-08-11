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

	enum LcdcFlags : uint8_t
	{
		LCD_ON_OFF = 0x80,
		WIN_TILE_MAP_SELECT = 0x40,
		WIN_ON_OFF = 0x20,
		BG_WIN_TILE_DATA_SELECT = 0x10,
		BG_TILE_MAP_SELECT = 0x08,
		OBJ_SIZE = 0x04,
		OBJ_ON_OFF = 0x02,
		BG_ON_OFF = 0x01
	};

	Mode GetMode() const;
	bool BitLCDC(LcdcFlags cflags) const;
	void SetMode(const Mode mode);

	uint16_t clock;
	uint8_t lcdc;
	uint8_t stat;
	uint8_t scy;
	uint8_t scx;
	uint8_t wy;
	uint8_t wx;
	uint8_t ly;
	uint8_t lyc;
	uint8_t bgp;
	uint8_t obp0;
	uint8_t obp1;
};






inline GPU::Mode GPU::GetMode() const 
{
	return static_cast<Mode>(stat & 0x03);
}

inline bool GPU::BitLCDC(LcdcFlags cflags) const
{
	return (lcdc & cflags) != 0;
}

inline void GPU::SetMode(const Mode mode) 
{
	stat &= 0xfc;
	stat |= static_cast<uint8_t>(mode);
}






}
#endif

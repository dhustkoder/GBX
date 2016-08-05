#include "Gameboy.hpp"

namespace gbx {



enum StatusMask : uint8_t
{
	INTERRUPT_ON_COINCIDENCE = 0x40,
	INTERRUPT_ON_OAM = 0x20,
	INTERRUPT_ON_VBLANK = 0x10,
	INTERRUPT_ON_HBLANK = 0x08,
	COINCIDENCE_FLAG = 0x04,
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




void Gameboy::UpdateGPU()
{
	if (!(gpu.control & LCD_ON_OFF)) {
		gpu.clock = 0;
		gpu.ly = 0;
		gpu.SetMode(GPU::Mode::VBLANK);
		return;
	}



	const auto compare_ly = [this]() {
		if (gpu.ly != gpu.lyc) {
			if (gpu.status & COINCIDENCE_FLAG)
				gpu.status &= ~COINCIDENCE_FLAG;
		} else {
			if (!(gpu.status & COINCIDENCE_FLAG))
				gpu.status |= COINCIDENCE_FLAG;
			if (gpu.status & INTERRUPT_ON_COINCIDENCE)
				hwstate.RequestInt(INTERRUPT_LCD_STAT);
		}
	};


	const auto check_stat_interrupt = [this](StatusMask interrupt_on) {
		if (gpu.status & interrupt_on)
			hwstate.RequestInt(INTERRUPT_LCD_STAT);
	};



	switch (gpu.GetMode()) {
	case GPU::Mode::HBLANK:
		if (gpu.clock >= 204) {
			++gpu.ly;

			if (gpu.ly == 143) {
				hwstate.interrupt_flags |= INTERRUPT_VBLANK;
				gpu.SetMode(GPU::Mode::VBLANK);
				check_stat_interrupt(INTERRUPT_ON_VBLANK);
			}
			else {
				gpu.SetMode(GPU::Mode::OAM);
				check_stat_interrupt(INTERRUPT_ON_OAM);
			}

			compare_ly();
			gpu.clock -= 204;
		}

		break;

	case GPU::Mode::VBLANK:
		if (gpu.clock >= 456) {
			++gpu.ly;

			if (gpu.ly > 153) {
				gpu.ly = 0;
				gpu.SetMode(GPU::Mode::OAM);
				check_stat_interrupt(INTERRUPT_ON_OAM);
			}

			compare_ly();
			gpu.clock -= 456;
		}

		break;

	case GPU::Mode::OAM:
		if (gpu.clock >= 80) {
			gpu.SetMode(GPU::Mode::TRANSFER);
			gpu.clock -= 80;
		}

		break;

	case GPU::Mode::TRANSFER:
		if (gpu.clock >= 172) {
			gpu.clock -= 172;
			gpu.SetMode(GPU::Mode::HBLANK);
			check_stat_interrupt(INTERRUPT_ON_HBLANK);
		}

		break;
	}
}




























}

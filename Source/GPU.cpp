#include "Gameboy.hpp"


namespace gbx {



enum StatusMask : uint8_t
{
	INT_ON_COINCIDENCE = 0x40,
	INT_ON_OAM = 0x20,
	INT_ON_VBLANK = 0x10,
	INT_ON_HBLANK = 0x08,
	COINCIDENCE_FLAG = 0x04,
};












void Gameboy::UpdateGPU(const uint8_t cycles)
{
	if (!(gpu.lcdc & GPU::LCD_ON_OFF)) {
		gpu.clock = 0;
		gpu.ly = 0;
		gpu.SetMode(GPU::Mode::HBLANK);
		return;
	}

	const auto compare_ly = [this] {
		if (gpu.ly != gpu.lyc) {
			if (gpu.stat & COINCIDENCE_FLAG)
				gpu.stat &= ~COINCIDENCE_FLAG;
		} else {
			gpu.stat |= COINCIDENCE_FLAG;
			if (gpu.stat & INT_ON_COINCIDENCE)
				hwstate.RequestInt(INT_LCD_STAT);
		}
	};

	const auto set_mode_n_stat = [this](GPU::Mode mode, StatusMask interrupt_on) {
		gpu.SetMode(mode);
		if (gpu.stat & interrupt_on)
			hwstate.RequestInt(INT_LCD_STAT);
	};

	gpu.clock += cycles;

	switch (gpu.GetMode()) {
	case GPU::Mode::HBLANK:
		if (gpu.clock >= 204) {
			++gpu.ly;

			if (gpu.ly != 144) {
				set_mode_n_stat(GPU::Mode::OAM, INT_ON_OAM);
			} else {
				hwstate.interrupt_flags |= INT_VBLANK;
				set_mode_n_stat(GPU::Mode::VBLANK, INT_ON_VBLANK);
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
				set_mode_n_stat(GPU::Mode::OAM, INT_ON_OAM);
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
			set_mode_n_stat(GPU::Mode::HBLANK, INT_ON_HBLANK);
			gpu.clock -= 172;
		}

		break;
	}
}




























}

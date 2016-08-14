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




void Gameboy::UpdateGPU(const uint8_t cycles)
{
	if (!(gpu.lcdc & GPU::LCD_ON_OFF)) {
		gpu.clock = 0;
		gpu.ly = 0;
		gpu.SetMode(GPU::Mode::HBLANK);
		return;
	}

	const auto compare_ly = [this]() {
		if (gpu.ly != gpu.lyc) {
			if (gpu.stat & COINCIDENCE_FLAG)
				gpu.stat &= ~COINCIDENCE_FLAG;
		} else {
			if (!(gpu.stat & COINCIDENCE_FLAG))
				gpu.stat |= COINCIDENCE_FLAG;
			if (gpu.stat & INTERRUPT_ON_COINCIDENCE)
				hwstate.RequestInt(INTERRUPT_LCD_STAT);
		}
	};

	const auto set_mode_and_check_stat = [this](GPU::Mode mode, StatusMask interrupt_on) {
		gpu.SetMode(mode);
		if (gpu.stat & interrupt_on)
			hwstate.RequestInt(INTERRUPT_LCD_STAT);
	};


	gpu.clock += cycles;

	switch (gpu.GetMode()) {
	case GPU::Mode::HBLANK:
		if (gpu.clock >= 204) {
			++gpu.ly;

			if (gpu.ly == 144) {
				hwstate.interrupt_flags |= INTERRUPT_VBLANK;
				set_mode_and_check_stat(GPU::Mode::VBLANK, INTERRUPT_ON_VBLANK);
			} else {
				set_mode_and_check_stat(GPU::Mode::OAM, INTERRUPT_ON_OAM);
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
				set_mode_and_check_stat(GPU::Mode::OAM, INTERRUPT_ON_OAM);
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
			set_mode_and_check_stat(GPU::Mode::HBLANK, INTERRUPT_ON_HBLANK);
			gpu.clock -= 172;
		}

		break;
	}
}




























}

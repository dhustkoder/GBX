#include <climits>
#include "SDL_audio.h"
#include "apu.hpp"

extern SDL_AudioDeviceID audio_device;

namespace gbx {


static double apu_samples[95];
static int16_t sound_buffer[1024];
static int8_t samples_index = 0;
static int16_t sound_buffer_index = 0;



void update_apu(const int16_t cycles, Apu* const apu)
{
	static const uint8_t dutytbl[4][8] = {
		{ 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 0, 0, 0, 0, 1 },
		{ 0, 1, 1, 1, 1, 1, 1, 0 }
	};

	for (int ticks = 0; ticks < cycles; ++ticks) {
		for (int i = 0; i < 2; ++i) {
			if (--apu->square[i].freq_cnt <= 0) {
				apu->square[i].freq_cnt = (2048 - apu->square[i].freq) * 4;
				apu->square[i].duty_pos += 1;
				apu->square[i].duty_pos &= 0x07;
			}
			if (!dutytbl[apu->square[i].reg1.duty][apu->square[i].duty_pos])
				apu->square[i].out = 0;
			else
				apu->square[i].out = apu->square[i].reg2.vol;
		}

		apu_samples[samples_index++] = apu->square[0].out + apu->square[1].out;
		if (samples_index >= 95) {
			samples_index = 0;
			double avg = 0;
			for (int i = 0; i < 95; ++i)
				avg += apu_samples[i];
			avg /= 95;
			sound_buffer[sound_buffer_index++] = avg * 500;
			if (sound_buffer_index >= 1024) {
				sound_buffer_index = 0;
				SDL_QueueAudio(audio_device, (uint8_t*)sound_buffer, sizeof(sound_buffer));
			}
		}
	}
}


} // namespace gbx


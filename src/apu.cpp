#include <climits>
#include "audio.hpp"
#include "apu.hpp"


namespace gbx {

constexpr const int kApuSamplesSize = 95;
constexpr const int kSoundBufferSize = 1024;

static int16_t apu_samples[kApuSamplesSize];
static int16_t sound_buffer[kSoundBufferSize];
static int samples_index = 0;
static int sound_buffer_index = 0;



static void tick_sweep(Apu* const apu)
{
	Apu::Square1& s = apu->square1;
	if (--s.sweep_period_cnt <= 0) {
		s.sweep_period_cnt = s.sweep_period_load;
		if (s.sweep_period_cnt == 0)
			s.sweep_period_cnt = 8;
		if (s.sweep_enabled && s.sweep_period_load > 0) {
			const uint16_t freq = apu_eval_sweep_freq(apu);
			if (freq < 0x800 && s.sweep_shift > 0) {
				s.freq_shadow = freq;
				s.freq_load = freq;
				apu_eval_sweep_freq(apu);
			}
			apu_eval_sweep_freq(apu);
		}
	}
}

static void tick_envelope(Apu* const apu)
{
	const auto tick_square_env = [](Apu::Square* const s) {
		if (--s->env_cnt <= 0) {
			s->env_cnt = s->env_period_load;

			if (s->env_cnt == 0)
				s->env_cnt = 8;
			
			if (s->env_period_load > 0) {
				if (s->env_add && s->volume < 15)
					++s->volume;
				else if (!s->env_add && s->volume > 0)
					--s->volume;
			}
		}
	};

	tick_square_env(&apu->square1);
	tick_square_env(&apu->square2);

	Apu::Noise& noise = apu->noise;

	if (--noise.env_cnt <= 0) {
		noise.env_cnt = noise.env_period_load;

		if (noise.env_cnt == 0)
			noise.env_cnt = 8;

		if (noise.env_period_load > 0) {
			if (noise.env_add && noise.volume < 15)
				++noise.volume;
			else if (!noise.env_add && noise.volume > 0)
				--noise.volume;
		}
	}
}

static void tick_frame_counter(Apu* const apu)
{
	if (--apu->frame_cnt <= 0)  {
		apu->frame_cnt = kApuFrameCntTicks;
		switch (apu->frame_step++) {
		case 0:
			tick_length(apu);
			break;
		case 2:
			tick_length(apu);
			tick_sweep(apu);
			break;
		case 4:
			tick_length(apu);
			break;
		case 6:
			tick_length(apu);
			tick_sweep(apu);
			break;
		case 7:
			tick_envelope(apu);
			apu->frame_step = 0;
			break;
		}
	}
}

static void tick_freq_counters(Apu* const apu)
{
	const auto tick_square_freq_cnt = [](Apu::Square* const s) {
		static const uint8_t dutytbl[4][8] = {
			{ 0, 0, 0, 0, 0, 0, 0, 1 },
			{ 1, 0, 0, 0, 0, 0, 0, 1 },
			{ 1, 0, 0, 0, 0, 1, 1, 1 },
			{ 0, 1, 1, 1, 1, 1, 1, 0 }
		};

		if (--s->freq_cnt <= 0) {
			s->freq_cnt = (2048 - s->freq_load) * 4;
			s->duty_pos += 1;
			s->duty_pos &= 0x07;


			if (!s->enabled || !dutytbl[s->duty_mode][s->duty_pos])
				s->out = 0;
			else
				s->out = s->volume;
		}
	};

	tick_square_freq_cnt(&apu->square1);
	tick_square_freq_cnt(&apu->square2);


	Apu::Wave& wave = apu->wave;

	if (--wave.freq_cnt <= 0) {
		wave.freq_cnt = (2048 - wave.freq_load) * 2;
		wave.pos = (wave.pos + 1) % 32;
		const uint8_t sample = wave.pattern_ram[wave.pos / 2];
		wave.volume = (wave.pos&0x01) ? sample >> 4 : sample&0x0F;

		if (!wave.enabled || wave.output_level == 0) {
			wave.out = 0;
		} else {
			wave.out = wave.volume >> (wave.output_level - 1);
		}
	}


	Apu::Noise& noise = apu->noise;
	if (--noise.freq_cnt <= 0) {
		noise.freq_cnt = kNoiseDivisors[noise.divisor_code]<<noise.clock_shift;
		uint8_t r = (noise.lfsr&0x01) ^ ((noise.lfsr>>1)&0x01);
		noise.lfsr >>= 1;
		noise.lfsr |= r<<14;
		if (noise.width_mode) {
			noise.lfsr &= ~0x40;
			noise.lfsr |= r<<6;
		}


		if (!noise.enabled || (noise.lfsr&0x01))
			noise.out = 0;
		else
			noise.out = noise.volume;
	}
}


void update_apu(const int16_t cycles, Apu* const apu)
{
	if (!apu->power)
		return;

	for (int ticks = 0; ticks < cycles; ++ticks) {
		tick_frame_counter(apu);
		tick_freq_counters(apu);

		apu_samples[samples_index] = 0;

		const auto mix = [](const int16_t out, const uint8_t vol) {
			mix_audio(&apu_samples[samples_index], out,
				  (kAudioMaxVolume * vol)/7);
		};

		if (apu->s1t1)
			mix(apu->square1.out, apu->rvol);
		if (apu->s1t2)	
			mix(apu->square1.out, apu->lvol);
		if (apu->s2t1)
			mix(apu->square2.out, apu->rvol);
		if (apu->s2t2)
			mix(apu->square2.out, apu->lvol);
		if (apu->s3t1)
			mix(apu->wave.out, apu->rvol);
		if (apu->s3t2)
			mix(apu->wave.out, apu->lvol);
		if (apu->s4t1)
			mix(apu->noise.out, apu->rvol);
		if (apu->s4t2)
			mix(apu->noise.out, apu->lvol);


		if (++samples_index >= kApuSamplesSize) {
			samples_index = 0;

			double avg = 0;

			for (int i = 0; i < kApuSamplesSize; ++i)
				avg += apu_samples[i];

			avg /= kApuSamplesSize;
			avg *= 125;

			sound_buffer[sound_buffer_index] = avg;
			if (++sound_buffer_index >= kSoundBufferSize) {
				sound_buffer_index = 0;
				queue_sound_buffer(sound_buffer, sizeof(sound_buffer));
			}
		}
	}
}


} // namespace gbx


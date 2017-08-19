#ifndef GBX_AUDIO_HPP_
#define GBX_AUDIO_HPP_
#include <stdint.h>
#include "SDL.h"
#include "SDL_audio.h"


constexpr const auto kAudioMaxVolume = SDL_MIX_MAXVOLUME;


inline void mix_audio(int16_t* const dest, const int16_t src, const int volume)
{
	*dest += src;
	*dest = (*dest * volume) / kAudioMaxVolume;
}


inline void queue_sound_buffer(const int16_t* const buffer, const uint_fast32_t len)
{
	extern SDL_AudioDeviceID audio_device;

	while (SDL_GetQueuedAudioSize(audio_device) > len)
		SDL_Delay(1);

	if (SDL_QueueAudio(audio_device, (const uint8_t*)buffer, len) != 0)
		fprintf(stderr, "Failed to queue audio: %s\n", SDL_GetError());
}



#endif

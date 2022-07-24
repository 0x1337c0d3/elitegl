/**
 * Elite - The New Kind.
 *
 * SDL/OpenGL Port by Mark Follett 2001-2002
 * email: <mef123@geocities.com>
 **/

#ifdef HAVE_SMPEG

#include "mp3.h"
#include <SDL2_mixer/SDL_mixer.h>
#include "smpeg.h"

static Mp3_Music *current_mp3;
static SDL_AudioSpec audiospec;


Mp3_Music *mp3_load (char *path)
{
//	SDL_AudioSpec spec;
	
	Mp3_Music *mp3;
	mp3 = SMPEG_new(path, NULL, 0);

//	SMPEG_wantedSpec(mp3, &spec);
//	printf("SMPEG Freq: %d Format: %d Channels: %d Samples: %d\n", spec.freq, spec.format, spec.channels, spec.samples);

	SMPEG_actualSpec(mp3, &audiospec);
	SMPEG_enableaudio(mp3, 1);
	return mp3;
}


void mp3_out (void *userdata, Uint8 *stream, int length)
{
	SMPEG_status(current_mp3);
	SMPEG_playAudio(current_mp3, stream, length);
}


void mp3_play (Mp3_Music *mp3)
{
	SMPEG_rewind(mp3);
	SMPEG_loop(mp3, 1);
	SMPEG_play(mp3);
	Mix_HookMusic(mp3_out, NULL);
	current_mp3 = mp3;
}


void mp3_stop (void)
{
	if (current_mp3)
	{
		Mix_HookMusic(NULL,NULL);
		SMPEG_stop(current_mp3);
		current_mp3 = NULL;
	}
}


void mp3_init (int freq, Uint16 format, int channels, int samples)
{
	audiospec.freq     = freq;
	audiospec.format   = format;
	audiospec.channels = channels;
	audiospec.samples  = samples;
}

#endif

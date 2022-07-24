/*
 * Elite - The New Kind.
 *
 * Reverse engineered from the BBC disk version of Elite.
 * Additional material by C.J.Pinder.
 *
 * The original Elite code is (C) I.Bell & D.Braben 1984.
 * This version re-engineered in C by C.J.Pinder 1999-2001.
 *
 * email: <christian@newkind.co.uk>
 *
 *
 */

/**
 * SDL/OpenGL Port by Mark Follett 2001-2002
 * email: <mef123@geocities.com>
 **/

/*
 * sound.c
 */

#include <stdlib.h>
#include "sound.h"
#include "elite.h"
#include "file.h"
#include <SDL2_mixer/SDL_mixer.h>

#ifdef HAVE_SMPEG
#  include "mp3.h"
#endif

#define NUM_SAMPLES 14
#define NUM_SONGS 2

static int sound_on;

union {

#ifdef HAVE_SMPEG
	Mp3_Music *mp3;
#endif

	Mix_Music *midi;
	Mix_Chunk *wav;

} songs[NUM_SONGS];


struct sound_sample
{
 	Mix_Chunk *sample;
	char filename[256];
	int runtime;
	int timeleft;
};

struct sound_sample sample_list[NUM_SAMPLES] =
{
	{NULL, "launch.wav",    32, 0},
	{NULL, "crash.wav",      7, 0},
	{NULL, "dock.wav",      36, 0},
	{NULL, "gameover.wav",  24, 0},
	{NULL, "pulse.wav",      4, 0},
	{NULL, "hitem.wav",		 4, 0},
	{NULL, "explode.wav",	23, 0},
	{NULL, "ecm.wav",		23, 0},
	{NULL, "missile.wav",	25, 0},
	{NULL, "hyper.wav",	    37, 0},
	{NULL, "incom1.wav",	 4, 0},
	{NULL, "incom2.wav",	 5, 0},
	{NULL, "beep.wav",		 2, 0},
	{NULL, "boop.wav",		 7, 0},
};


#ifdef HAVE_SMPEG
Mp3_Music *load_music_mp3 (char *filename)
{
	Mp3_Music *music;
	char *path;
	path = concat_paths(MUSIC_DIRECTORY, filename);
	music = mp3_load(path);
	free(path);
	return music;
}
#endif

Mix_Chunk *load_music_wav (char *filename)
{
	Mix_Chunk *music;
	char *path;
	path = concat_paths(MUSIC_DIRECTORY, filename);
	music = Mix_LoadWAV(path);
	free(path);
	return music;
}

Mix_Music *load_music_midi (char *filename)
{
	Mix_Music *music;
	char *path;
	path = concat_paths(MUSIC_DIRECTORY, filename);
	music = Mix_LoadMUS(path);
	free(path);
	return music;
}

Mix_Chunk *load_sound (char *filename)
{
	Mix_Chunk *sound;
	char *path;
	path = concat_paths(SOUND_DIRECTORY, filename);
	sound = Mix_LoadWAV(path);
	free(path);
	return sound;
}


#ifdef HAVE_SMPEG
void music_startup_mp3 (void)
{
	int frequency;
	Uint16 format;
	int channels;

	Mix_QuerySpec(&frequency, &format, &channels);
//	printf("Freq: %d Format: %d Channels: %d\n", frequency, format, channels);
	mp3_init(frequency, format, channels, sound_buffersize);

	Mix_VolumeMusic(128);

	songs[SND_ELITE_THEME].mp3 = load_music_mp3("theme.mp3");
	songs[SND_BLUE_DANUBE].mp3 = load_music_mp3("danube.mp3");
}
#endif

void music_startup_midi (void)
{
	Mix_VolumeMusic(64);

	songs[SND_ELITE_THEME].midi = load_music_midi("theme.mid");
	songs[SND_BLUE_DANUBE].midi = load_music_midi("danube.mid");
}

void music_startup_wav (void)
{
	Mix_VolumeMusic(128);
	Mix_ReserveChannels(1);

	songs[SND_ELITE_THEME].wav = load_music_wav("theme.wav");
	songs[SND_BLUE_DANUBE].wav = load_music_wav("danube.wav");
}

void snd_sound_startup (void)
{
	int i;
    int frequency;
    Uint16 format;
    int channels;

 	sound_on = 1;

    switch (sound_freq)
    {
        case SND_FREQ_11KHZ:
            frequency = 11025;
            break;

        case SND_FREQ_22KHZ:
            frequency = 22050;
            break;

        case SND_FREQ_44KHZ:
            frequency = 44100;
            break;

        default:
            frequency = MIX_DEFAULT_FREQUENCY;
            break;
    }

    switch (sound_format)
    {
        case SND_FORMAT_8BIT:
            format = AUDIO_U8;
            break;

        case SND_FORMAT_16BIT:
            format = AUDIO_S16SYS;
            break;

        default:
            format = MIX_DEFAULT_FORMAT;
            break;
    }

    switch (sound_channels)
    {
        case SND_MONO:
            channels = 1;
            break;

        case SND_STEREO:
            channels = 2;
            break;

        default:
            channels = MIX_DEFAULT_CHANNELS;
            break;
    }

	if (Mix_OpenAudio(frequency, format, channels, sound_buffersize) < 0) {
		printf("Couldn't open audio: %s", SDL_GetError());
		sound_on = 0;
		return;
	}

	Mix_Volume(-1, 128);

	switch (music_mode)
	{

#ifdef HAVE_SMPEG
		case MUSIC_MP3:
			music_startup_mp3();
			break;
#endif

		case MUSIC_MIDI:
			music_startup_midi();
			break;

		case MUSIC_WAV:
			music_startup_wav();
			break;
	}

	/* Load the sound samples... */

	for (i = 0; i < NUM_SAMPLES; i++)
	{
		sample_list[i].sample = load_sound(sample_list[i].filename);
	}
}

void snd_sound_shutdown (void)
{
	int i;

	if (!sound_on)
		return;

	for (i = 0; i < NUM_SAMPLES; i++)
	{
		if (sample_list[i].sample != NULL)
		{
			Mix_FreeChunk(sample_list[i].sample);
			sample_list[i].sample = NULL;
		}
	}
}


void snd_play_sample (int sample_no)
{
	if (!sound_on)
		return;

	if (sample_list[sample_no].timeleft != 0)
		return;

	sample_list[sample_no].timeleft = sample_list[sample_no].runtime;
		
	Mix_PlayChannel(-1,sample_list[sample_no].sample,0);
}


void snd_update_sound (void)
{
	int i;

	if (!sound_on)
		return;

	for (i = 0; i < NUM_SAMPLES; i++)
	{
		if (sample_list[i].timeleft > 0)
			sample_list[i].timeleft--;
	}
}


void snd_play_midi (int midi_no, int repeat)
{
	if (!sound_on)
		return;

	switch (music_mode)
	{

#ifdef HAVE_SMPEG
		case MUSIC_MP3:
			mp3_play(songs[midi_no].mp3);
			break;
#endif

		case MUSIC_MIDI:
			Mix_PlayMusic(songs[midi_no].midi, -1);
			break;

		case MUSIC_WAV:
			Mix_PlayChannel(0, songs[midi_no].wav, -1);
			break;
	}
}

void snd_stop_midi (void)
{
	if (!sound_on)
		return;

	switch (music_mode)
	{

#ifdef HAVE_SMPEG
		case MUSIC_MP3:
			mp3_stop();
			break;
#endif

		case MUSIC_MIDI:
			Mix_HaltMusic();
			break;

		case MUSIC_WAV:
			Mix_HaltChannel(0);
			break;
	}
}

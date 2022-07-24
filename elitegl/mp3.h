/**
 * Elite - The New Kind.
 *
 * SDL/OpenGL Port by Mark Follett 2001-2002
 * email: <mef123@geocities.com>
 **/

#ifndef MP3_H
#define MP3_H

#include <SDL2_mixer/SDL_mixer.h>

typedef void Mp3_Music;

Mp3_Music *mp3_load (char *path);
void       mp3_play (Mp3_Music *mp3);
void       mp3_stop (void);
void       mp3_init (int freq, Uint16 format, int channels, int samples);



#endif /* MP3_H */

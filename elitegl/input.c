/**
 * Elite - The New Kind.
 *
 * SDL/OpenGL Port by Mark Follett 2001-2002
 * email: <mef123@geocities.com>
 **/

#include <string.h>
 
#include "input.h"
#include "keysyms.h"
#include "main.h"
#include "gfx.h"

#include <SDL2/SDL.h>

#define JOY_UP       0
#define JOY_DOWN     1
#define JOY_LEFT     2
#define JOY_RIGHT    3
#define JOY_BUTTON0  4
#define JOY_BUTTON1  5
#define JOY_BUTTON2  6
#define JOY_LAST     7

int joy[JOY_LAST];
char key[KEY_LAST];
int kbd_table[KEY_LAST];
int kbd[KBD_LAST];


void kbd_init_translation_table (void)
{
    int i;

    for (i = 0; i < KEY_LAST; i++)
    {
        kbd_table[i] = KBD_NONE;
    }

    kbd_table[SDL_SCANCODE_1]  = KBD_F1;
    kbd_table[SDL_SCANCODE_2]  = KBD_F2;
    kbd_table[SDL_SCANCODE_3]  = KBD_F3;
    kbd_table[SDL_SCANCODE_4]  = KBD_F4;
    kbd_table[SDL_SCANCODE_5]  = KBD_F5;
    kbd_table[SDL_SCANCODE_6]  = KBD_F6;
    kbd_table[SDL_SCANCODE_7]  = KBD_F7;
    kbd_table[SDL_SCANCODE_8]  = KBD_F8;
    kbd_table[SDL_SCANCODE_9]  = KBD_F9;
    kbd_table[SDL_SCANCODE_0] = KBD_F10;
    kbd_table[SDL_SCANCODE_MINUS] = KBD_F11;
    kbd_table[SDL_SCANCODE_EQUALS] = KBD_F12;

    kbd_table[SDL_SCANCODE_Y] = KBD_Y;
    kbd_table[SDL_SCANCODE_N] = KBD_N;

    kbd_table[SDL_SCANCODE_A]      = KBD_FIRE;
    kbd_table[SDL_SCANCODE_E]      = KBD_ECM;
    kbd_table[SDL_SCANCODE_TAB]    = KBD_ENERGY_BOMB;
    kbd_table[SDL_SCANCODE_H]      = KBD_HYPERSPACE;
    kbd_table[SDL_SCANCODE_LCTRL]  = KBD_CTRL;
    kbd_table[SDL_SCANCODE_RCTRL]  = KBD_CTRL;
    kbd_table[SDL_SCANCODE_J]      = KBD_JUMP;
    kbd_table[SDL_SCANCODE_ESCAPE] = KBD_ESCAPE;

    kbd_table[SDL_SCANCODE_C] = KBD_DOCK;
    kbd_table[SDL_SCANCODE_D] = KBD_D;
    kbd_table[SDL_SCANCODE_O] = KBD_ORIGIN;
    kbd_table[SDL_SCANCODE_F] = KBD_FIND;

    kbd_table[SDL_SCANCODE_M] = KBD_FIRE_MISSILE;
    kbd_table[SDL_SCANCODE_T] = KBD_TARGET_MISSILE;
    kbd_table[SDL_SCANCODE_U] = KBD_UNARM_MISSILE;

    kbd_table[SDL_SCANCODE_P] = KBD_PAUSE;
    kbd_table[SDL_SCANCODE_R] = KBD_RESUME;
    
    kbd_table[SDL_SCANCODE_SPACE] = KBD_INC_SPEED;
    kbd_table[SDL_SCANCODE_SLASH] = KBD_DEC_SPEED;

    kbd_table[SDL_SCANCODE_S]      = KBD_UP;
    kbd_table[SDL_SCANCODE_UP]     = KBD_UP;
    kbd_table[SDL_SCANCODE_X]      = KBD_DOWN;
    kbd_table[SDL_SCANCODE_DOWN]   = KBD_DOWN;
    kbd_table[SDL_SCANCODE_COMMA]  = KBD_LEFT;
    kbd_table[SDL_SCANCODE_LEFT]   = KBD_LEFT;
    kbd_table[SDL_SCANCODE_PERIOD] = KBD_RIGHT;
    kbd_table[SDL_SCANCODE_RIGHT]  = KBD_RIGHT;

    kbd_table[SDL_SCANCODE_RETURN]    = KBD_ENTER;
    kbd_table[SDL_SCANCODE_BACKSPACE] = KBD_BACKSPACE;
}


int input_startup (void)
{
    int i;

//	SDL_EnableKeyRepeat(20,10);
//	SDL_EnableUNICODE(1);

	for (i = 0; i < KEY_LAST; i++)
    {
		key[i] = 0;
	}

    kbd_init_translation_table();

    for (i = 0; i < JOY_LAST; i++)
    {
        joy[i] = 0;
    }

    if (SDL_NumJoysticks())
    {
        SDL_JoystickEventState(SDL_ENABLE);
        SDL_JoystickOpen(0);
    }

	return 0;
}


int input_shutdown (void)
{
	return 0;
}


void kbd_translate_keys (void)
{
    int i;

    memset(kbd, 0, sizeof(int) * KBD_LAST);

    for (i = 0; i < KEY_LAST; i++)
    {
        kbd[kbd_table[i]] |= key[i];
    }

    kbd[KBD_UP]    |= joy[JOY_UP];
    kbd[KBD_DOWN]  |= joy[JOY_DOWN];
    kbd[KBD_LEFT]  |= joy[JOY_LEFT];
    kbd[KBD_RIGHT] |= joy[JOY_RIGHT];

    kbd[KBD_FIRE]      |= joy[JOY_BUTTON0];
    kbd[KBD_INC_SPEED] |= joy[JOY_BUTTON1];
    kbd[KBD_DEC_SPEED] |= joy[JOY_BUTTON2];
}


void handle_events (void)
{
	SDL_Event event;
		
	/* Poll for events. SDL_PollEvent() returns 0 when there are no  */
	/* more events on the event queue, our while loop will exit when */
	/* that occurs.                                                  */
	while (SDL_PollEvent(&event))
    {
		switch(event.type)
        {
			case SDL_KEYDOWN:
				key[event.key.keysym.scancode] = 1;
				handle_key_event(kbd_table[event.key.keysym.scancode], (char)(event.key.keysym.scancode & 0x7f));
				break;

			case SDL_KEYUP:
				key[event.key.keysym.scancode] = 0;
				break;

            case SDL_JOYAXISMOTION:
                if (event.jaxis.axis == 0)
                {
                    joy[JOY_LEFT]  = (event.jaxis.value < -3200);
                    joy[JOY_RIGHT] = (event.jaxis.value > 3200);
                }

                if (event.jaxis.axis == 1)
                {
                    joy[JOY_DOWN]  = (event.jaxis.value > 3200);
                    joy[JOY_UP]    = (event.jaxis.value < -3200);
                }
                break;

             case SDL_JOYBUTTONDOWN:
             case SDL_JOYBUTTONUP:
                switch (event.jbutton.button)
                {
                    case 0:
                        joy[JOY_BUTTON0] = (event.jbutton.state == SDL_PRESSED);
                        break;

                    case 1:
                        joy[JOY_BUTTON1] = (event.jbutton.state == SDL_PRESSED);
                        break;

                    case 2:
                        joy[JOY_BUTTON2] = (event.jbutton.state == SDL_PRESSED);
                        break;
                }
                break;

            case SDL_WINDOWEVENT_RESIZED:
				gfx_resize_window(event.window.data1, event.window.data2);
				break;

			case SDL_QUIT:
				finish_game();
				break;
				
			default:
				break;
		}
	}

    kbd_translate_keys();
}

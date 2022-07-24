/**
 * Elite - The New Kind.
 *
 * SDL/OpenGL Port by Mark Follett 2001-2002
 * email: <mef123@geocities.com>
 **/

#ifndef INPUT_H
#define INPUT_H
 
#define KBD_NONE            0
#define KBD_F1              1
#define KBD_F2              2
#define KBD_F3              3
#define KBD_F4              4
#define KBD_F5              5
#define KBD_F6              6
#define KBD_F7              7
#define KBD_F8              8
#define KBD_F9              9
#define KBD_F10             10
#define KBD_F11             11
#define KBD_F12             12
#define KBD_Y               13
#define KBD_N               14
#define KBD_FIRE            15
#define KBD_ECM             16
#define KBD_ENERGY_BOMB     17
#define KBD_HYPERSPACE      18
#define KBD_CTRL            19
#define KBD_JUMP            20
#define KBD_ESCAPE          21
#define KBD_DOCK            22
#define KBD_D               23
#define KBD_ORIGIN          24
#define KBD_FIND            25
#define KBD_FIRE_MISSILE    26
#define KBD_TARGET_MISSILE  27
#define KBD_UNARM_MISSILE   28
#define KBD_PAUSE           29
#define KBD_RESUME          30
#define KBD_INC_SPEED       31
#define KBD_DEC_SPEED       32
#define KBD_UP              33
#define KBD_DOWN            34
#define KBD_LEFT            35
#define KBD_RIGHT           36
#define KBD_ENTER           37
#define KBD_BACKSPACE       38
#define KBD_LAST            39

extern int kbd[KBD_LAST];


int input_startup (void);
int input_shutdown (void);
void handle_events (void);

#endif /* INPUT_H */


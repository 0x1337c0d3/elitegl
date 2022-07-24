/**
 * Elite - The New Kind.
 *
 * SDL/OpenGL Port by Mark Follett 2001-2002
 * email: <mef123@geocities.com>
 **/

#ifndef LOADSAVE_H
#define LOADSAVE_H

void initialize_load_commander_screen (void);
void draw_load_commander_screen (void);
void handle_load_commander_screen_keys (int key, char ascii);
void complete_load_commander_screen();

void initialize_save_commander_screen (void);
void draw_save_commander_screen (void);
void handle_save_commander_screen_keys (int key, char ascii);

#endif /* LOADSAVE_H */

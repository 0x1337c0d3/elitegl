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
 * file.h
 */

#ifndef FILE_H
#define FILE_H

#define MAX_CMDR_NAME_LENGTH      31

void write_config_file (void);
void read_config_file (void);
int save_commander_file (char *filename);
int load_commander_file (char *filename, struct commander *saved_cmdr);
char *concat_paths (char *patha, char *pathb);

#endif


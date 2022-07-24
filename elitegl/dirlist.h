/**
 * Elite - The New Kind.
 *
 * SDL/OpenGL Port by Mark Follett 2001-2002
 * email: <mef123@geocities.com>
 **/

#ifndef DIRLIST_H
#define DIRLIST_H


typedef struct Dirlist {
	char *filename;
	struct Dirlist *next;
} Dirlist;

Dirlist *dirlist_append(Dirlist *prev_elem, char *filename);
Dirlist *dirlist_create(char *filename);
void dirlist_free(Dirlist *dirlist);
Dirlist *get_dir_file_list(char *filename, char *pattern);


#endif /* DIRLIST_H */

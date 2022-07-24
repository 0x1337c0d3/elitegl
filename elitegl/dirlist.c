/**
 * Elite - The New Kind.
 *
 * SDL/OpenGL Port by Mark Follett 2001-2002
 * email: <mef123@geocities.com>
 **/

#include "dirlist.h"

#ifdef WIN32
#  include <windows.h>
#  include <direct.h>
#else
#  include <stdlib.h>
#  include <string.h>
#  include <sys/types.h>
#  include <dirent.h>
#  include <fnmatch.h>
#endif

#include <stdio.h>

Dirlist *dirlist_append(Dirlist *prev_elem, char *filename)
{
	Dirlist *cur_elem;

	cur_elem = (Dirlist *)malloc(sizeof(Dirlist));
	cur_elem->filename = (char *)malloc(strlen(filename) + 1);
	strcpy(cur_elem->filename, filename);

	if (prev_elem != NULL) {
		cur_elem->next = prev_elem->next;
		prev_elem->next = cur_elem;
	} else {
		cur_elem->next = NULL;
	}
	return cur_elem;
}

Dirlist *dirlist_create(char *filename)
{
	return dirlist_append(NULL, filename);
}

void dirlist_free(Dirlist *dirlist)
{
	Dirlist *cur_elem, *next;

	for (cur_elem = dirlist; cur_elem != NULL; cur_elem = next) {
		next = cur_elem->next;
		free(cur_elem->filename);
		free(cur_elem);
	}
}

// ripped from TuxRacer
Dirlist *get_dir_file_list( char *dirname , char *pattern )
{
#if defined ( WIN32 )

    /* Win32 */

	char path[MAX_PATH];
    Dirlist *dirlist = NULL;
    Dirlist *cur_elem = NULL;
    HANDLE hFind;
    WIN32_FIND_DATA finddata;

	_snprintf(path, MAX_PATH, "%s\\%s", dirname, pattern);

    if ( ( hFind = FindFirstFile( path, &finddata ) ) ==
	 INVALID_HANDLE_VALUE )
    {
		return NULL;
    }

	dirlist = dirlist_create(finddata.cFileName);
	cur_elem = dirlist;

	while ( FindNextFile( hFind, &finddata ) )
		cur_elem = dirlist_append(cur_elem, finddata.cFileName);

    if ( !FindClose( hFind ) ) {
		printf("Couldn't close directory %s", dirname);
    }

    return dirlist;

#else

    /* Unix/Linux */

    DIR *dir_stream;
    Dirlist *dirlist = NULL;
    Dirlist *cur_elem = NULL;
    struct dirent* cur_entry;

    dir_stream = opendir( dirname );

    if ( dir_stream == NULL ) {
	return NULL;
    }

    while ( ( cur_entry = readdir( dir_stream ) ) != NULL ) {
		if (fnmatch(pattern, cur_entry->d_name, 0) == 0) {
			if (dirlist == NULL) {
				dirlist = dirlist_create(cur_entry->d_name);
				cur_elem = dirlist;
			} else {
				cur_elem = dirlist_append(cur_elem, cur_entry->d_name);
			}
		}
    }

    if ( closedir( dir_stream ) != 0 ) {
		printf("Couldn't close directory %s", dirname);
    }

    return dirlist;

#endif /* defined ( WIN32 ) */
}




/**
 * Elite - The New Kind.
 *
 * SDL/OpenGL Port by Mark Follett 2001-2002
 * email: <mef123@geocities.com>
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "dirlist.h"
#include "gfx.h"
#include "docked.h"
#include "elite.h"
#include "main.h"
#include "file.h"
#include "input.h"

#define COMMANDER_LIST_SIZE       12

#define SUBSCR_LOAD_CMDR_SELECT   1
#define SUBSCR_LOAD_CMDR_CONFIRM  2

#define SUBSCR_SAVE_CMDR_INPUT    1
#define SUBSCR_SAVE_CMDR_CONFIRM  2

static int current_subscreen;

static Dirlist *commanders;
static int current_commander;
static int current_commander_base;
static int num_commanders;
static struct commander current_cmdr_data;
static int current_cmdr_file_is_corrupt;

static char save_name[MAX_CMDR_NAME_LENGTH + 1];
static int cmdr_file_exists;


void load_current_cmdr_data (void)
{
	int i;
	Dirlist *cur_elem;

	cur_elem = commanders;
	for (i = 0; i < current_commander; i++)
	{
		cur_elem = cur_elem->next;
	}

	current_cmdr_file_is_corrupt = load_commander_file (cur_elem->filename, &current_cmdr_data);
}


void initialize_load_commander_screen (void)
{
	Dirlist *cur_elem;

	commanders = get_dir_file_list(save_game_dir,"*.nkc");
	current_commander = 0;
	current_commander_base = 0;
	num_commanders = 0;

	for (cur_elem = commanders; cur_elem != NULL; cur_elem = cur_elem->next)
	{
		num_commanders++;
	}

	if (commanders != NULL)
	{
		load_current_cmdr_data();
	}

	current_subscreen = SUBSCR_LOAD_CMDR_SELECT;
}


void complete_load_commander_screen (void)
{
	dirlist_free(commanders);
}


void select_previous_commander (void)
{
	if (current_commander > 0)
	{
		current_commander--;
		load_current_cmdr_data();
	}
	if (current_commander < current_commander_base)
		current_commander_base--;
}


void select_next_commander (void)
{
	if (current_commander < (num_commanders - 1))
	{
		current_commander++;
		load_current_cmdr_data();
	}
	if ((current_commander - current_commander_base) == COMMANDER_LIST_SIZE)
		current_commander_base++;
}


void handle_load_commander_screen_keys (int key, char ascii)
{
	char str[100];

	switch (current_subscreen)
	{
    	case SUBSCR_LOAD_CMDR_SELECT:
	    	if (key == KBD_UP)
		    	select_previous_commander();

    		if (key == KBD_DOWN)
	    		select_next_commander();

		    if (key == KBD_ENTER)
			    if (commanders && !current_cmdr_file_is_corrupt)
				    current_subscreen = SUBSCR_LOAD_CMDR_CONFIRM;

    		if (key == KBD_ESCAPE)
	    		switch_to_screen(SCR_CMDR_STATUS);

    		break;

    	case SUBSCR_LOAD_CMDR_CONFIRM:
	    	if (key == KBD_Y) {
		    	saved_cmdr = current_cmdr_data;
			    restore_saved_commander();
			    switch_to_screen(SCR_CMDR_STATUS);
			    sprintf(str, "Commander %s Loaded.", current_cmdr_data.name);
			    info_message(str);
    		}

	    	if (key == KBD_N) {
		    	current_subscreen = SUBSCR_LOAD_CMDR_SELECT;
    		}
	    	break;
	}
}


void draw_load_commander_screen_select (void)
{
	int i, y;
	Dirlist *cur_elem;

	gfx_display_centre_text (10, "LOAD COMMANDER", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);

	if (commanders == NULL) {
		gfx_display_centre_text(200, "No saved commanders", 140, GFX_COL_GOLD);
		return;
	}

	if (current_cmdr_file_is_corrupt)
	{
		gfx_display_centre_text(90, "File is corrupt", 140, GFX_COL_GOLD);
	}
	else
	{
		display_abbreviated_status(&current_cmdr_data);
	}

	y = 160 + (current_commander - current_commander_base) * 16;
	gfx_draw_rectangle (2, y+1, 510, y + 15, GFX_COL_DARK_RED);

	cur_elem = commanders;
	for (i = 0; i < current_commander_base; i++) {
		cur_elem = cur_elem->next;
	}

	for (i = 0; ((i < COMMANDER_LIST_SIZE) && cur_elem); i++) {
		gfx_display_text(10, 160 + (i * 16), cur_elem->filename);
		cur_elem = cur_elem->next;
	}
}


void draw_load_commander_screen_confirm (void)
{
	char str[MAX_CMDR_NAME_LENGTH + 10];

	gfx_display_centre_text (10, "LOAD COMMANDER", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);

	gfx_display_centre_text (190, "Load Commander", 140, GFX_COL_GOLD);
	sprintf(str, "%s (Y/N)?", current_cmdr_data.name);
	gfx_display_centre_text (215, str, 140, GFX_COL_GOLD);
}


void draw_load_commander_screen (void)
{
	switch (current_subscreen)
	{

	case SUBSCR_LOAD_CMDR_SELECT:
		draw_load_commander_screen_select();
		break;

	case SUBSCR_LOAD_CMDR_CONFIRM:
		draw_load_commander_screen_confirm();
		break;

	}
}


void initialize_save_commander_screen (void)
{
	strcpy(save_name, cmdr.name);
	current_subscreen = SUBSCR_SAVE_CMDR_INPUT;
}


void draw_save_commander_screen_input (void)
{
	char str[MAX_CMDR_NAME_LENGTH + 10];

	gfx_display_centre_text (10, "SAVE COMMANDER", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);

	gfx_display_colour_text (16, 58, "Commander Name:", GFX_COL_GREEN_1);
	sprintf(str, "%s_", save_name);
	gfx_display_text(170, 58, str);

}


void draw_save_commander_screen_confirm (void)
{
	char str[MAX_CMDR_NAME_LENGTH + 10];

	gfx_display_centre_text (10, "SAVE COMMANDER", 140, GFX_COL_GOLD);
	gfx_draw_line (0, 36, 511, 36);

	gfx_display_centre_text (190, (cmdr_file_exists) ? "Overwrite Existing Commander" : "Save Commander", 140, GFX_COL_GOLD);
	sprintf(str, "%s (Y/N)?", save_name);
	gfx_display_centre_text (215, str, 140, GFX_COL_GOLD);
}


void draw_save_commander_screen (void)
{
	switch (current_subscreen)
	{

	case SUBSCR_SAVE_CMDR_INPUT:
		draw_save_commander_screen_input();
		break;

	case SUBSCR_SAVE_CMDR_CONFIRM:
		draw_save_commander_screen_confirm();
		break;

	}
}


static void add_name_char (int letter)
{
	int len;

	len = strlen(save_name);
	if (len == MAX_CMDR_NAME_LENGTH)
		return;

	save_name[len]   = toupper(letter);
	save_name[len+1] = '\0';
}


static void delete_name_char (void)
{
	int len;

	len = strlen (save_name);
	if (len == 0)
		return;

	save_name[len - 1] = '\0';	
}


void check_existing_file (void)
{
	Dirlist *dirlist = NULL;
	char str[MAX_CMDR_NAME_LENGTH + 4 + 1];
	sprintf(str, "%s.nkc", save_name);

	dirlist = get_dir_file_list(save_game_dir,str);
	if (dirlist)
	{
		cmdr_file_exists = 1;
	} else
	{
		cmdr_file_exists = 0;
	}
	dirlist_free(dirlist);
}


void save_commander (void)
{
	char str[MAX_CMDR_NAME_LENGTH + 4 + 1];
	sprintf(str, "%s.nkc", save_name);

	if (save_commander_file (str))
	{
		info_message("Error Saving Commander!");
	} else
	{
		info_message("Commander Saved.");
	}

	strcpy(cmdr.name, save_name);
	saved_cmdr = cmdr;
	saved_cmdr.ship_x = docked_planet.d;
	saved_cmdr.ship_y = docked_planet.b;
}


void handle_save_commander_screen_keys (int key, char ascii)
{
	switch (current_subscreen)
	{

    	case SUBSCR_SAVE_CMDR_INPUT:
	    	if (isalnum(ascii))
		    {
			    add_name_char (ascii);
    			return;
	    	}

    		if (key == KBD_BACKSPACE)
	    	{
		    	delete_name_char ();
			    return;
    		}
		
    		if (key == KBD_ENTER)
	    	{
		    	if (save_name[0] != '\0')
			    {
				    check_existing_file();
    				current_subscreen = SUBSCR_SAVE_CMDR_CONFIRM;
	    		}
		    }

    		if (key == KBD_ESCAPE)
	    		switch_to_screen(SCR_CMDR_STATUS);

    		break;

    	case SUBSCR_SAVE_CMDR_CONFIRM:

    		if (key == KBD_Y) {
	    		save_commander();
		    	switch_to_screen(SCR_CMDR_STATUS);
    		}

	    	if (key == KBD_N) {
		    	current_subscreen = SUBSCR_SAVE_CMDR_INPUT;
    		}
	    	break;
	}
}

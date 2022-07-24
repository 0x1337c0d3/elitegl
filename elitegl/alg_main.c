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
 * alg_main.c
 *
 * SDL/OpenGL version of the main game handler.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h> 
#include <ctype.h>
#include <time.h>

#include "config.h"
#include "gfx.h"
#include "main.h"
#include "vector.h"
#include "elite.h"
#include "docked.h"
#include "intro.h"
#include "shipdata.h"
#include "shipface.h"
#include "space.h"
#include "sound.h"
#include "threed.h"
#include "swat.h"
#include "random.h"
#include "options.h"
#include "stars.h"
#include "missions.h"
#include "pilot.h"
#include "loadsave.h"
#include "file.h"
#include "input.h"

#include <SDL2/SDL.h>


int frame_count;
int cross_timer;
int game_over_timer;
int break_pattern_timer;
int escape_pod_timer;
int escape_pod_ship;

int draw_lasers;
int mcount;
int message_count;
char message_string[80];
int rolling;
int climbing;
int game_paused;

int find_input;
char find_name[20];

double timeslice;
double timedelta;


/*
 * Initialise the game parameters.
 */

void initialise_game(void)
{
	set_rand_seed (time(NULL));

	restore_saved_commander();

	flight_speed = 1;
	flight_roll = 0;
	flight_climb = 0;
	docked = 1;
	front_shield = 255;
	aft_shield = 255;
	energy = 255;
	draw_lasers = 0;
	mcount = 0;
	hyper_ready = 0;
	find_input = 0;
	witchspace = 0;
	game_paused = 0;
	auto_pilot = 0;
	
	create_new_stars();
	clear_universe();

	cross_timer = 0;

    
	myship.max_speed = 40;		/* 0.27 Light Mach */
	myship.max_roll = 31;
	myship.max_climb = 8;		/* CF 8 */
	myship.max_fuel = 70;		/* 7.0 Light Years */
}


void reset_game (void)
{
    game_over = 0;
    initialise_game();
	dock_player ();
	switch_to_screen(SCR_INTRO_ONE);
}


void finish_game (void)
{
	finish = 1;
}







/*
 * Move the planet chart cross hairs to specified position.
 */


void move_cross (int dx, int dy)
{
	cross_timer = 5;

	if (current_screen == SCR_SHORT_RANGE)
	{
		cross_x += (dx * 4);
		cross_y += (dy * 4);
		return;
	}

	if (current_screen == SCR_GALACTIC_CHART)
	{
		cross_x += (dx * 2);
		cross_y += (dy * 2);

		if (cross_x < 1)
			cross_x = 1;
			
		if (cross_x > 510)
			cross_x = 510;

		if (cross_y < 37)
			cross_y = 37;
		
		if (cross_y > 293)
			cross_y = 293;
    }
}


/*
 * Draw the cross hairs at the specified position.
 */

void draw_cross (int cx, int cy, int size, int clip_ty)
{
    gfx_set_clip_region (0, 0, 510, clip_ty);
	gfx_draw_colour_line (cx - size, cy, cx + size, cy, GFX_COL_RED);
	gfx_draw_colour_line (cx, cy - size, cx, cy + size, GFX_COL_RED);
	gfx_set_clip_region (0, 0, 512, 512);
}



void draw_laser_sights (void)
{
	int laser = 0;
	int x1,y1,x2,y2;
	
	switch (current_screen)
	{
		case SCR_FRONT_VIEW:
			gfx_display_centre_text (32, "Front View", 120, GFX_COL_WHITE);
			laser = cmdr.front_laser;
			break;
		
		case SCR_REAR_VIEW:
			gfx_display_centre_text (32, "Rear View", 120, GFX_COL_WHITE);
			laser = cmdr.rear_laser;
			break;

		case SCR_LEFT_VIEW:
			gfx_display_centre_text (32, "Left View", 120, GFX_COL_WHITE);
			laser = cmdr.left_laser;
			break;

		case SCR_RIGHT_VIEW:
			gfx_display_centre_text (32, "Right View", 120, GFX_COL_WHITE);
			laser = cmdr.right_laser;
			break;
	}
	

	if (laser)
	{
		x1 = 128 * GFX_SCALE;
		y1 = (96-8) * GFX_SCALE;
		y2 = (96-16) * GFX_SCALE;
   
		gfx_draw_colour_line (x1-1, y1, x1-1, y2, GFX_COL_GREY_1); 
		gfx_draw_colour_line (x1, y1, x1, y2, GFX_COL_WHITE);
		gfx_draw_colour_line (x1+1, y1, x1+1, y2, GFX_COL_GREY_1); 

		y1 = (96+8) * GFX_SCALE;
		y2 = (96+16) * GFX_SCALE;

		gfx_draw_colour_line (x1-1, y1, x1-1, y2, GFX_COL_GREY_1); 
		gfx_draw_colour_line (x1, y1, x1, y2, GFX_COL_WHITE);
		gfx_draw_colour_line (x1+1, y1, x1+1, y2, GFX_COL_GREY_1);

		x1 = (128-8) * GFX_SCALE;
		y1 = 96 * GFX_SCALE;
		x2 = (128-16) * GFX_SCALE;

		gfx_draw_colour_line (x1, y1-1, x2, y1-1, GFX_COL_GREY_1);
		gfx_draw_colour_line (x1, y1, x2, y1, GFX_COL_WHITE);
		gfx_draw_colour_line (x1, y1+1, x2, y1+1, GFX_COL_GREY_1);

		x1 = (128+8) * GFX_SCALE;
		x2 = (128+16) * GFX_SCALE;

		gfx_draw_colour_line (x1, y1-1, x2, y1-1, GFX_COL_GREY_1);
		gfx_draw_colour_line (x1, y1, x2, y1, GFX_COL_WHITE);
		gfx_draw_colour_line (x1, y1+1, x2, y1+1, GFX_COL_GREY_1);
	}
}


void run_game_over_screen (void);
void run_escape_sequence (void);
void initialize_break_pattern (void);
void display_break_pattern (void);


void switch_to_screen (int screen)
{
    if (screen == current_screen)
        return;

	// destructor code for each screen
	switch (current_screen)
	{
		case SCR_LOAD_CMDR:
			complete_load_commander_screen();
			break;
	}

	current_screen = screen;

	// initialization code for each screen

	switch (screen)
	{
		case SCR_INTRO_ONE:
			gfx_set_camera(0);
			snd_play_midi (SND_ELITE_THEME, 1);
			initialise_intro1();
			break;

		case SCR_INTRO_TWO:
			gfx_set_camera(0);
			snd_play_midi (SND_BLUE_DANUBE, 1);
			initialise_intro2();

			flight_speed = 3;
			flight_roll = 0;
			flight_climb = 0;
			break;

		case SCR_FRONT_VIEW:
			gfx_set_camera(0);
            flip_stars();
			break;

		case SCR_REAR_VIEW:
			gfx_set_camera(180);
            flip_stars();
			break;

		case SCR_LEFT_VIEW:
			gfx_set_camera(90);
            flip_stars();
			break;

		case SCR_RIGHT_VIEW:
			gfx_set_camera(-90);
            flip_stars();
			break;

		case SCR_GALACTIC_CHART:
		case SCR_SHORT_RANGE:
            find_input = 0;
			center_cursor_on_planet(hyperspace_planet);
			sprintf(line1, "%s", "");
			sprintf(line2, "%s", "");
			break;

		case SCR_PLANET_DATA:
			break;

		case SCR_MARKET_PRICES:
            initialize_market_screen();
			break;

		case SCR_CMDR_STATUS:
			break;

		case SCR_BREAK_PATTERN:
            initialize_break_pattern();
			break;

		case SCR_INVENTORY:
			break;

		case SCR_EQUIP_SHIP:
            equip_ship();
			break;

		case SCR_OPTIONS:
            initialize_options_screen();
			break;

        case SCR_SETTINGS:
            initialize_settings_screen();
            break;

		case SCR_LOAD_CMDR:
			initialize_load_commander_screen();
			break;

		case SCR_SAVE_CMDR:
			initialize_save_commander_screen();
			break;

		case SCR_QUIT:
			break;

		case SCR_MISSION:
			gfx_set_camera(0);
			break;

        case SCR_GAME_OVER:
			gfx_set_camera(180);
            run_game_over_screen();
            break;

        case SCR_ESCAPE_POD:
            gfx_set_camera(0);
            run_escape_sequence();
            break;
	}
}


int ready_to_draw (void)
{
	static long oldtime = 0;
	static long ms_count = 0;
	long newtime;
    long elapsed;

	newtime = SDL_GetTicks();

	if ((newtime == oldtime) || (oldtime == 0)) {
		oldtime = newtime;
		return 0;
	}

    elapsed = newtime - oldtime;
	oldtime = newtime;

	ms_count += elapsed;
	frame_count = frame_count + (ms_count / speed_cap);
	ms_count = ms_count - ((ms_count / speed_cap) * speed_cap);

	if (frame_count > 0) {
		return 0;
	}

	timeslice = (double)ms_count / (double)speed_cap;
	timedelta = (double)elapsed / (double)speed_cap;
	return 1;
}


void draw_screen (void)
{
    if (game_paused)
        return;

    if (current_screen != SCR_GAME_OVER)
    	update_console ();

	switch (current_screen)
	{
		case SCR_INTRO_ONE:
			draw_intro1 ();
			break;

		case SCR_INTRO_TWO:
			draw_intro2 ();
			break;

		case SCR_FRONT_VIEW:
		case SCR_REAR_VIEW:
		case SCR_LEFT_VIEW:
		case SCR_RIGHT_VIEW:
			update_starfield ();
			gfx_draw_view();

			if (draw_lasers)
			{
				draw_laser_lines();
			}
			draw_laser_sights ();
			break;

		case SCR_GALACTIC_CHART:
			display_galactic_chart ();
			draw_cross (cross_x, cross_y, 8, 293);
			display_info_text ();
			break;

		case SCR_SHORT_RANGE:
			display_short_range_chart ();
			draw_cross (cross_x, cross_y, 16, 339);
			display_info_text ();
			break;

		case SCR_PLANET_DATA:
			display_data_on_planet ();
			break;

		case SCR_MARKET_PRICES:
            draw_market_screen ();
			break;

		case SCR_CMDR_STATUS:
			display_commander_status ();
			break;

		case SCR_BREAK_PATTERN:
            display_break_pattern();
			break;

		case SCR_INVENTORY:
			display_inventory ();
			break;

		case SCR_EQUIP_SHIP:
            draw_equip_screen();
			break;

		case SCR_OPTIONS:
			display_options ();
			break;

        case SCR_SETTINGS:
            game_settings_screen ();
            break;

		case SCR_LOAD_CMDR:
			draw_load_commander_screen();
			break;

		case SCR_SAVE_CMDR:
			draw_save_commander_screen();
			break;

		case SCR_QUIT:
			quit_screen ();
			break;

		case SCR_MISSION:
			draw_mission_screen();
			break;

        case SCR_GAME_OVER:
		    update_starfield();
            gfx_draw_view();
            gfx_display_centre_text (190, "GAME OVER", 140, GFX_COL_GOLD);
            break;

        case SCR_ESCAPE_POD:
            update_starfield();
            gfx_draw_view();
            if (escape_pod_timer < 90)
            {
        		gfx_display_centre_text (358, "Escape pod launched - Ship auto-destuct initiated.", 120, GFX_COL_WHITE);
            }
            break;
	}

	if (message_count > 0)
		gfx_display_centre_text (358, message_string, 120, GFX_COL_WHITE);

	if (!docked && hyper_ready &&
        (current_screen != SCR_GAME_OVER) &&
        (current_screen != SCR_ESCAPE_POD))
		display_hyper_status ();

	gfx_update_screen();
}


void add_find_char (int letter)
{
	char str[40];
	
	if (strlen (find_name) == 16)
		return;
		
	str[0] = toupper (letter);
	str[1] = '\0';
	strcat (find_name, str);

	sprintf (line1, "Planet Name? %s", find_name);
	sprintf (line2, "%s", "");
}


void delete_find_char (void)
{
	int len;

	len = strlen (find_name);
	if (len == 0)
		return;
		
	find_name[len - 1] = '\0';	
		
	sprintf (line1, "Planet Name? %s", find_name);
	sprintf (line2, "%s", "");
}


void handle_find_input (int key, char ascii)
{
	if (isalpha(ascii))
	{
		add_find_char (ascii);
		return;
	}
		
	if (key == KBD_ENTER)
	{
		find_input = 0;
		find_planet_by_name (find_name);
		return;
	}

	if (key == KBD_BACKSPACE)
	{
		delete_find_char ();
		return;
	}
	
	return;
}


void handle_roll_and_climb (void)
{
	if (kbd[KBD_RIGHT])
	{
		if (flight_roll > 0)
			flight_roll = 0;
		else
		{
			decrease_flight_roll();
			decrease_flight_roll();
			rolling = 1;
		}
	}

	if (kbd[KBD_LEFT])
	{
		if (flight_roll < 0)
			flight_roll = 0;
		else
		{
			increase_flight_roll();
			increase_flight_roll();
			rolling = 1;
		}
	}

    if (kbd[KBD_UP])
	{
		if (flight_climb > 0)
			flight_climb = 0;
		else
		{
			decrease_flight_climb();
		}
		climbing = 1;
	}

	if (kbd[KBD_DOWN])
	{
		if (flight_climb < 0)
			flight_climb = 0;
		else
		{
			increase_flight_climb();
		}
		climbing = 1;
	}
}


void handle_function_keys (int key)
{
	switch (key)
    {
    	case KBD_F1:
    		if (docked)
	    		launch_player();
		    else
			    switch_to_screen(SCR_FRONT_VIEW);
    		break;

    	case KBD_F2:
	    	if (!docked)
		    	switch_to_screen(SCR_REAR_VIEW);
		    break;

	    case KBD_F3:
		    if (!docked)
			    switch_to_screen(SCR_LEFT_VIEW);
    		break;

    	case KBD_F4:
		    if (!docked)
	    		switch_to_screen(SCR_RIGHT_VIEW);
    		else
			    switch_to_screen(SCR_EQUIP_SHIP);
		    break;

	    case KBD_F5:
            switch_to_screen(SCR_GALACTIC_CHART);
            break;

    	case KBD_F6:
            switch_to_screen(SCR_SHORT_RANGE);
            break;

	    case KBD_F7:
            switch_to_screen(SCR_PLANET_DATA);
            break;

	    case KBD_F8:
            if (!witchspace)
                switch_to_screen(SCR_MARKET_PRICES);
            break;

    	case KBD_F9:
            switch_to_screen(SCR_CMDR_STATUS);
            break;

    	case KBD_F10:
            switch_to_screen(SCR_INVENTORY);
            break;

    	case KBD_F11:
            switch_to_screen(SCR_OPTIONS);
            break;

    	default:
            break;
	}
}


void auto_dock (void)
{
	struct univ_object ship;

	ship.location.x = 0;
	ship.location.y = 0;
	ship.location.z = 0;
	
	set_init_matrix (ship.rotmat);
	ship.rotmat[2].z = 1;
	ship.rotmat[0].x = -1;
	ship.type = -96;
	ship.velocity = flight_speed;
	ship.acceleration = 0;
	ship.bravery = 0;
	ship.rotz = 0;
	ship.rotx = 0;

    // bug in TNK_1_0_RELEASE: this must be set
    // or we will fly into the planet!
    ship.flags = 0;

	auto_pilot_ship (&ship);

	if (ship.velocity > 22)
		flight_speed = 22;
	else
		flight_speed = ship.velocity;
	
	if (ship.acceleration > 0)
	{
		flight_speed++;
		if (flight_speed > 22)
			flight_speed = 22;
	}

	if (ship.acceleration < 0)
	{
		flight_speed--;
		if (flight_speed < 1)
			flight_speed = 1;
	}	

	if (ship.rotx == 0)
		flight_climb = 0;
	
	if (ship.rotx < 0)
	{
		increase_flight_climb();

		if (ship.rotx < -1)
			increase_flight_climb();
	}
	
	if (ship.rotx > 0)
	{
		decrease_flight_climb();

		if (ship.rotx > 1)
			decrease_flight_climb();
	}
	
	if (ship.rotz == 127)
		flight_roll = -14;
	else
	{
		if (ship.rotz == 0)
			flight_roll = 0;

		if (ship.rotz > 0)
		{
			increase_flight_roll();

			if (ship.rotz > 1)
				increase_flight_roll();
		}
		
		if (ship.rotz < 0)
		{
			decrease_flight_roll();

			if (ship.rotz < -1)
				decrease_flight_roll();
		}
	}
}


void run_escape_sequence (void)
{
	int newship;
	Matrix rotmat;

	flight_speed = 1;
	flight_roll = 0;
	flight_climb = 0;

	set_init_matrix (rotmat);
	rotmat[2].z = 1.0;
	
	newship = add_new_ship (SHIP_COBRA3, 0, 0, 200, rotmat, -127, -127);
	universe[newship].velocity = 7;
	snd_play_sample (SND_LAUNCH);

    escape_pod_timer = 0;
    escape_pod_ship = newship;
}


void update_escape_sequence (void)
{
    int i;
	int newship;

    newship = escape_pod_ship;

    if (escape_pod_timer < 90)
    {
		if (escape_pod_timer == 40)
		{
			universe[newship].flags |= FLG_DEAD;
			snd_play_sample (SND_EXPLODE);
		}

		update_universe();

		universe[newship].location.x = 0;
		universe[newship].location.y = 0;
		universe[newship].location.z += 2;

        escape_pod_timer++;

    } else if ((ship_count[SHIP_CORIOLIS] == 0) &&
               (ship_count[SHIP_DODEC] == 0))
	{
		auto_dock();

		if ((abs(flight_roll) < 3) && (abs(flight_climb) < 3))
		{
			for (i = 0; i < MAX_UNIV_OBJECTS; i++)
			{
				if (universe[i].type != 0)
					universe[i].location.z -= 1500;
			}

		}

		warp_stars = 1;
		update_universe();
    } else
    {
    	abandon_ship();
    }
}


void handle_flight_keys (void)
{
	if (kbd[KBD_FIRE])
    {
    	if (draw_lasers == 0)
	    	draw_lasers = fire_laser();
    }

	if (kbd[KBD_DOCK] && cmdr.docking_computer)
    {
        if (instant_dock)
            engage_docking_computer();
        else
            engage_auto_pilot();
    }
	
	if (kbd[KBD_ECM] && cmdr.ecm)
		activate_ecm(1);
	
	if (kbd[KBD_HYPERSPACE])
	{
		if (kbd[KBD_CTRL])
			start_galactic_hyperspace();
		else
			start_hyperspace();
	}

	if (kbd[KBD_JUMP] && !witchspace)
		jump_warp();
	
	if (kbd[KBD_FIRE_MISSILE])
		fire_missile();

	if (kbd[KBD_TARGET_MISSILE])
		arm_missile();		

	if (kbd[KBD_UNARM_MISSILE])
		unarm_missile();
	
	if (kbd[KBD_INC_SPEED])
		if (flight_speed < myship.max_speed)
			flight_speed++;

	if (kbd[KBD_DEC_SPEED])
		if (flight_speed > 1)
			flight_speed--;

	if (kbd[KBD_ENERGY_BOMB] && cmdr.energy_bomb)
	{
		cmdr.energy_bomb = 0;
		detonate_bomb = 1;
	}

    if (kbd[KBD_ESCAPE] && cmdr.escape_pod && !witchspace)
	{
        switch_to_screen(SCR_ESCAPE_POD);
	}

    if (kbd[KBD_PAUSE])
	   	game_paused = 1;
}


void handle_keyboard_state (void) {

    if (game_paused)
        return;

	switch (current_screen)
	{
		case SCR_FRONT_VIEW:
		case SCR_REAR_VIEW:
		case SCR_LEFT_VIEW:
		case SCR_RIGHT_VIEW:
			handle_roll_and_climb();
			break;

		case SCR_GALACTIC_CHART:
		case SCR_SHORT_RANGE:
			if (find_input)
			{
				return;
			}

			if (kbd[KBD_RIGHT])
				move_cross (1, 0);

			if (kbd[KBD_LEFT])
				move_cross (-1, 0);

			if (kbd[KBD_UP])
				move_cross (0, -1);

			if (kbd[KBD_DOWN])
				move_cross (0, 1);
		
			break;

        case SCR_GAME_OVER:
            return;
            break;

        case SCR_BREAK_PATTERN:
            return;
            break;

        case SCR_ESCAPE_POD:
            return;
            break;
	}

    if (!docked)
    	handle_flight_keys();
}


void handle_key_event (int key, char ascii)
{
    if (game_paused)
	{
		if (key == KBD_RESUME)
			game_paused = 0;
		return;
	}

	if ((current_screen != SCR_INTRO_ONE) &&
        (current_screen != SCR_INTRO_TWO) &&
        (current_screen != SCR_GAME_OVER) &&
        (current_screen != SCR_MISSION) &&
        (current_screen != SCR_ESCAPE_POD) &&
        (current_screen != SCR_BREAK_PATTERN))
	{
		handle_function_keys(key);
	}

	switch (current_screen)
	{
		case SCR_INTRO_ONE:
			if (key == KBD_Y)
			{
				snd_stop_midi();
				switch_to_screen(SCR_LOAD_CMDR);
			}
		
			if (key == KBD_N)
			{ 
				snd_stop_midi();
				switch_to_screen(SCR_INTRO_TWO);
			}
			break;

		case SCR_INTRO_TWO:
			if (key == KBD_INC_SPEED || key == KBD_FIRE) {
				snd_stop_midi();
				dock_player ();
				switch_to_screen(SCR_CMDR_STATUS);
			}
			break;

		case SCR_FRONT_VIEW:
		case SCR_REAR_VIEW:
		case SCR_LEFT_VIEW:
		case SCR_RIGHT_VIEW:
			if (key == KBD_D) {
                if (auto_pilot)
				    disengage_auto_pilot();
            }
			break;

		case SCR_GALACTIC_CHART:
		case SCR_SHORT_RANGE:
			if (find_input)
			{
				handle_find_input(key, ascii);
				return;
			}

			if (key == KBD_ORIGIN) {
				move_cursor_to_origin ();
			}
			if (key == KBD_D) {
				show_distance_to_planet ();
			}
			if (key == KBD_FIND) {
				find_input = 1;
				*find_name = '\0';
				sprintf(line1, "%s", "Planet Name?");
				sprintf(line2, "%s", "");
			}
			
			break;

		case SCR_EQUIP_SHIP:
			if (key == KBD_UP)
				select_previous_equip ();

			if (key == KBD_DOWN)
				select_next_equip ();

			if (key == KBD_ENTER)
				buy_equip ();

			break;
		
		case SCR_OPTIONS:
			if (key == KBD_UP)
				select_previous_option ();

			if (key == KBD_DOWN)
				select_next_option ();

			if (key == KBD_ENTER)
				do_option ();

			break;

		case SCR_SETTINGS:
			if (key == KBD_RIGHT)
				select_right_setting ();

			if (key == KBD_LEFT)
				select_left_setting ();

			if (key == KBD_UP)
				select_up_setting ();

			if (key == KBD_DOWN)
				select_down_setting ();

			if (key == KBD_ENTER)
				toggle_setting ();

			break;

        case SCR_MARKET_PRICES:
			if (docked)
			{
				if (key == KBD_RIGHT)
					buy_stock ();

				if (key == KBD_LEFT)
					sell_stock ();

				if (key == KBD_UP)
					select_previous_stock ();

				if (key == KBD_DOWN)
					select_next_stock ();
			}
            break;

		case SCR_LOAD_CMDR:
			handle_load_commander_screen_keys (key, ascii);
			break;

		case SCR_SAVE_CMDR:
			handle_save_commander_screen_keys (key, ascii);
			break;

		case SCR_QUIT:
			if (key == KBD_Y) {
				finish_game();
			}

			if (key == KBD_N) {
				if (docked)
					switch_to_screen(SCR_CMDR_STATUS);
				else
					switch_to_screen(SCR_FRONT_VIEW);
			}

			break;

		case SCR_MISSION:
			handle_mission_screen_keys (key, ascii);
			break;
	}
}


/*
 * Draw the game over sequence. 
 */

void run_game_over_screen (void)
{
	int i;
	int newship;
	Matrix rotmat;
	int type;
	
	flight_speed = 6;
	flight_roll = 0;
	flight_climb = 0;
	clear_universe();

	set_init_matrix (rotmat);

	newship = add_new_ship (SHIP_COBRA3, 0, 0, -400, rotmat, 0, 0);
	universe[newship].flags |= FLG_DEAD;

	for (i = 0; i < 5; i++)
	{
		type = (rand255() & 1) ? SHIP_CARGO : SHIP_ALLOY;
		newship = add_new_ship (type, (rand255() & 63) - 32,
								(rand255() & 63) - 32, -400, rotmat, 0, 0);
		universe[newship].rotz = ((rand255() * 2) & 255) - 128;
		universe[newship].rotx = ((rand255() * 2) & 255) - 128;
		universe[newship].velocity = rand255() & 15;
	}
	
    update_universe();

    message_count = 0;
    game_over_timer = 0;
}


void update_game_over_screen (void)
{
    update_universe();
    game_over_timer++;
    if (game_over_timer == 100)
    {
        reset_game();
    }
}


/*
 * Draw a break pattern (for launching, docking and hyperspacing).
 * Just draw a very simple one for the moment.
 */

void display_break_pattern (void)
{
    int i, j;

    i = break_pattern_timer;

	gfx_set_clip_region (1, 2, 510, 383);
	for (j = 0; j <= i; j++) {
	    gfx_draw_circle (256, 192, 30 + j * 15, GFX_COL_WHITE);
	}
	gfx_set_clip_region (0, 0, 512, 512);
}


void initialize_break_pattern (void)
{
    break_pattern_timer = 0;
}


void update_break_pattern (void)
{
    break_pattern_timer++;
    if (break_pattern_timer == 20)
    {
    	if (docked)
    	{
    		check_mission_brief();
    	}
    	else
        {
            switch_to_screen(SCR_FRONT_VIEW);
        }
    }
}


void info_message (char *message)
{
	strcpy (message_string, message);
	message_count = 37;
	snd_play_sample (SND_BEEP);
}




void update_simulation (void)
{
    if (auto_pilot)
        auto_dock ();

	update_universe();

	if (!rolling)
	{
		if (flight_roll > 0)
			decrease_flight_roll();
			
		if (flight_roll < 0)
			increase_flight_roll();
    }

	if (!climbing)
	{
		if (flight_climb > 0)
			decrease_flight_climb();

		if (flight_climb < 0)
			increase_flight_climb();
    }

	rolling = 0;
	climbing = 0;

	if (draw_lasers > 0)
        draw_lasers--;

	mcount--;
	if (mcount < 0)
		mcount = 255;

    if (((mcount & 127) == 0) && (auto_pilot))
        info_message ("Docking Computers On");

	if (((mcount & 3) == 0) && (hyper_ready))
		countdown_hyperspace();

	if ((mcount & 7) == 0)
		regenerate_shields();

	if ((mcount & 31) == 10)
    {
        if (energy < 50)
        {
            info_message ("ENERGY LOW");
            snd_play_sample (SND_BEEP);
        }

		update_altitude();
    }
				
	if ((mcount & 31) == 20)
		update_cabin_temp();
					
	if ((mcount == 0) && (!witchspace))
		random_encounter();
					
	cool_laser();				
	time_ecm();
}


void update_screen (void)
{
    if (game_paused)
        return;

	switch (current_screen)
	{
    	case SCR_INTRO_ONE:
	    	update_intro1();
		    break;

    	case SCR_INTRO_TWO:
	    	update_intro2();
		    break;

    	case SCR_MISSION:
	    	update_mission_screen();
		    break;

        case SCR_GAME_OVER:
            update_game_over_screen();
            break;

        case SCR_BREAK_PATTERN:
            update_break_pattern();
            break;

        case SCR_ESCAPE_POD:
            update_escape_sequence();
            break;

    	default:
	    	if (!docked)
		    {
    			update_simulation();

                if (game_over == 1)
                {
                    switch_to_screen(SCR_GAME_OVER);
                }
	    	}

    	    if (cross_timer > 0)
    	    {
        		cross_timer--;
	        	if (cross_timer == 0)
		        {
    		        show_distance_to_planet ();
        		}
	        }
    }

   	if (message_count > 0) 
        message_count--;
}



int main( int argc, char **argv )
{
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
    {
        printf("Could not initialize SDL: %s.\n", SDL_GetError());
        return 1;
	}

    input_startup();

	read_config_file();

	if (gfx_graphics_startup () == 1)
	{
		return 1;
	}
	
	snd_sound_startup();

	frame_count = 0;
	finish = 0;
	
    reset_game();

	while (!finish)
	{
		snd_update_sound();
		if (ready_to_draw())
			draw_screen();

		while (frame_count > 0)
        {
			gfx_advance_frame();

    		handle_events();
			handle_keyboard_state();

   			update_screen();
			frame_count--;
		}
	}

	snd_sound_shutdown();
    input_shutdown();
	gfx_graphics_shutdown ();
	
    SDL_Quit();

	return 0;
}


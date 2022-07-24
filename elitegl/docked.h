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

#ifndef DOCKED_H
#define DOCKED_H

#include "elite.h"

void display_short_range_chart (void);
void display_galactic_chart (void);
void display_data_on_planet (void);
void show_distance_to_planet (void);
void move_cursor_to_origin (void);
void find_planet_by_name (char *find_name);
void display_market_prices (void);
void display_commander_status (void);
int calc_distance_to_planet (struct galaxy_seed from_planet, struct galaxy_seed to_planet);
void highlight_stock (int i);
void select_previous_stock (void);
void select_next_stock (void);
void buy_stock (void);
void sell_stock (void);
void display_inventory (void);
void equip_ship (void);
void select_next_equip (void);
void select_previous_equip (void);
void buy_equip (void);


extern int cross_x;
extern int cross_y;

void draw_equip_screen (void);
void draw_market_screen (void);
void center_cursor_on_planet(struct galaxy_seed planet);
void display_info_text (void);
void display_abbreviated_status (struct commander *cmdr);
void initialize_market_screen (void);

extern char line1[];
extern char line2[];

#endif


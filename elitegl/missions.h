#ifndef MISSIONS_H
#define MISSIONS_H

char *mission_planet_desc (struct galaxy_seed planet);
void check_mission_brief (void);
void draw_mission_screen(void);
void handle_mission_screen_keys (int key, char ascii);
void update_mission_screen(void);

#endif

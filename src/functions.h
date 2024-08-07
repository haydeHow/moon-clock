#ifndef FUNCTIONS_H
#define FUNCTIONS_H

void get_lat_and_lon(float coords[2]);
void get_temp(char *current_temp);
void get_time(char *current_time);
void get_moon(char *current_phase);
void get_next_full(char *next_phase);
void get_date(char *date);

void format_print_temp(char *temp);
void format_print_time(char *time);
void format_print_date(char *date);
void format_print_moon_phase(char *phase);
void format_print_next_full(char *next_phase);
void format_print_moon_phase_picture(char *phase);

void init_wifi();
void init_ssd1306();
int init_params(char *init_time, char *init_temp, char *init_moon_phase, char *init_next_full, char *init_date);
void draw_vertical_split();
void clear_section(int x, int y, int w, int h);

void minute_update(char *time);
void quarter_update(char *temp);
void daily_update(char *temp, char *moon_phase, char *next_full, char *date);

int time_to_daily_update(char *time);
int time_to_quarter_update(char *time);

#endif

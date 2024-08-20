#ifndef FUNCTIONS_H
#define FUNCTIONS_H

enum InitState
{
    INIT_RUNNING,
    INIT_COMPLETE
};

extern InitState init_state;

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

int should_minute_update(unsigned long current);
int should_quarter_update(unsigned long current);
int should_daily_update(unsigned long current);

#endif

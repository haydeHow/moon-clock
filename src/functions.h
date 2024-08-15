#ifndef FUNCTIONS_H
#define FUNCTIONS_H

typedef struct 
{
    char time[10];
    char temp[10];
    char moon[15];
    char next_full[15];
    char date[10];

} Data; 

enum InitState
{
    INIT_IDLE,
    INIT_RUNNING,
    INIT_COMPLETE
};

void get_lat_and_lon(float coords[2]);
void get_temp();
void get_time();
void get_moon();
void get_next_full();
void get_date();

void format_print_temp();
void format_print_time();
void format_print_date();
void format_print_moon_phase();
void format_print_next_full();
void format_print_moon_phase_picture();

void draw_vertical_split();
void clear_section(int x, int y, int w, int h);

void init_wifi();
void init_ssd1306();
int init_params();
int init_complete();

#endif

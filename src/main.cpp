#include "functions.h"
#include <Arduino.h>

void setup()
{
    Serial.begin(9600);
    init_wifi();
    init_ssd1306();
}

void loop()
{

    unsigned long current_millis = millis();

    static char time[20] = "";
    static char temp[10] = "";
    static char moon_phase[15] = "";
    static char next_full[15] = "";
    static char date[10] = "";

    switch (init_state)
    {
    case INIT_RUNNING:
        if (init_params(time, temp, moon_phase, next_full, date))
            init_state = INIT_COMPLETE;
        break;

    case INIT_COMPLETE:
        if (should_minute_update(current_millis))
            minute_update(time);

        if (should_quarter_update(current_millis))
            quarter_update(temp);

        if (should_daily_update(current_millis))
            init_state = INIT_RUNNING;

    default:
        break;
    }
}

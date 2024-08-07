#include "functions.h"
#include <Arduino.h>

enum InitState
{
    INIT_IDLE,
    INIT_RUNNING,
    INIT_COMPLETE
};

InitState init_state = INIT_IDLE;

void setup()
{
    Serial.begin(9600);
    init_wifi();
    init_ssd1306();
    init_state = INIT_RUNNING;
}

void loop()
{
    static char time[20];
    static char temp[8];
    static char moon_phae[10] = "";
    static char next_full[10] = "";
    static char date[10] = "";

    switch (initState)
    {
    case INIT_RUNNING:
        if (init_params(time, temp, moon_phase, next_full, date))
        {
            initState = INIT_COMPLETE;
        }
        break;

    case INIT_COMPLETE:
        get_time(timeBuffer);
        minute_update(timeBuffer);

        if (time_to_quarter_update(timeBuffer))
            quarter_update(temp);

        if (time_to_daily_update(timeBuffer))
            daily_update(temp, moonPhase, nextFull, dateBuffer);

        delay(60000);
        break;

    default:
        break;
    }
}

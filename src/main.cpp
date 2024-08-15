#include "functions.h"
#include <Arduino.h>

void setup()
{
    Serial.begin(9600);
    init_wifi();
    init_ssd1306();
    initState = INIT_RUNNING;
}

void loop()
{
    switch (initState)
    {
    case INIT_RUNNING:
        if (init_complete())
            initState = INIT_COMPLETE;
        break;

    case INIT_COMPLETE:
        get_time(time);
        minute_update(time);

        if (time_to_quarter_update(time))
            quarter_update(temp);

        if (time_to_daily_update(time))
            setup();

        delay(60000);
        break;

    default:
        break;
    }
}

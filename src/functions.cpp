// #include <DNSServer.h>
// #include <WiFiManager.h> // Include WiFiManager library
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <string.h>

#include "functions.h"
#include "phases.h"
#include "secrets.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SSD1306_I2C_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
HTTPClient http;
WiFiClient client;
InitState init_state = INIT_RUNNING;

const unsigned long minute_interval = 60000;
const unsigned long quarter_interval = 900000;
const unsigned long daily_interval = 86400000;

unsigned long previous_minute = 0;
unsigned long previous_minute_quarter = 0;
unsigned long previous_minute_day = 0;

void get_lat_and_lon(float coords[2])
{
    // Make a request to ip-api.com
    if (WiFi.status() == WL_CONNECTED)
    {
        http.begin(client, "http://ip-api.com/json");

        int httpCode = http.GET();
        if (httpCode > 0)
        {
            String payload = http.getString();

            DynamicJsonDocument doc(1024);
            deserializeJson(doc, payload);

            float lat = doc["lat"];
            float lon = doc["lon"];

            coords[0] = lat;
            coords[1] = lon;
        }
        http.end();
    }
    else
        Serial.println("Error in get_lat_and_lon FUNCTION");
}

// FINISHED
void get_temp(char *current_temp)
{
    memset(current_temp, '\0', 10);

    float coords[2];
    get_lat_and_lon(coords);

    char lat[20];
    char lon[20];

    sprintf(lat, "%.2f", coords[0]);
    sprintf(lon, "%.2f", coords[1]);

    if (WiFi.status() == WL_CONNECTED)
    {

        char serverPath[1024] = "";
        strcat(serverPath, "https://api.openweathermap.org/data/3.0/onecall?lat=");
        strcat(serverPath, lat);
        strcat(serverPath, "&lon=");
        strcat(serverPath, lon);
        strcat(serverPath, "&exclude=minutely,daily,hourly,alerts&appid=");
        strcat(serverPath, API_KEY);
        strcat(serverPath, "&units=imperial");

        http.begin(client, API);
        int httpCode = http.GET();

        if (httpCode > 0)
        {
            String payload = http.getString();
            // Serial.println(payload);

            DynamicJsonDocument doc(1024);
            deserializeJson(doc, payload);

            float temperature = doc["current"]["temp"];
            char str_temp[5];
            sprintf(str_temp, "%.0f", temperature);
            strcpy(current_temp, str_temp);
        }
        http.end();
    }
    else
        Serial.println("Error in get_temp FUNCTION");

}

void get_time(char *time)
{
    // Clear the contents of the 'time' array
    memset(time, '\0', 20);

    // Check if the WiFi is connected
    if (WiFi.status() == WL_CONNECTED)
    {
        const char *url = "http://worldtimeapi.org/api/timezone/America/New_York";
        http.begin(client, url);

        int httpResponseCode = http.GET();
        if (httpResponseCode > 0)
        {
            String payload = http.getString();

            // Parse JSON
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, payload);
            const char *datetime = doc["datetime"];

            char current_without[6] = {0}; // Initialize the array

            int time_iter = 0;
            for (int i = 11; i < 16; ++i)
            {
                current_without[time_iter] = datetime[i];
                time_iter++;
            }

            // Copy the parsed time to the 'time' variable
            strcpy(time, current_without);
        }
        http.end();
    }
    else
        Serial.println("Error in get_time FUNCTION: WiFi not connected");
}

void get_moon(char *moon)
{
    memset(moon, '\0', 15);
    float coords[2];
    double phase;
    char phase_str[5];

    get_lat_and_lon(coords);

    char lat[20];
    char lon[20];

    sprintf(lat, "%.2f", coords[0]);
    sprintf(lon, "%.2f", coords[1]);

    if (WiFi.status() == WL_CONNECTED)
    {

        char url[160] = "http://api.openweathermap.org/data/3.0/onecall?lat=";
        strcat(url, lat);
        strcat(url, "&lon=");
        strcat(url, lon);
        strcat(url, "&exclude=minutely,hourly,alerts&appid=");
        strcat(url, API_KEY);
        strcat(url, "&units=imperial");

        http.begin(client, url);
        int httpCode = http.GET();

        if (httpCode > 0)
        {
            String payload = http.getString();

            DynamicJsonDocument doc(1024);
            deserializeJson(doc, payload);

            phase = doc["daily"][0]["moon_phase"];
            phase *= 1;
        }
        http.end();
    }
    else
        Serial.println("Error in get_temp FUNCTION");

    if (phase == 0.25)
        strcpy(moon, "FIR QUAR");
    else if (phase == 0.50)
        strcpy(moon, "FULL MOON");
    else if (phase == 0.75)
        strcpy(moon, "THI QUAR");
    else if ((phase == 1.00) || (phase == 0.00))
        strcpy(moon, "NEW MOON");
    else
        Serial.println("");

    if ((phase > 0.0) && (phase < 0.25))
        strcpy(moon, "WAX CRES");
    else if ((phase > .25) && (phase < 0.50))
        strcpy(moon, "WAX GIBB");
    else if ((phase > .50) && (phase < 0.75))
        printf("WAN GIBB");
    else if ((phase > 0.75) && (phase < 1.00))
        strcpy(moon, "WAX CRES");
    else
        Serial.println("ERROR in get_phase");
}

// FINISHED
void get_next_full(char *next_phase)
{
    memset(next_phase, '\0', 15);
    float coords[2];
    get_lat_and_lon(coords);

    char lat[20];
    char lon[20];

    sprintf(lat, "%.2f", coords[0]);
    sprintf(lon, "%.2f", coords[1]);

    // Serial.println(lat);
    // Serial.println(lon);
    // Serial.println("");
    // Serial.println("");

    if (WiFi.status() == WL_CONNECTED)
    {
        WiFiClientSecure client;
        const char *url = "https://moon-phase.p.rapidapi.com/basic";

        client.setInsecure();
        http.begin(client, url);
        http.addHeader("x-rapidapi-host", "moon-phase.p.rapidapi.com");
        http.addHeader("x-rapidapi-key", RAPID_API_KEY);

        int httpCode = http.GET();

        if (httpCode > 0)
        {
            String payload = http.getString();

            DynamicJsonDocument doc(1024);
            deserializeJson(doc, payload);

            int next_full = doc["days_until_next_full_moon"];
            char next_full_str[10] = "";

            if (next_full == 1)
                sprintf(next_full_str, "%d day", next_full);

            sprintf(next_full_str, "%d days", next_full);
            char *s_char = strchr(next_full_str, 's');
            int pos = s_char - next_full_str;
            next_full_str[pos + 1] = '\0';

            strcpy(next_phase, next_full_str);
        }
        http.end();
    }
    else
        Serial.println("Error in get_next_full FUNCTION");
}

void get_date(char *date)
{
    memset(date, '\0', 10);
    if (WiFi.status() == WL_CONNECTED)
    {
        const char *url = "http://worldtimeapi.org/api/timezone/America/New_York";
        http.begin(client, url);

        int httpResponseCode = http.GET();
        if (httpResponseCode > 0)
        {
            String payload = http.getString();

            // Parse JSON
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, payload);
            if (error)
            {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error.c_str());
                http.end();
                return;
            }

            const char *utc_datetime = doc["datetime"];

            int date_iter = 0;
            for (int i = 5; i < 10; ++i)
            {
                if (i == 5 && utc_datetime[i] == '0')
                    continue;
                if (utc_datetime[i] == '-')
                {
                    date[date_iter] = '/';
                    date_iter++;
                    continue;
                }
                date[date_iter] = utc_datetime[i];
                date_iter++;
            }

            http.end();
        }
    }
    else
        Serial.println("Error in get_date FUNCTION");
}

const uint8_t degreeSymbol[] PROGMEM = {0x00, 0x06, 0x09, 0x09, 0x06};

void format_print_temp(char *temp)
{
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    // display.setCursor(1, 37);
    display.setCursor(1, 25);
    display.drawBitmap(9, 23, degreeSymbol, 8, 8, WHITE);

    display.print(temp);
    display.display();
}
void format_print_date(char *date)
{
    display.setCursor(1, 37);
    display.print(date);
    display.display();
}
void format_print_moon_phase(char *phase)
{
    int line_width = 48;

    if (strcmp(phase, "FULL MOON") == 0)
        line_width = 55;

    display.drawLine(1, 15, line_width, 15, SSD1306_WHITE);
    display.setCursor(1, 7);
    display.setTextSize(1);
    display.print(phase);
    display.display();
}
void format_print_next_full(char *next_phase)
{
    display.setCursor(1, 48);
    display.print(next_phase);
    display.display();
}
void format_print_moon_phase_picture(char *phase)
{

    if (strcmp(phase, "NEW MOON") == 0)
    {
        display.drawBitmap(72, 14, new_moon, 50, 50, SSD1306_WHITE);
        display.display();
        return;
    }
    else if (strcmp(phase, "WAX GIBB") == 0)
    {
        display.drawBitmap(72, 14, waxing_gibbous, 50, 50, SSD1306_WHITE);
        display.display();
        return;
    }
    else if (strcmp(phase, "WAX CRES") == 0)
    {
        display.drawBitmap(72, 14, waxing_crescent, 50, 50, SSD1306_WHITE);
        display.display();
        return;
    }
    else if (strcmp(phase, "FIR QUAR") == 0)
    {
        display.drawBitmap(72, 14, first_quarter, 50, 50, SSD1306_WHITE);
        display.display();
        return;
    }
    else if (strcmp(phase, "WAN GIBB") == 0)
    {
        display.drawBitmap(72, 14, waning_gibbous, 50, 50, SSD1306_WHITE);
        display.display();
        return;
    }
    else if (strcmp(phase, "WAN CRES") == 0)
    {
        display.drawBitmap(72, 14, waning_crescent, 50, 50, SSD1306_WHITE);
        display.display();
        return;
    }
    else if (strcmp(phase, "FULL MOON") == 0)
    {
        display.drawBitmap(72, 14, full_moon, 50, 50, SSD1306_WHITE);
        display.display();
        return;
    }
    else
        Serial.println("ERROR in print_moon_phase_picture()");
}
void format_print_time(char *time)
{
    char format_time[6];
    int hour, minute;

    sscanf(time, "%d:%d", &hour, &minute);
    if (hour == 0)
        hour = 12;
    if (hour > 12)
        hour -= 12;

    sprintf(format_time, "%d:%02d", hour, minute);

    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    if (strlen(format_time) == 5)
        display.setCursor(68, 1);
    else
        display.setCursor(80, 1);
    display.print(format_time);
    display.display();
}
void draw_vertical_split()
{
    int x = 62;
    int y = 20;
    int length = 40;
    display.drawLine(x, y, x, y + length, SSD1306_WHITE);
    display.display();
}
void init_ssd1306()
{
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ;
    }
    display.clearDisplay();
    draw_vertical_split();
}
void init_wifi()
{
    WiFi.begin(SSID, PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}
int init_params(char *init_time, char *init_temp, char *init_moon_phase, char *init_next_full, char *init_date)
{
    // get temp
    get_temp(init_temp);
    // Serial.println(init_temp);
    format_print_temp(init_temp);

    // get time
    get_time(init_time);
    // Serial.println(time);
    format_print_time(init_time);

    // get phase
    get_moon(init_moon_phase);
    // Serial.println(moon_phase);
    format_print_moon_phase(init_moon_phase);
    format_print_moon_phase_picture(init_moon_phase);

    // get next full
    get_next_full(init_next_full);
    // Serial.println(init_next_full);
    format_print_next_full(init_next_full);

    // get date
    get_date(init_date);
    // Serial.println(date);
    format_print_date(init_date);

    return 1;
}

void clear_section(int x, int y, int w, int h)
{
    display.setTextSize(1);
    // display.setTextColor(SSD1306_WHITE);
    // display.drawRect(x, y, w, h, SSD1306_WHITE);
    display.fillRect(x, y, w, h, SSD1306_BLACK);
    display.display();
}
/*
int time_to_daily_update(char *time)
{
    if (strcmp(time, "00:00") == 0)
        return 1;
    return 0;
}
int time_to_quarter_update(char *time)
{
    int case_fifteen = strcmp(time + 3, "15");
    int case_thirty = strcmp(time + 3, "30");
    int case_forty_five = strcmp(time + 3, "45");
    int case_hour = strcmp(time + 3, "00");
    if (case_fifteen == 0 || case_thirty == 0 || case_forty_five == 0 || case_hour == 0)
        return 1;
    return 0;
}
*/

int should_minute_update(unsigned long current)
{
    if (current - previous_minute >= minute_interval)
    {
        previous_minute = current;
        return 1;
    }
    return 0;
}
int should_quarter_update(unsigned long current)
{
    if (current - previous_minute_quarter >= quarter_interval)
    {
        previous_minute_quarter = current;
        return 1;
    }
    return 0;
}
int should_daily_update(unsigned long current)
{
    if (current - previous_minute_day >= daily_interval)
    {
        previous_minute_day = current;
        return 1;
    }
    return 0;
}
void minute_update(char *time)
{
    clear_section(60, 0, 70, 17);
    format_print_time(time);
}
void quarter_update(char *temp)
{
    clear_section(0, 20, 40, 15);
    format_print_temp(temp);
}

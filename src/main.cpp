// #include <DNSServer.h>
// #include <WiFiManager.h> // Include WiFiManager library
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>

#include "phases.h"
#include "secrets.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SSD1306_I2C_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
HTTPClient http;
WiFiClient client;

// PROTOS
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
void draw_vertical_split();
void clear_section(int x, int y, int w, int h);

void minute_update(char *time);
void quarter_update(char *temp);
void daily_update(char *moon_phase, char *next_full, char *date);

int time_to_daily_update(char *time);
int time_to_quarter_update(char *time);
bool delay();

void setup()
{
    // wire.begin();
    Serial.begin(9600);
    // delay(5000);

    /*

    // Create an instance of WiFiManager
    WiFiManager wifiManager;

    // Uncomment to reset saved settings (for testing purposes)
    // wifiManager.resetSettings();

    // Start WiFiManager and connect to network
    if (!wifiManager.autoConnect("AutoConnectAP")) {
        Serial.println("Failed to connect and hit timeout");
        delay(3000);
        ESP.reset();
        delay(5000);
    }
    */

    // WiFi Setup
    init_wifi();

    // Display Setup
    // init_ssd1306();

    static char temp[8];
    static char time[20];
    static char moon_phase[10] = "";
    static char next_full[10] = "";
    static char date[10] = "";

    Serial.println("");

    // get temp
    // get_temp(temp);
    // Serial.println(temp);
    // format_print_temp(temp);

    // get time
    get_time(time);
    Serial.println(time);
    // format_print_time(time);

    // get phase
    get_moon(moon_phase);
    Serial.println(moon_phase);
    // format_print_moon_phase(moon_phase);
    // format_print_moon_phase_picture(moon_phase);

    // get date
    get_date(date);
    Serial.println(date);
    // format_print_date(date);

    // get next full
    get_next_full(next_full);
    Serial.println(next_full);
    // format_print_next_full(next_full);
}

void loop()
{
    static char temp[8];
    static char time[20];
    static char moon_phase[10] = "";
    static char next_full[10] = "";
    static char date[10] = "";

    if (delay())
    {

        minute_update(time);

        if (time_to_daily_update(time))
        {
        }

        if (time_to_quarter_update(time))
        {
        }
    }
}

// FINISHED
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
    float coords[2];
    get_lat_and_lon(coords);

    char lat[20];
    char lon[20];

    sprintf(lat, "%.4f", coords[0]);
    sprintf(lon, "%.4f", coords[1]);

    if (WiFi.status() == WL_CONNECTED)
    {

        char serverPath[1024] = "";
        strcat(serverPath, "https://api.openweathermap.org/data/3.0/onecall?lat=");
        strcat(serverPath, lat);
        strcat(serverPath, "&lon=");
        strcat(serverPath, lon);
        strcat(serverPath, "&exclude=minutely,daily,hourly,alerts&appid=");
        strcat(serverPath, APIKEY);
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
            char str_temp[3];
            sprintf(str_temp, "%.0f", temperature);
            str_temp[2] = '\0';
            strcpy(current_temp, str_temp);
        }
        http.end();
    }
    else
        Serial.println("Error in get_temp FUNCTION");
}

// FINISHED
void get_time(char *time)
{
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

            char current_without[6];

            int time_iter = 0;
            for (int i = 11; i < 16; ++i)
            {
                current_without[time_iter] = datetime[i];
                time_iter++;
            }
            current_without[5] = '\0';
            strcpy(time, current_without);
        }
        http.end();
    }
    else
        Serial.println("Error in get_time FUNCTION");
}

// FINISHED
void get_moon(char *current_moon)
{
    float coords[2];
    get_lat_and_lon(coords);

    char lat[20];
    char lon[20];

    sprintf(lat, "%.4f", coords[0]);
    sprintf(lon, "%.4f", coords[1]);

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

            const char *current = doc["phase_name"];

            const char *new_moon = "NEW MOON";
            const char *wax_gib = "WAX GIBB";
            const char *wax_cres = "WAX CRES";
            const char *first_quarter = "FIR QUAR";
            const char *second_quarter = "SEC QUAR";
            const char *wan_gib = "WAN GIBB";
            const char *wan_cres = "WAN CRES";
            const char *full_moon = "FULL MOON";

            char phase[10];

            if (strcmp(current, "New Moon") == 0)
                strcpy(phase, new_moon);
            else if (strcmp(current, "Waxing Gibbous") == 0)
                strcpy(phase, wax_gib);
            else if (strcmp(current, "Waxing Crescent") == 0)
                strcpy(phase, wax_cres);
            else if (strcmp(current, "First Quarter") == 0)
                strcpy(phase, first_quarter);
            else if (strcmp(current, "Second Quarter") == 0)
                strcpy(phase, second_quarter);
            else if (strcmp(current, "Waning Gibbous") == 0)
                strcpy(phase, wan_gib);
            else if (strcmp(current, "Waning Crescent") == 0)
                strcpy(phase, wan_cres);
            else if (strcmp(current, "Full Moon") == 0)
                strcpy(phase, full_moon);
            else
                Serial.println("Moon Name Error in get_moon FUNCTION");

            strcpy(current_moon, phase);
        }
        http.end();
    }
    else
        Serial.println("Error in get_moon FUNCTION");
}

// FINISHED
void get_next_full(char *next_phase)
{
    float coords[2];
    get_lat_and_lon(coords);

    char lat[20];
    char lon[20];

    sprintf(lat, "%.4f", coords[0]);
    sprintf(lon, "%.4f", coords[1]);

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
{ // Ensure date array can hold "yyyy-mm-dd" plus null terminator
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
    display.setCursor(35, 25);
    display.drawBitmap(43, 23, degreeSymbol, 8, 8, WHITE);

    display.print(temp);
    display.display();
}
void format_print_date(char *date)
{
    display.setCursor(1, 25);
    display.print(date);
    display.display();
}
void format_print_moon_phase(char *phase)
{
    int line_width = 0;
    if (strcmp(phase, "WAX GIBB") == 0)
        line_width = 48;
    else if (strcmp(phase, "WAN GIBB") == 0)
        line_width = 48;

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
    else if (strcmp(phase, "WAX CRES") == 0)
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
void clear_section(int x, int y, int w, int h)
{
    display.setTextSize(1);
    // display.setTextColor(SSD1306_WHITE);
    // display.drawRect(x, y, w, h, SSD1306_WHITE);
    display.fillRect(x, y, w, h, SSD1306_BLACK);
    display.display();
}

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

void minute_update(char *time)
{
    // TODO clear time area
    get_time(time);
    format_print_time(time);
}
void quarter_update(char *temp)
{
    // TODO clear temp area
    get_temp(temp);
    format_print_temp(temp);
}
void daily_update(char *time, char *temp, char *moon_phase, char *next_full, char *date)
{
    display.clearDisplay();

    get_time(time);
    get_temp(temp);
    get_moon(moon_phase);
    get_next_full(next_full);
    get_date(date);

    format_print_time(time);
    format_print_temp(temp);
    format_print_moon_phase(moon_phase);
    format_print_moon_phase_picture(moon_phase);
    format_print_date(date);
    format_print_next_full(next_full);
}

int delay()
{
    static unsigned long startMillis = 0; // Stores the start time
    int isTiming = 0;                     // Keeps track of whether the timer is running

    if (!isTiming)
    {
        startMillis = millis(); // Record the start time
        isTiming = 1;           // Start the timer
    }

    // Check if 1 minute (60000 milliseconds) has passed
    if (millis() - startMillis >= 60000)
    {
        isTiming = 0; // Reset the timer
        return 1;     // Indicate that 1 minute has passed
    }

    return 0; // Indicate that 1 minute has not passed yet
}

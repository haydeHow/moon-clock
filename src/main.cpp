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

void init_ssd1306();
void draw_vertical_split();

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

    // Display Setup
    init_ssd1306();
    draw_vertical_split();

    static char temp[8];
    static char time[6];
    static char moon_phase[10] = "";
    static char next_full[10] = "";
    static char date[20] = "";

    Serial.println("");
    // get temp
    // get_temp(temp);
    format_print_temp("80");

    // Serial.print("Temp: ");
    // Serial.println(temp);

    // get time
    get_time(time);
    format_print_time(time);

    // Serial.print("Time: ");
    // Serial.println(time);

    // get phase
    get_moon(moon_phase);
    // Serial.print("Phase: ");
    // Serial.println(moon_phase);
    format_print_moon_phase(moon_phase);
    format_print_moon_phase_picture();

    // get date
    get_date(date);
    format_print_date(date);
    // Serial.print("Date: ");
    // Serial.println(date);

    // get next full
    get_next_full(next_full);
    format_print_next_full(next_full);

    // Serial.print("Next Full: ");
    // Serial.println(next_full);
}

void loop()
{
    /*
    // Display the byte array
    display.drawBitmap(10, 10, waxing_gibbous, 50, 50, SSD1306_WHITE);

    // Display changes
    display.display();
    delay(2000);            // Delay to keep the display on
    display.clearDisplay(); // Clear the display for the next update
    */
}

// FINISHED
void get_lat_and_lon(float coords[2])
{
    // Make a request to ip-api.com
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;
        WiFiClient client;
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
        HTTPClient http;
        WiFiClient client;

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
        HTTPClient http;
        WiFiClient client;

        String url = "http://worldtimeapi.org/api/timezone/America/New_York";
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
        HTTPClient http;
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
        HTTPClient http;
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
        HTTPClient http;
        WiFiClient client;

        String url = "http://worldtimeapi.org/api/timezone/America/New_York";
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

void format_print_time(char *time)
{
    char format_time[6];
    int digits = 4;
    if (time[0] == '0')
    {
        if (time[0] == '0' && time[1] == '0')
        {
            format_time[0] = '1';
            format_time[1] = '2';
            const char *after_twelve = time + 2;
            strcat(format_time, after_twelve);
            format_time[5] = '\0';
        }
        else
        {
            const char *after_leading_zero = time + 1;
            strcpy(format_time, after_leading_zero);
            format_time[4] = '\0';
            digits = 3;
        }
    }

    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    if (digits == 3)
        display.setCursor(75, 1);
    else
        display.setCursor(65, 1);
    display.print(format_time);
    display.display();
}
void format_print_temp(char *temp)
{
    display.setCursor(1, 30);
    display.setTextSize(1);
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
    // TODO
    if (strcmp(phase, "WAX GIBB") == 0)
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
    char display_phase[20];

    const char *new_moon = "NEW MOON";
    const char *wax_gib = "WAX GIBB";
    const char *wax_cres = "WAX CRES";
    const char *first_quarter = "FIR QUAR";
    const char *second_quarter = "SEC QUAR";
    const char *wan_gib = "WAN GIBB";
    const char *wan_cres = "WAN CRES";
    const char *full_moon = "FULL MOON";

    if (strcmp(phase, "NEW MOON") == 0)
        strcpy(display_phase, "new_moon");
    else if (strcmp(phase, "WAX GIBB") == 0)
        strcpy(display_phase, "waxing_gibbous");
    else if (strcmp(phase, "WAX CRES") == 0)
        strcpy(display_phase, "waxing_crescent");
    else if (strcmp(phase, "FIR QUAR") == 0)
        strcpy(display_phase, "first_quarter");
    else if (strcmp(phase, "WAN GIBB") == 0)
        strcpy(display_phase, "waning_gibbous");
    else if (strcmp(phase, "WAX CRES") == 0)
        strcpy(display_phase, "waning_crescent");
    else if (strcmp(phase, "FULL MOON") == 0)
        strcpy(display_phase, "full_moon");
    else
        strcpy(display_phase, "ERROR");

    if (strcmp(display_phase, "ERROR") == 0)
    {
        Serial.println("error in format_print_moon_phase_picture()");
        return;
    }

    display.drawBitmap(72, 14, display_phase, 50, 50, SSD1306_WHITE);
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
}

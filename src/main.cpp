// #include <DNSServer.h>
// #include <WiFiManager.h> // Include WiFiManager library
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>

#include "faces.h"
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
void get_next_moon(char *next_phase);
void get_date(char *date);

void format_print_temp(char *temp);
void format_print_time(char *time);
void format_print_date(char *date);
void format_print_moon_phase(char *moon_phase);

void setup()
{
    // wire.begin();
    Serial.begin(9600);
    delay(5000);

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

    static char temp[] = "";
    static char time[] = "";
    static char moon_phase[] = "";
    static char next_full[] = "";
    static char date[] = "";

    Serial.println("");
    // get temp
    get_temp(temp);
    Serial.println(temp);
    // get time
    get_time(time);
    Serial.println(time);
    // get phase
    get_moon(moon_phase);
    Serial.println(moon_phase);
    // get date
    get_date(date);
    Serial.println(date);
    // get next
    get_next_moon(next_full); 
    Serial.println(next_full);

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

    // Serial.println(lat);
    // Serial.println(lon);
    // Serial.println("");
    // Serial.println("");

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

        /*
        String serverPath = "https://api.openweathermap.org/data/3.0/onecall?lat=" + lat + "&lon=" + lon +
        "&exclude=minutely,daily,hourly,alerts&appid=" + APIKEY + "&units=imperial";
        "https://api.openweathermap.org/data/3.0/onecall?lat=" + lat + "&lon=" + lon +
        "&exclude=minutely,daily,hourly,alerts&appid=" + APIKEY + "&units=imperial";
        */

        http.begin(client, API);
        int httpCode = http.GET();

        if (httpCode > 0)
        {
            String payload = http.getString();
            // Serial.println(payload);

            DynamicJsonDocument doc(1024);
            deserializeJson(doc, payload);

            float temperature = doc["current"]["temp"];
            char str_temp[20];
            sprintf(str_temp, "%.0f", temperature);
            strcpy(current_temp, str_temp);
        }
    http.end();
    }
    else
	    Serial.println("Error in get_temp FUNCTION");
}

// TODO
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

            int time_iter = 0;
            for (int i = 11; i < 16; ++i)
            {
		    // FIXME
                // forget the zero
                if (i == 11 && datetime[i] == '0')
                    continue;

                time[time_iter] = datetime[i];
                time_iter++;
            }
        }
    http.end();
    }
    else
	    Serial.println("Error in get_time FUNCTION");
}

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


	const char* url = "https://moon-phase.p.rapidapi.com/basic";

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


            strcpy(current_moon, current);
        }
        http.end();
    }
    else
	    Serial.println("Error in get_moon FUNCTION");
}


void get_next_moon(char *next_phase)
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


	const char* url = "https://moon-phase.p.rapidapi.com/basic";

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
	    char next_full_str[] = "";
	    sprintf(next_full_str, "%d days", next_full);

	    if (next_full == 0)
		    strcpy(next_phase, "CURRENT");
	    else

		    strcpy(next_phase, next_full_str);
        }
        http.end();
    }
    else 
	    Serial.println("Error in get_next_moon FUNCTION");
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
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(75, 1);
  display.println(time);
  display.display();
}


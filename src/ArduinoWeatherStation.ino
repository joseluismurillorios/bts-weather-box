#include <SPI.h>
#include <Wire.h>

//TFT display
#include "Adafruit_GFX.h"
#include <Adafruit_ST7735.h>
#include <Adafruit_ST7789.h>

//wifi module
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "WeatherData.h"
#include "TimeData.h"
#include "Secrets.h"

// Color definitions
#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0
#define WHITE    0xFFFF
#define GREY     0xC618

// Define pins that will be used as parameters in the TFT constructor
#define cs   5
#define dc   4
#define rst  16

WeatherData weatherData = WeatherData();
TimeData timeData = TimeData();
Secrets secrets = Secrets();
WiFiClient wifiClient;

String city = "Tijuana"; // add your prefered city
String units = "metric"; // (options: metric/imperial )

// Instantiate the TFT constructor with the pin values defined above
Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, rst);

long weatherDataTimer = 0;

void setup () {

  Serial.begin(115200);

  Wire.begin();

  // tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
  tft.initR(INITR_GREENTAB);   // initialize a ST7735S chip, black tab
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(30, 80);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.print("Connecting...");

  WiFi.begin(secrets.ssid, secrets.password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("Connecting..");
  }
  Serial.println("Connected..");

  tft.fillScreen(BLACK);
  tft.setCursor(36, 80);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.print("Connected");

  delay(1000);
  initWeather();
}

void loop() {
  if (millis() - weatherDataTimer > 60000) {
    initWeather();
    weatherDataTimer = millis();
  }
}

void initWeather() {
  if (WiFi.status() != WL_CONNECTED) {
    tft.fillScreen(BLACK);
    tft.setCursor(30, 80);
    tft.setTextColor(WHITE);
    tft.setTextSize(1);
    tft.print("Disconnected");
  } else {
    getWeatherData();
  }
}


void getWeatherData() {
  drawBTSLogo(50, 105, WHITE);
  String url = "http://api.openweathermap.org/data/2.5/weather?q=" + String(city) + "&units=" + units + "&APPID=" + secrets.apiKey;
  Serial.println(url);

  HTTPClient http;  //Declare an object of class HTTPClient
  http.begin(wifiClient, url);  //Specify request destination
  int httpCode = http.GET();//Send the request
  Serial.println(httpCode);
  if (httpCode > 0) { //Check t he returning code
    String payload = http.getString();   //Get the request response payload
    //parse data
    parseWeatherData(payload);
  } else {
    delay(5000);
    getWeatherData();
  }
  http.end();//Close connection
}

/**
 * Comverted json data using 
 * https://arduinojson.org/v5/assistant/
 */
void parseWeatherData(String payload) {
  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + 2 * JSON_OBJECT_SIZE(2) + 2 * JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(7) + JSON_OBJECT_SIZE(11) + 500;
  DynamicJsonBuffer jsonBuffer(capacity);

  JsonObject& root = jsonBuffer.parseObject(payload);

  float coord_lon = root["coord"]["lon"]; // 25.61
  float coord_lat = root["coord"]["lat"]; // 45.65

  JsonObject& weather_0 = root["weather"][0];
  int weather_0_id = weather_0["id"]; // 803
  const char* weather_0_main = weather_0["main"]; // "Clouds"
  const char* weather_0_description = weather_0["description"]; // "broken clouds"
  const char* weather_0_icon = weather_0["icon"]; // "04d"

  const char* base = root["base"]; // "stations"

  JsonObject& main = root["main"];
  float main_temp = main["temp"]; // -6.04
  // float main_pressure = main["pressure"]; // 1036.21
  // int main_humidity = main["humidity"]; // 65
  // float main_temp_min = main["temp_min"]; // -6.04
  // float main_temp_max = main["temp_max"]; // -6.04
  // float main_sea_level = main["sea_level"]; // 1036.21
  // float main_grnd_level = main["grnd_level"]; // 922.42

  // float wind_speed = root["wind"]["speed"]; // 1.21
  // float wind_deg = root["wind"]["deg"]; // 344.501

  // int clouds_all = root["clouds"]["all"]; // 68

  long dt = root["dt"]; // 1674828900
  long timezone = root["timezone"]; // -28800

  JsonObject& sys = root["sys"];
  // float sys_message = sys["message"]; // 0.0077
  // const char* sys_country = sys["country"]; // COUNTRY
  // long sys_sunrise = sys["sunrise"]; // 1550984672
  // long sys_sunset = sys["sunset"]; // 1551023855

  // long id = root["id"]; // 683844
  // const char* cityName = root["name"]; // CITY
  // int cod = root["cod"]; // 200

  // tft.fillScreen(BLACK);
  timeData.parseUnixTime(dt, timezone);

  // getCurrentTimeRequest(coord_lat, coord_lon);
  tft.fillScreen(BLACK);
  drawBTSLogo(50, 105, MAGENTA);

  displayTemperature(main_temp);
  displayIcon(weather_0_icon);
  displayDescription(weather_0_description);
  // displayLocation(cityName);
  displayCurrentTime();

  delay(5000);
}

void drawBTSLogo(uint16 x, uint16 y, uint16 color) {
  uint16 column1 = x;
  uint16 column2 = x+12;
  uint16 column3 = x+16;
  uint16 column4 = x+28;
  uint16 row1 = y;
  uint16 row2 = y+12;
  uint16 row3 = y+39;
  uint16 row4 = y+44;

  tft.drawLine(column1, row1, column1, row4, color);
  tft.drawLine(column2, row2, column2, row3, color);

  tft.drawLine(column1, row1, column2, row2, color);
  tft.drawLine(column1, row4, column2, row3, color);

  tft.drawLine(column3, row2, column3, row3, color);
  tft.drawLine(column4, row1, column4, row4, color);

  tft.drawLine(column3, row2, column4, row1, color);
  tft.drawLine(column3, row3, column4, row4, color);

  // Bold
  tft.drawLine(column1+1, row1+1, column1+1, row4-1, color);
  tft.drawLine(column2-1, row2+1, column2-1, row3-1, color);

  tft.drawLine(column1+1, row1+2, column2-1, row2, color);
  tft.drawLine(column1+1, row4-1, column2-1, row3-1, color);

  tft.drawLine(column3+1, row2+1, column3+1, row3-1, color);
  tft.drawLine(column4-1, row1+1, column4-1, row4-1, color);

  tft.drawLine(column3+1, row2, column4-1, row1+2, color);
  tft.drawLine(column3+1, row3, column4-1, row4-1, color);
}

// TIME
void displayCurrentTime() {
  tft.setTextSize(3);
  tft.setCursor(2, 66);

  tft.setTextColor(WHITE);
  tft.print(timeData.timeOnly);

  tft.setTextColor(GREY);

  tft.setTextSize(1);
  tft.setCursor(112, 73);
  tft.print(timeData.ampm);

  tft.setTextSize(2);
  tft.setCursor(88, 110);
  tft.print(timeData.monthOnly);

  tft.setTextSize(2);
  tft.setCursor(8, 110);
  tft.print(timeData.day);

  tft.setTextSize(2);
  tft.setCursor(92, 130);
  tft.print(timeData.yearOnly);

  tft.setTextSize(2);
  tft.setCursor(12, 130);
  tft.print(timeData.dayOnly);
}

// TEMPERATURE
void displayTemperature(float main_temp) {
  tft.setTextColor(GREY);
  tft.setTextSize(2);

  tft.setCursor(8, 14);
  // String temperatureValue = String((int)main_temp) + (char)247 + "C";
  String temperatureValue = String((int)main_temp) + "C";
  tft.print(temperatureValue);
}

// ICON
void displayIcon(String weatherIcon) {
  tft.setTextSize(1);
  tft.drawBitmap(75, 5, weatherData.GetIcon(weatherIcon) , 50, 50, MAGENTA);
}

// DESCRIPTION
void displayDescription(String weatherDescription) {
  tft.setTextSize(1);
  tft.setCursor(8, 38);
  if(weatherDescription.length() > 18){
   weatherDescription = weatherDescription.substring(0, 15) + "...";
  }
  String description = String(weatherDescription);
  for (auto & c: description) c = (char)toupper(c);
  tft.print(description);
}

// SUNRISE
void displaySunriseTime(String sys_sunrise) {
  tft.setTextSize(1);
  tft.setCursor(5, 105);

  String timeOnly = sys_sunrise.substring(11);
  String sunrise = "SUNRISE: " + timeOnly;
  tft.print(sunrise);
}


// SUNSET
void displaySunsetTime(String sys_sunset) {
  tft.setTextSize(1);
  tft.setCursor(5, 115);

  String timeOnly = sys_sunset.substring(11);
  String sunset = "SUNSET: " + timeOnly;
  tft.print(sunset);
}

// LOCATION
void displayLocation(String cityName) {
  tft.setTextSize(1);
  tft.setCursor(5, 140);
  String loc = "City: " + String(cityName);
  for (auto & c: loc) c = (char)toupper(c);
  tft.print(loc);
}

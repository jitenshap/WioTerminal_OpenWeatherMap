#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <LovyanGFX.hpp>
#include "src/types.h"
#include "src/certificate.h"

const char* ssid     = "YOUR_SSID";     // your network SSID (name of wifi network)
const char* password = "YOUR_WIFI_PWD"; // your network password
char* api_key = "YOUR_OWM_API_KEY";
char* city = "YOUR_CITY";

const char* apiserver = "api.openweathermap.org";
const char* iconserver = "openweathermap.org";


WiFiClientSecure client;
static LGFX lcd;
static LGFX_Sprite sprite(&lcd);

void get_icon(char* icon)
{ 
  Serial.println("\nStarting connection to server...");
  if (!client.connect(iconserver, 443)) 
  {
    Serial.println("Connection failed!");
  } 
  else 
  {
    Serial.println("Connected to server!");
    // Make a HTTP request:
    char reqBuf[128];
    sprintf(reqBuf, "GET https://openweathermap.org/img/w/%s.png HTTP/1.1", icon);
    Serial.println(reqBuf);
    client.println(reqBuf);
    client.println("Host: openweathermap.org");
    client.println("Connection: close");
    client.println();
    int len = 0;
    while (client.connected()) 
    {
      String line = client.readStringUntil('\n');
      int pos = line.indexOf("Content-Length: ");
      if(pos > -1)
      {
        pos += 16;
        len = line.substring(pos).toInt();
      }
      if (line == "\r") 
      {
        Serial.println("headers received");
        break;
      }
    }
    Serial.println("Icon len = " + (String)len);
    byte icon[len];
    int icon_len = 0;
    Serial.println();
    while (client.available()) 
    {
      icon[icon_len] = (byte)client.read();
      //Serial.write(icon[icon_len]);
      icon_len ++;
      //Serial.write(client.read());
    }
    icon_len --;
    client.stop();
    Serial.println("Icon download complete. len = " + (String)icon_len);
    lcd.drawPng(icon, icon_len, 0, 50, 50, 50 ,0, 0, 1.0);
  }
}

weather_t get_weather()
{
  weather_t weather;
  weather.icon[0] = '\0';
  weather.stat[0] = '\0';
  weather.temp = -173;
  weather.humidity = -100;  

  StaticJsonBuffer<2000> jsonBuffer;
  Serial.println("\nStarting connection to server...");
  if (!client.connect(apiserver, 443)) 
  {
    Serial.println("Connection failed!");
    return weather;
  } 
  else 
  {
    Serial.println("Connected to server!");
    // Make a HTTP request:
    char tmp[19];
    sprintf(tmp, "GET https://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s HTTP/1.1", city, api_key);
    Serial.print("Request: ");
    Serial.println(tmp);
    client.println(tmp);
    client.println("Host: api.openweathermap.org");
    client.println("Connection: close");
    client.println();
    String headers = "";
    while (client.connected()) 
    {
      String line = client.readStringUntil('\n');
      headers += line + "\n";
      if (line == "\r") 
      {
        Serial.println("Headers received");
        Serial.println(headers);
        break;
      }
    }
    char body[1024];
    char* body_p = body;
    while (client.available()) 
    {
      *body_p = (char)client.read();
      body_p ++;
    }
    client.stop();
    body_p --;
    *body_p = '\0';
    Serial.println(body);
    JsonObject& root = jsonBuffer.parseObject(body);
    if (!root.success()) 
    {
      Serial.println("parseObject() failed");
      return weather;
    }
    else
    {
      strcpy(weather.icon, root["weather"][0]["icon"]);
      strcpy(weather.stat, root["weather"][0]["main"]);
      weather.temp = ((float)root["main"]["temp"] - 273.15);
      weather.humidity = (int)(root["main"]["humidity"]);
      return weather;
    }
  }
}



void setup() 
{
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  lcd.init();
  lcd.setRotation(1);
  lcd.setBrightness(255);
  lcd.setColorDepth(24);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Weather info");
  delay(100);

  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print(".");
    // wait 1 second for re-trying
    delay(1000);
  }

  Serial.print("Connected to ");
  Serial.println(ssid);
  
  client.setCACert(test_root_ca);
  weather_t weather = get_weather();
  Serial.print("weather = ");
  Serial.println(weather.stat);
  Serial.print("icon = ");
  Serial.println(weather.icon);
  Serial.println("temp = " + (String)weather.temp);
  Serial.println("humidity = " + (String)weather.humidity);
  lcd.setCursor(0, 20);
  lcd.print("Weather: ");
  lcd.println(weather.stat);
  lcd.print("Temperature: ");
  lcd.println(weather.temp);
  lcd.print("Humidity: ");
  lcd.println(weather.humidity);
  get_icon(weather.icon);
  Serial.println("Done");
}

void loop() 
{
  // do nothing
}

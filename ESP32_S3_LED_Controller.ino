#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include "secrets.h"

#define LED_PIN            48   // onboard WS2812 data pin on most ESP32-S3 devkits
#define LED_COUNT          1
#define POLL_INTERVAL_MS   5000

Adafruit_NeoPixel pixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

unsigned long lastPoll = 0;

void connectWiFi() 
{
  Serial.printf("Connecting to %s", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
}

// Fetches /led.json and updates the pixel. Returns false on any network,
// HTTP, or JSON error (pixel is left as-is).
bool updateLedFromFirebase() 
{
  WiFiClientSecure client;
  client.setInsecure(); // skips cert validation - fine for a hobby project

  HTTPClient http;
  http.begin(client, FIREBASE_URL);

  int status = http.GET();
  if (status != HTTP_CODE_OK) 
  {
    Serial.printf("[FIREBASE] HTTP error: %d\n", status);
    http.end();
    return false;
  }

  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, http.getStream());
  http.end();

  if (err) 
  {
    Serial.printf("[FIREBASE] JSON parse failed: %s\n", err.c_str());
    return false;
  }

  bool power = doc["power"] | false;
  int red = doc["red"] | 0;
  int green = doc["green"] | 0;
  int blue = doc["blue"] | 0;
  int brightness = doc["brightness"] | 100; // 0-100 (%)

  red = constrain(red, 0, 255);
  green = constrain(green, 0, 255);
  blue = constrain(blue, 0, 255);
  brightness = constrain(brightness, 0, 100);

  if (power) 
  {
    float scale = brightness / 100.0f;
    pixel.setPixelColor(0, pixel.Color(red * scale, green * scale, blue * scale));
  } 
  else
    pixel.setPixelColor(0, 0);

  pixel.show();
  Serial.printf("Power: %s | RGB: (%d, %d, %d) | Brightness: %d%%\n", power ? "ON" : "OFF", red, green, blue, brightness);

  return true;
}

void setup() 
{
  Serial.begin(115200);
  delay(300);

  pixel.begin();
  pixel.show(); // start off

  connectWiFi();
}

void loop() 
{
  if (millis() - lastPoll >= POLL_INTERVAL_MS) 
  {
    lastPoll = millis();

    if (!updateLedFromFirebase()) 
      Serial.println("Request failed, will retry next poll");
  }
}

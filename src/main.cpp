#include <map>
#include <Arduino.h>

#ifdef ESP32
#include <WiFi.h>
#endif

#define FASTLED_ESP8266_DMA
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>
#include <iostream>

#include "SinricPro.h"
#include "SinricProLight.h"

#include "colortables.h"
#include "sensitiveData.h"

// This is defined in the sensitiveData.h header file in order to keep the data private; you can do the same,
// or just copy and paste into your code. and uncomment the next five lines.
#ifndef WIFI_SSID
#define WIFI_SSID yourwifiSSID // SSID
#endif

#ifndef WIFI_PASS
#define WIFI_PASS yourwifiPASS // Password
#endif

#ifndef APP_KEY
#define APP_KEY yourAppKey // Should look like "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx"
#endif

#ifndef APP_SECRET
#define APP_SECRET yourAppSecret // Should look like "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"
#endif

#ifndef LIGHT_ID
#define LIGHT_ID yourLightID // Should look like "5dc1564130xxxxxxxxxxxxxx"
#endif

#define BAUD_RATE 9600 // Change baudrate to your need
#define NUM_LEDS 65    // how many LEDs are on the stripe
#define LED_PIN 2      // LED strip is connected to PIN 2

#define MAX_BRIGHTNESS 255
#define VOLTAGE 5          //[V] Power Supply Voltage
#define CURRENT_LIMIT 2000 //[mA] Current Limit for power supply; going over the rated limit will risk your supply

bool powerState;
int globalBrightness = 100;
CRGB leds[NUM_LEDS];

struct colors
{
  byte r, g, b;
};

colors colorTemperatures(int &colorTemperature)
{
  byte r = r_table[colorTemperature];
  byte g = g_table[colorTemperature];
  byte b = b_table[colorTemperature];
  return colors{r, g, b};
}

bool onPowerState(const String &deviceId, bool &state)
{
  powerState = state;
  if (state)
  {
    FastLED.setBrightness(map(globalBrightness, 0, 100, 0, MAX_BRIGHTNESS));
  }
  else
  {
    FastLED.setBrightness(0);
  }
  FastLED.show();
  Serial.printf("Power state changed to %s\n", state ? "on" : "off");
  return true; // request handled properly
}

bool onBrightness(const String &deviceId, int &brightness)
{
  globalBrightness = brightness;
  FastLED.setBrightness(map(brightness, 0, 100, 0, MAX_BRIGHTNESS));
  FastLED.show();
  Serial.printf("Brightness changed to %d\n", globalBrightness);
  return true;
}

bool onAdjustBrightness(const String &deviceId, int brightnessDelta)
{
  globalBrightness += brightnessDelta;
  brightnessDelta = globalBrightness;
  FastLED.setBrightness(map(globalBrightness, 0, 100, 0, MAX_BRIGHTNESS));
  FastLED.show();
  Serial.printf("Brightness changed to %d\n", globalBrightness);
  return true;
}

bool onColor(const String &deviceId, byte &r, byte &g, byte &b)
{
  fill_solid(leds, NUM_LEDS, CRGB(g, r, b));
  FastLED.show();
  Serial.printf("Color changed to GRB(%d, %d, %d)\n", g, r, b);
  return true;
}

bool onColorTemperature(const String &deviceId, int &colorTemperature)
{
  colors color = colorTemperatures(colorTemperature);
  fill_solid(leds, NUM_LEDS, CRGB(color.g, color.r, color.b));
  FastLED.show();
  return true;
}

void setupFastLED()
{
  FastLED.addLeds<WS2812B, LED_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(map(globalBrightness, 0, 100, 0, MAX_BRIGHTNESS));
  FastLED.setMaxPowerInVoltsAndMilliamps(VOLTAGE, CURRENT_LIMIT);

  // Set StartUp Color to be Warm White
  CRGB default_color = CRGB(0, 0, 0);
  fill_solid(leds, NUM_LEDS, default_color);
  FastLED.show();
}

void setupWiFi()
{
  Serial.printf("\r\n[Wifi]: Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.printf(".");
    delay(250);
  }
  IPAddress localIP = WiFi.localIP();
  Serial.printf("connected!\r\n[WiFi]: IP-Address is %s\r\n", localIP.toString().c_str());
}

void setupSinricPro()
{
  // get a new Light device from SinricPro
  SinricProLight &myLight = SinricPro[LIGHT_ID];

  // set callback function to device
  myLight.onPowerState(onPowerState);
  myLight.onBrightness(onBrightness);
  myLight.onAdjustBrightness(onAdjustBrightness);
  myLight.onColor(onColor);
  myLight.onColorTemperature(onColorTemperature);

  // setup SinricPro
  SinricPro.onConnected([]()
                        { Serial.printf("Connected to SinricPro\r\n"); });
  SinricPro.onDisconnected([]()
                           { Serial.printf("Disconnected from SinricPro\r\n"); });
  SinricPro.restoreDeviceStates(true);
  SinricPro.begin(APP_KEY, APP_SECRET);
}

// main setup function
void setup()
{
  Serial.begin(BAUD_RATE);
  Serial.printf("\r\n\r\n");
  setupFastLED();
  setupWiFi();
  setupSinricPro();
}

void loop()
{
  SinricPro.handle();
}
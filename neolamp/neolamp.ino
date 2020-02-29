/*
   Copyright (C) 2020 Michael Matthews

   This file is part of NeoLamp.

   NeoLamp is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   NeoLamp is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with NeoLamp.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "neolamp.h"

#include "Adafruit_NeoPixel.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

ESP8266WiFiMulti wifiMulti;

const char fingerprint[] PROGMEM = FINGERPRINT;
const char* host = "io.adafruit.com";
const int httpsPort = 443;

// 4 of the 8 pixel strips chained together is 32 pixels.
Adafruit_NeoPixel strip = {Adafruit_NeoPixel(32, D1, NEO_GRBW + NEO_KHZ800)};

typedef enum
{
 NoStyle=0,
 Normal =1,
 Rainbow=2,
 Cycle  =3,
 Red    =4,
 Yellow =5,
 Green  =6,
 Cyan   =7,
 Blue   =8,
 Magenta=9
} styles;
uint8_t currentStyle=NoStyle;
uint8_t requiredStyle=Normal;

uint8_t currentBrightness=0;
uint8_t requiredBrightness=0;

uint16_t counter=0;
int firstPixelHue=0;

static const char *wifiuserpass[] = { WIFI_USERPASS };

void setup() {
  Serial.begin(115200);

  strip.begin(); // Initialise strip.
  strip.show(); // Turn off all pixels.

  // Reduce overall brightness to prevent drawing too much power.  The lamp would also be too bright.
  // Reference: https://learn.adafruit.com/adafruit-neopixel-uberguide/powering-neopixels
  strip.setBrightness(70);

  WiFi.mode(WIFI_STA);

  // Add all known access points for WiFi scanning.
  uint8_t i=0;
  while(wifiuserpass[i]!=NULL)
  {
    wifiMulti.addAP(wifiuserpass[i], wifiuserpass[i+1]);
    i+=2;
  }

  // Wait for WiFi connection.
  Serial.println("Connecting Wifi...");
  while(wifiMulti.run() != WL_CONNECTED)
  {
    // Cycle the bottom pixel around the lamp in green to give a visual
    // indication that WiFi scanning is in progress.
    for(uint8_t p=0; p<4; p++)
    {
      for(uint8_t s=0; s<4; s++)
      {
        strip.setPixelColor(s*8, s==p?strip.Color(0, 255, 0, 0):strip.Color(0, 0, 0, 0));
        strip.show();
      }
      delay(50);
      strip.setPixelColor(24, strip.Color(0, 0, 0, 0));
      strip.show();
    }
  }

  // WiFi is now connected so the main loop() can now start.
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (counter++==0)
    checkCloud();

  if(currentStyle!=requiredStyle || currentBrightness!=requiredBrightness || currentStyle==Rainbow || currentStyle==Cycle)
  {
    currentStyle=requiredStyle;

    if(currentBrightness>requiredBrightness)
      currentBrightness--;
    else if(currentBrightness<requiredBrightness)
      currentBrightness++;

    if(currentStyle==Normal)
      setStrip(&strip, strip.Color(0, 0, 0, currentBrightness));
    else if(currentStyle==Rainbow)
      rainbow();
    else if(currentStyle==Cycle)
      cycle();
    else if(currentStyle==Red)
      setStrip(&strip, strip.Color(255*currentBrightness/255, 0, 0, 0));
    else if(currentStyle==Yellow)
      setStrip(&strip, strip.Color(180*currentBrightness/180, 180*currentBrightness/180, 0, 0));
    else if(currentStyle==Green)
      setStrip(&strip, strip.Color(0, 255*currentBrightness/255, 0, 0));
    else if(currentStyle==Cyan)
      setStrip(&strip, strip.Color(0, 180*currentBrightness/180, 180*currentBrightness/180, 0));
    else if(currentStyle==Blue)
      setStrip(&strip, strip.Color(0, 0, 255*currentBrightness/255, 0));
    else if(currentStyle==Magenta)
      setStrip(&strip, strip.Color(180*currentBrightness/180, 0, 180*currentBrightness/180, 0));

    strip.show();
  }

  if (counter>500)
    counter=0;

  delay(12);
}

void checkCloud() {
  HTTPClient http;

  // start request with URL and TLS cert fingerprint for verification
  http.begin("https://" + String(host) + String("/api/v2/") + String(ADAFRUIT_ACCOUNT) + String("/feeds/") + String(ADAFRUIT_FEED) + String("/data/retain"), fingerprint);

  // IO API authentication
  http.addHeader("X-AIO-Key", ADAFRUIT_KEY);
  http.addHeader("Connection", "close");

  // start connection and send HTTP header
  int httpCode = http.GET();

  // httpCode will be negative on error
  if(httpCode > 0)
  {
    // HTTP 200 OK
    if(httpCode == HTTP_CODE_OK)
    {
      String payload = http.getString();
      int16_t light=payload.toInt();

      uint8_t style=light/1000;
      if(style>0 && style<=Magenta)
        requiredStyle=style;
      requiredBrightness=min(max(0, light%1000),255);
    }

    http.end();
  }
  else
    Serial.printf("[HTTP] GET request error!\n");
}

void setStrip(Adafruit_NeoPixel *strip, uint32_t color)
{
  for(uint16_t i=0; i<strip->numPixels(); i++)
    strip->setPixelColor(i, color);
  strip->show();
}

// Rainbow the whole strip.
void rainbow()
{
  if(firstPixelHue>65536)
    firstPixelHue=0;

  for(uint8_t i=0; i<strip.numPixels(); i++)
  {
    int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
    strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue, 255, currentBrightness)));
  }
  firstPixelHue+=256;
}

// Rainbow cycle around the lamp.
void cycle()
{
  if(firstPixelHue>65536)
    firstPixelHue=0;

  for(uint8_t s=0; s<strip.numPixels()/8; s++)
  {
    int pixelHue = firstPixelHue + (s * 65536L / strip.numPixels());
    for(uint8_t p=0; p<8; p++)
      strip.setPixelColor(s*8+p, strip.gamma32(strip.ColorHSV(pixelHue, 255, currentBrightness)));
  }
  firstPixelHue+=256;
}

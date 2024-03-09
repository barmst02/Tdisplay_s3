#include <Arduino.h>
#include "TinyGPSPlus.h"
#include "TFT_eSPI.h" /* Please use the TFT library provided in the library. */
#include "pin_config.h"

/*
   This sample code demonstrates the normal use of a TinyGPSPlus (TinyGPSPlus) object.
*/
static const uint32_t GPSBaud = 9600;

// The TinyGPSPlus object
TinyGPSPlus gps;

TFT_eSPI tft = TFT_eSPI();

void setup()
{
  Serial.begin(115200);
  Serial0.begin(GPSBaud);

  tft.begin();
  tft.setRotation(3);
  tft.setTextSize(2);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  //find the width of one character
  int charWidth = tft.drawString("0", 0, 150, 8);

  Serial.print("Height: ");
  Serial.print(tft.getViewportHeight());
  Serial.println();
  Serial.print("Width: ");
  Serial.print(tft.getViewportWidth());
  Serial.println();

  Serial.println();
  Serial.println(F("Sats HDOP  Latitude   Longitude   Fix  Date       Time     Date Alt    Course Speed Card  Distance Course Card  Chars Sentences Checksum"));
  Serial.println(F("           (deg)      (deg)       Age                      Age  (m)    --- from GPS ----  ---- to London  ----  RX    RX        Fail"));
  Serial.println(F("----------------------------------------------------------------------------------------------------------------------------------------"));
}

// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (Serial0.available())
      gps.encode(Serial0.read());
  } while (millis() - start < ms);
}

static void printFloat(float val, bool valid, int len, int prec)
{
  if (!valid)
  {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  }
  else
  {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      Serial.print(' ');
  }
  smartDelay(0);
}

static void printInt(unsigned long val, bool valid, int len)
{
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0) 
    sz[len-1] = ' ';
  Serial.print(sz);
  smartDelay(0);
}

static void printDateTime(TinyGPSDate &d, TinyGPSTime &t)
{
  if (!d.isValid())
  {
    Serial.print(F("********** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
    Serial.print(sz);
  }
  
  if (!t.isValid())
  {
    Serial.print(F("******** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
    Serial.print(sz);
  }

  printInt(d.age(), d.isValid(), 5);
  smartDelay(0);
}

static void printStr(const char *str, int len)
{
  int slen = strlen(str);
  for (int i=0; i<len; ++i)
    Serial.print(i<slen ? str[i] : ' ');
  smartDelay(0);
}

void loop()
{
  //Time
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  char tm[32];
  TinyGPSTime t = gps.time;
    if (!t.isValid())
  {
    sprintf(tm, "******** ");
  }
  else
  {
    sprintf(tm, "%02d:%02d:%02d ", gps.time.hour(), gps.time.minute(), gps.time.second());  
  }
  tft.drawString(tm, 0, 140, 2);
  
  //Speed
  tft.setTextColor(TFT_RED, TFT_BLACK);
  char mph[3];
  if (gps.speed.isValid()) {
    int speed = floor(gps.speed.mph());
    sprintf(mph, "%d", speed);
  }
  else {
    sprintf(mph, "%d", gps.satellites.value());
  }
  int x=(3-strlen(mph)) * 75 + 10;
  tft.drawString(mph,x,0,8);
  
  //Course
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  //tft.drawString(TinyGPSPlus::cardinal(gps.course.deg()),0,0,6);
  tft.drawString((gps.course.isValid() ? TinyGPSPlus::cardinal(gps.course.deg()) : "*** "), 0, 0, 2);
  //tft.drawFloat(gps.course.deg(),0,0,0,2);
  
  printInt(gps.satellites.value(), gps.satellites.isValid(), 5);
  printFloat(gps.hdop.hdop(), gps.hdop.isValid(), 6, 1);
  printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
  printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);
  printInt(gps.location.age(), gps.location.isValid(), 5);
  printDateTime(gps.date, gps.time);
  printFloat(gps.altitude.meters(), gps.altitude.isValid(), 7, 2);
  printFloat(gps.course.deg(), gps.course.isValid(), 7, 2);
  printFloat(gps.speed.kmph(), gps.speed.isValid(), 6, 2);
  printStr(gps.course.isValid() ? TinyGPSPlus::cardinal(gps.course.deg()) : "*** ", 6);

  printInt(gps.charsProcessed(), true, 6);
  printInt(gps.sentencesWithFix(), true, 10);
  printInt(gps.failedChecksum(), true, 9);
  Serial.println();
  
  smartDelay(1000);

  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));
}
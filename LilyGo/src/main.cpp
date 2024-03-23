#include <Arduino.h>
#include "TinyGPSPlus.h"
#include "TFT_eSPI.h" /* Please use the TFT library provided in the library. */
#include "pin_config.h"

static const uint32_t GPSBaud = 9600;

// The TinyGPSPlus object
TinyGPSPlus gps;

TFT_eSPI tft = TFT_eSPI();

static int dHeight = 170;
static int dWidth = 320;

static int maxSat = 12;
char mphOld[4];
int brightness;
int speed;

static int battCHG = 80;

#include "OneButton.h"

// Setup a new OneButton on pin A1.  
OneButton button1(PIN_BUTTON_2, true);
// Setup a new OneButton on pin A2.  
OneButton button2(PIN_BUTTON_1, true);


// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    button1.tick();
    button2.tick();
    while (Serial0.available())
      gps.encode(Serial0.read());
  } while (millis() - start < ms);
}

// This function will be called when the button1 was pressed 1 time (and no 2. button press followed).
void click1() {
  brightness = ledcRead(0);
  brightness += battCHG;
  if (brightness >250) brightness = 250;
  ledcWrite(0, brightness);
  Serial.println("Button 1 click.");
  Serial.printf("Brightness: %i", brightness);
} // click1

// This function will be called when the button1 was pressed 1 time (and no 2. button press followed).
void click2() {
  brightness = ledcRead(0);
  brightness -= battCHG;
  if (brightness <10) brightness = 10;
  ledcWrite(0, brightness);
  Serial.println("Button 2 click.");
  Serial.printf("Brightness: %i", brightness);
} // click2

void longPressStart1() {
  ledcWrite(0, 250);
}

void longPressStart2(){
  ledcWrite(0, 10);
}

void setup()
{
  Serial.begin(115200);
  Serial0.begin(GPSBaud);

  tft.begin();
  tft.setRotation(1);
  tft.setTextSize(2);
  tft.fillScreen(TFT_BLACK);
  //tft.setTextColor(TFT_RED, TFT_BLACK);
  //find the width of one character
  //Serial.print(tft.getViewportHeight());
  //Serial.print(tft.getViewportWidth());
  
  // Turn on backlight
  ledcSetup(0, 2000, 8);
  ledcAttachPin(PIN_LCD_BL, 0);
  ledcWrite(0, 170);
  
  // link the button 1 functions.
  button1.attachClick(click1);
  //button1.attachDoubleClick(doubleclick1);
  button1.attachLongPressStart(longPressStart1);
  //button1.attachLongPressStop(longPressStop1);
  //button1.attachDuringLongPress(longclick1);

  // link the button 2 functions.
  button2.attachClick(click2);
  //button2.attachDoubleClick(doubleclick2);
  button2.attachLongPressStart(longPressStart2);
  //button2.attachLongPressStop(longPressStop2);
  //button2.attachDuringLongPress(longclick2);

  Serial.println("Initial delay...");
  smartDelay(1000);
}

void loop()
{
  Serial.println("Starting loop...");
  int size;
  int x;
  char mph[4];

  //Serial.printf("isValid: %s", gps.satellites.isValid() ? "true" : "false"); Serial.println();
  //Serial.printf("isUpdated: %s", gps.satellites.isUpdated() ? "true" : "false"); Serial.println();
  Serial.printf("Satellites: %d", gps.satellites.value()); Serial.println();
  //tft.printf("Satellites: %d", gps.satellites.value()); 
  Serial.printf("HDOP: %d", gps.hdop.value()); Serial.println();
  Serial.println();
  
  Serial.println("SATELLITES--------------------------");
  int numSat = 0; 

  if (gps.satellites.isValid())
  {
    numSat = gps.satellites.value();
    if (numSat>maxSat) 
      numSat=maxSat;
  }
  char strSat[maxSat+1];
  sprintf(strSat, "%.*s", maxSat, "                                             ");  //string of maxSat spaces
  for (int i=1; i<=maxSat-numSat; ++i)
    {
      strSat[i-1] = '=';
    }
  
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawString(strSat, 0, 140, 2);
  Serial.println("END SATELLITES--------------------------");
  

  Serial.println("SPEED--------------------------");
   
  Serial.printf("Speed Valid: %d", gps.speed.isValid() ? floor(gps.speed.mph()): -1); Serial.println();
  
  if (gps.speed.isValid()) {
    speed = floor(gps.speed.mph());
    //---------------------------------
    //speed = random(20); 
    //Serial.printf("Speed Random: %d", speed); Serial.println();
  }
  else {
    speed = floor(gps.speed.mph());
  }
  sprintf(mph, "--");  
  
  if (mph != mphOld) {
    //display the prior value in black to "erase" it
    Serial.println(mphOld);
    tft.setTextColor(TFT_BLACK, TFT_BLACK);
    x=(3-strlen(mphOld)) * 110;   // each character is 110 pixels
    size = tft.drawString(mphOld,x,0,8);
  
    //display the new value
    Serial.println(mph);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    x=(3-strlen(mph)) * 110;   // each character is 110 pixels
    size = tft.drawString(mph,x,0,8);
  
    sprintf(mphOld, "%d", speed); 
  }
  Serial.println("END SPEED--------------------------");

  Serial.println("COURSE --------------------------");
  
  char strCard[4] = "???";

  Serial.printf("Cardinal Dir: %s", gps.course.isValid() ? TinyGPSPlus::cardinal(gps.course.deg()) : "???"); Serial.println();

  if (gps.course.isValid())
    sprintf(strCard, "%s     ", TinyGPSPlus::cardinal(gps.course.deg()));

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  size = tft.drawString(strCard, 0, 0, 2);  // each character is 28 pixels
  //Serial.println(size);
 Serial.println("END COURSE --------------------------");
  

  smartDelay(2000);
  Serial.println("Looping...");

  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));
}


/*TFT_BLACK       0x0000
TFT_NAVY        0x000F
TFT_DARKGREEN   0x03E0
TFT_DARKCYAN    0x03EF
TFT_MAROON      0x7800
TFT_PURPLE      0x780F
TFT_OLIVE       0x7BE0
TFT_LIGHTGREY   0xC618
TFT_DARKGREY    0x7BEF
TFT_BLUE        0x001F
TFT_GREEN       0x07E0
TFT_CYAN        0x07FF
TFT_RED         0xF800
TFT_MAGENTA     0xF81F
TFT_YELLOW      0xFFE0
TFT_WHITE       0xFFFF
TFT_ORANGE      0xFDA0
TFT_GREENYELLOW 0xB7E0
TFT_PINK        0xFC9F
*/
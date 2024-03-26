#include <Arduino.h>
#include "pin_config.h"
#include "TinyGPSPlus.h" //GPS library
#include "TFT_eSPI.h"    //Display library /* Please use the TFT library provided in the library. */
#include "OneButton.h"   //Button library

// Create objects
TinyGPSPlus gps;
TFT_eSPI tft = TFT_eSPI();
OneButton button1(PIN_BUTTON_2, true);
OneButton button2(PIN_BUTTON_1, true);

// GPS 
static const uint32_t GPSBaud = 9600;
static const unsigned long gpsRefresh = 1000;

// Static values
static int dHeight = 170;
static int dWidth = 320;
static int maxSat = 12;
static int battCHG = 80;

// Display variables
int brightness;
int size;
int x;

// Satellite variables
int numSat;

// Speed variables
int speedTarget;
int speedDisplay = 0;
char mph[4];
int steps;
unsigned long stepDelay;

// Direction variables
char strCard[4];

// Misc variables
int i;
unsigned long getRandom = gpsRefresh;

// This custom version of delay() ensures that the gps object is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    button1.tick(); // Monitor for a click
    button2.tick(); // Monitor for a click
    while (Serial0.available())
      gps.encode(Serial0.read());
  } while (millis() - start < ms);
}

// This function will be called when the button1 was pressed 1 time (and no 2. button press followed).
void click1()
{
  brightness = ledcRead(0); // Read the current brightness
  brightness += battCHG;    // Increase by battCHG
  if (brightness > 250)
    brightness = 250;       // Don't go brighter than 250
  ledcWrite(0, brightness); // Write the new value
  Serial.printf("Button 1 click. Increase brightness to %i\n", brightness);
} // click1

// This function will be called when the button1 was pressed 1 time (and no 2. button press followed).
void click2()
{
  brightness = ledcRead(0); // Read the current brightness
  brightness -= battCHG;    // Decrease by battCHG
  if (brightness < 10)
    brightness = 10;        // Don't go darker than 10
  ledcWrite(0, brightness); // Write the new value
  Serial.printf("Button 2 click. Decrease brightness to %i\n", brightness);
} // click2

void longPressStart1()
{
  ledcWrite(0, 250); // Set the brightness to max
}

void longPressStart2()
{
  ledcWrite(0, 10); // Set the brightness to min
}

void setup()
{
  // Establish monitoring port
  Serial.begin(115200);

  // Establish communications with GPS receiver
  Serial0.begin(GPSBaud);

  // Set up display
  tft.begin();
  tft.setRotation(1);
  tft.setTextSize(2);
  tft.fillScreen(TFT_BLACK);
  ledcSetup(0, 2000, 8);

  // Turn on backlight
  ledcAttachPin(PIN_LCD_BL, 0);
  ledcWrite(0, 170);

  // link the button functions
  button1.attachClick(click1);
  button1.attachLongPressStart(longPressStart1);
  button2.attachClick(click2);
  button2.attachLongPressStart(longPressStart2);

  getRandom = millis(); //Set initial time for random value
}

void loop()
{
  //--------------------------------------
  Serial.println("--SATELLITES--");

  if (gps.satellites.isValid())
  {
    numSat = gps.satellites.value();
    Serial.printf("Satellites found: %d \n", numSat);
    if (numSat > maxSat)
      numSat = maxSat;
  }
  else
  {
    numSat = 0;
    Serial.printf("No satellites found \n");
  }
  char strSat[maxSat + 1];
  sprintf(strSat, "%.*s", maxSat, "                                             "); // string of maxSat spaces
  for (int i = 1; i <= maxSat - numSat; ++i)
  {
    strSat[i - 1] = '=';
  }

  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawString(strSat, 0, 140, 2);

  //--------------------------------------
  Serial.println("--SPEED--");

  if (gps.speed.isValid() and gps.speed.mph() < 5)
  {
    // Round down if speed <5 MPH. Make sure we get a consistent zero
    speedTarget = floor(gps.speed.mph());
    Serial.printf("Floor: speedTarget %d \n", speedTarget);
  }
  else if (gps.speed.isValid())
  {
    // Round to the nearest value.
    speedTarget = round(gps.speed.mph());
    Serial.printf("Round: speedTarget %d \n", speedTarget);
  }
  else
  {
    //No speed to report
    speedTarget = -1;
    Serial.printf("Invalid: speedTarget %d \n", speedTarget);
  }

  /*if (millis() - getRandom > gpsRefresh) {
    //---------------------------------
    speedTarget = random(20);
    Serial.printf("Random: speedTarget %d \n", speedTarget);
    getRandom = millis();
  }
  */
  

  // ONLY if the value has changed
  if (speedTarget != speedDisplay)
  {
    
    if (speedDisplay == -1) {
      //Previous speed was invalid speed; overwrite with '-'
      sprintf(mph, "-");
    }
    else {
      sprintf(mph, "%d", speedDisplay);
    }

    // display the prior value in black to "erase" it
    Serial.printf("Clearing display: (speedTarget %d != speedDisplay %d) with old display value %s \n", speedTarget, speedDisplay, mph);
    tft.setTextColor(TFT_BLACK, TFT_BLACK);
    x = (3 - strlen(mph)) * 110; // each character is 110 pixels
    size = tft.drawString(mph, x, 0, 8);

    // Increment or decrement the speedDisplay value by one
    // and set the smartDelay appropriately so we make it to the speedTarget within
    // the GPS refresh time.

    if (speedTarget == -1) {
      //Invalid speed; just set display to -1 
      speedDisplay = -1;
      sprintf(mph, "-");
    }
    else if (speedDisplay < speedTarget) {
      ++speedDisplay;
      sprintf(mph, "%d", speedDisplay);
    }
    else if (speedDisplay > speedTarget) {
      --speedDisplay;
      sprintf(mph, "%d", speedDisplay);
    }
    
    // display the new value
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    x = (3 - strlen(mph)) * 110; // each character is 110 pixels
    size = tft.drawString(mph, x, 0, 8);

    Serial.printf("Calculating steps for delay (speedTarget %i - speedDisplay %i) \n", speedTarget, speedDisplay);

    if (speedTarget == -1) {
      steps = 1;  
    }
    else {
      steps = abs(speedTarget - speedDisplay) + 1;
    }
    
    Serial.printf("steps for delay: %d \n", steps);
    stepDelay = gpsRefresh / steps /2;
    Serial.printf("stepDelay %d  \n", stepDelay);

    Serial.printf("steps for delay: %d \n", steps);
    stepDelay = gpsRefresh / steps /2;
    Serial.printf("stepDelay %d  \n", stepDelay);

    Serial.printf("steps for delay: %d \n", steps);
    stepDelay = gpsRefresh / steps /2;
    Serial.printf("stepDelay %d  \n", stepDelay);
  }

  //--------------------------------------
  Serial.println("--COURSE--");

  if (gps.course.isValid())
  {
    sprintf(strCard, "%s     ", TinyGPSPlus::cardinal(gps.course.deg())); // Add spaces to overwrite any old direction that was longer
  }
  else
  {
    sprintf(strCard, "%s     ", "???");
  }
  Serial.printf("Cardinal Dir: %s \n", strCard);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  size = tft.drawString(strCard, 0, 0, 2); // each character is 28 pixels

  //--------------------------------------
  Serial.println("--LOOP--");

  Serial.printf("steps for delay: %d \n", steps);
  if (steps == 0) steps = 1;
  stepDelay = gpsRefresh / steps /2;  
  Serial.printf("stepDelay %d  \n\n", stepDelay);
  
  smartDelay(stepDelay);

  if (millis() > 6000 && gps.charsProcessed() < 10)
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
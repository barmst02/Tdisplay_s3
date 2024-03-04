#include <Arduino.h>
#include "pin_config.h"


//GPS Baud rate
#define GPSBaud 9600 
 
//Serial Monitor Baud Rate
#define Serial_Monitor_Baud 115200 


void setup() {
  Serial0.begin(GPSBaud ); // Initialize serial communication
  
  Serial.begin(Serial_Monitor_Baud); // Initialize serial communication
  Serial.println("Hello T-Display-S3");
  
}


void loop() {
  while (Serial0.available() > 0)
    Serial.write(Serial0.read());
  
}



//----------------------------------------------------------------//
//
// SAIC Robotics Fall 2023
// Brett Ian Balogh
// https://github.com/giantmolecules/ROBOTICS_FA23_CODE.git
//
// project_template.ino
//
// This is a template for our code that readies the TFT for use.
//
//----------------------------------------------------------------//

// Include libraries for TFT
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// Create a TFT object
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

//----{SETUP}-----------------------------------------------------//

void setup() {

  // Start Serial COM
  Serial.begin(115200);
  
  // turn on backlite
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);

  // turn on the TFT / I2C power supply
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
  delay(10);

  // initialize TFT
  tft.init(135, 240); // Init ST7789 240x135
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);
  
  // default text size
  tft.setTextSize(1);
  
  // set text foreground and background colors
  tft.setTextColor(ST77XX_WHITE,ST77XX_BLACK);

  Serial.println(F("TFT Initialized"));

}

//----{LOOP}------------------------------------------------------//

void loop() {
 
}

//----{END}------------------------------------------------------//

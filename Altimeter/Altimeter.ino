/*
  Example animated analogue meters using a ILI9341 TFT LCD screen
  Needs Font 2 (also Font 4 if using large scale label)
  Make sure all the display driver and pin comnenctions are correct by
  editting the User_Setup.h file in the TFT_eSPI library folder.

  #########################################################################
  ###### DON'T FORGET TO UPDATE THE User_Setup.h FILE IN THE LIBRARY ######
  #########################################################################
*/

/***************************************************************************
  This is a library for the BMP280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BMP280 Breakout
  ----> http://www.adafruit.com/products/2651

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <TFT_eSPI.h>             // Hardware-specific library
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include "Button2.h";

Adafruit_BMP280 bmp; // I2C

TFT_eSPI tft = TFT_eSPI();        // Invoke custom library

#define BUTTON_PIN  36

Button2 button = Button2(BUTTON_PIN);

#define TFT_GREY 0x5AEB

#define LOOP_PERIOD 35            // Display updates every 35 ms

uint16_t osx = 120, osy = 120;    // Saved x & y coords
uint32_t updateTime = 100;        // time for next update

float old_analog =  -999;         // Value last displayed

int value[6] = {0, 0, 0, 0, 0, 0};
float altitude;
int d = 0;

static int16_t w, h, center;
static int16_t sHandLen, markLen;
static float sdeg, mdeg;

#define BACKGROUND TFT_BLACK
#define MARK_COLOR TFT_WHITE
#define SUBMARK_COLOR TFT_GREY     // LIGHTGREY

#define SIXTIETH_RADIAN 0.125663706 
#define RIGHT_ANGLE_RADIAN 1.5707963

float SeaLevelPressure = 1029.00;  // atmospheric pressure in your area in mBar

void setup(void) {
  button.setPressedHandler(pressed);
  tft.init();
  tft.setRotation(0);
  Serial.begin(115200); // For debug
  if (!bmp.begin(0x76)) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
    }
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */    
  tft.fillScreen(TFT_BLACK);

  // init LCD constant
  w = tft.width();
  h = tft.height();
  if (w < h){
    center = w / 2;
  }
  else{
    center = h / 2;
  }
  sHandLen = center * 5 / 6;
  markLen = sHandLen / 6;
  // Draw 60 clock marks
  draw_round_clock_mark(center - markLen, center, center - (markLen * 2 / 3), center, center - (markLen / 2), center);
  updateTime = millis(); // Next update time
}

void loop() {
  button.loop();
  if (updateTime <= millis()) {
    updateTime = millis() + LOOP_PERIOD;
    altitude = bmp.readAltitude(SeaLevelPressure);
    int pressure =  bmp.readPressure();
    Serial.print(pressure);
    Serial.print(", ");
    Serial.println(altitude);
    //unsigned long t = millis();
    plotNeedle(altitude, 0);
    //Serial.println(millis()-t); // Print time taken for meter update      
  }
}

void draw_round_clock_mark(int16_t innerR1, int16_t outerR1, int16_t innerR2, int16_t outerR2, int16_t innerR3, int16_t outerR3){
  float x, y;
  int16_t x0, x1, y0, y1, innerR, outerR;
  uint16_t c;

  for (uint8_t i = 0; i < 50; i++){
    if ((i % 5) == 0){
      innerR = innerR1;
      outerR = outerR1;
      c = MARK_COLOR;
    }
    else{
      innerR = innerR3;
      outerR = outerR3;
      c = SUBMARK_COLOR;
    }

    mdeg = (SIXTIETH_RADIAN * i) - RIGHT_ANGLE_RADIAN;
    x = cos(mdeg);
    y = sin(mdeg);
    x0 = x * outerR + center;
    y0 = y * outerR + center;
    x1 = x * innerR + center;
    y1 = y * innerR + center;

    tft.drawLine(x0, y0, x1, y1, c);
  }
  tft.drawCentreString("0", 120, 20, 2);
  tft.drawCentreString("1", 177, 35, 2);
  tft.drawCentreString("2", 213, 83, 2);
  tft.drawCentreString("3", 213, 145, 2);
  tft.drawCentreString("4", 177, 192, 2);
  tft.drawCentreString("5", 120, 210, 2);
  tft.drawCentreString("6", 65, 192, 2);
  tft.drawCentreString("7", 29, 145, 2);
  tft.drawCentreString("8", 29, 83, 2);
  tft.drawCentreString("9", 65, 35, 2);
}

// #########################################################################
// Update needle position
// This function is blocking while needle moves, time depends on ms_delay
// 10ms minimises needle flicker if text is drawn within needle sweep area
// Smaller values OK if text not in sweep area, zero for instant movement but
// does not look realistic... (note: 100 increments for full scale deflection)
// #########################################################################
void plotNeedle(float value, byte ms_delay){

  tft.drawCircle(120, 120, 6, TFT_RED);
  tft.fillCircle(120, 120, 5, TFT_MAGENTA);

  // Move the needle util new value reached
  while (!(value == old_analog)) {

    if (ms_delay == 0) old_analog = value; // Update immediately id delay is 0

    if (old_analog <= 10){
      sdeg = old_analog * 36;
    }
    else if(old_analog > 10 & old_analog <= 100) {
      sdeg = old_analog/10 * 36;
    }
    else if(old_analog > 100 & old_analog <= 1000) {
      sdeg = old_analog/100 * 36; 
    }
    else {
      sdeg = old_analog/1000 * 36;
    }
    // Calcualte tip of needle coords
    float sx = cos((sdeg * 0.0174532925) - RIGHT_ANGLE_RADIAN);
    float sy = sin((sdeg * 0.0174532925) - RIGHT_ANGLE_RADIAN);

    // Erase old needle image
    tft.drawLine(120 - 1, 120, osx - 1, osy, TFT_BLACK);
    tft.drawLine(120, 120, osx, osy, TFT_BLACK);
    tft.drawLine(120 + 1, 120, osx + 1, osy, TFT_BLACK);

    // Re-plot text under needle
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawCentreString("ALT", 120, 50, 4); // // Comment out to avoid font 4

    // Re-plot valie under needle
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    char buf[8]; 
    dtostrf(value, 4, 1, buf);
    tft.fillRect(85, 80, 70, 20, TFT_BLACK);
    tft.drawCentreString(buf, 120, 80, 4);

    // Store new needle end coords for next erase
    osx = sx * 85 + 120;
    osy = sy * 85 + 120;

    // Draw the needle in the new postion, magenta makes needle a bit bolder
    // draws 3 lines to thicken needle
    tft.drawLine(120 - 1, 120, osx - 1, osy, TFT_RED);
    tft.drawLine(120, 120, osx, osy, TFT_MAGENTA);
    tft.drawLine(120 + 1, 120, osx + 1, osy, TFT_RED);

    // Slow needle down slightly as it approaches new postion
    if (abs(old_analog - value) < 10) ms_delay += ms_delay / 5;

    // Wait before next update
    delay(ms_delay);
  }
}

void pressed(Button2& btn){
  SeaLevelPressure = bmp.readPressure() / 100;
  delay(100);
}

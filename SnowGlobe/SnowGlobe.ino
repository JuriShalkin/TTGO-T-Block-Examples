/**************************************************************************
 * Based on M5Stack IMU Snow-Globe
 * 
 * A simple program that turns an M5Stack Fire into a digital snow globe.
 * Note: The PSRAM is needed to fit the fullscreen sprite in 16bit.
 * The PSRAM must be activated in the compiler!
 * 
 * Hague Nusseck @ electricidea
 * v0.01 16.December.2020
 * https://github.com/electricidea/M5Stack-Snow-Globe
 * 
 * 
 * Distributed as-is; no warranty is given.
**************************************************************************/

#define LILYGO_WATCH_BLOCK
#define LILYGO_GC9A01A_MODULE                   //Use GC9A01A
#include <LilyGoWatch.h>    

// Stuff for the Graphical output
// The M5Stack screen pixel is 320x240, with the top left corner of the screen as the origin (0,0)
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240

float accX = 0.0F;
float accY = 0.0F;
float accZ = 0.0F;

/*

Direction of acceleration:

      . down = +z
  - - - - -
  |       |
  |       | --> -x
  | O O O |
  - - - - - 
      |
      V
     +y 
*/


// bitmap of one snowflake (XBM Format)
#include "snowflake.h"

// background image as uint16_t RGB565 array
#include "ChristmassThree.h"

// structure for thr position of every snow flake
const int flake_max = 150;
struct flakeObject {
  int32_t x;
  int32_t y;
  float speed;
};

flakeObject flakeArray[flake_max];

TTGOClass *watch;
TFT_eSPI *tft ;
MPU6050 *mpu;

// Sprite object "img" with pointer to "M5.Lcd" object
// the pointer is used by pushSprite() to push it onto the LCD
TFT_eSprite *Buffer;  

void checkSettings(){
    Serial.println();
    Serial.print(" * Sleep Mode:            ");
    Serial.println(mpu->getSleepEnabled() ? "Enabled" : "Disabled");
    Serial.print(" * Clock Source:          ");
    switch (mpu->getClockSource()) {
    case MPU6050_CLOCK_KEEP_RESET:     Serial.println("Stops the clock and keeps the timing generator in reset"); break;
    case MPU6050_CLOCK_EXTERNAL_19MHZ: Serial.println("PLL with external 19.2MHz reference"); break;
    case MPU6050_CLOCK_EXTERNAL_32KHZ: Serial.println("PLL with external 32.768kHz reference"); break;
    case MPU6050_CLOCK_PLL_ZGYRO:      Serial.println("PLL with Z axis gyroscope reference"); break;
    case MPU6050_CLOCK_PLL_YGYRO:      Serial.println("PLL with Y axis gyroscope reference"); break;
    case MPU6050_CLOCK_PLL_XGYRO:      Serial.println("PLL with X axis gyroscope reference"); break;
    case MPU6050_CLOCK_INTERNAL_8MHZ:  Serial.println("Internal 8MHz oscillator"); break;
    }
    Serial.print(" * Accelerometer:         ");
    switch (mpu->getRange()) {
    case MPU6050_RANGE_16G:            Serial.println("+/- 16 g"); break;
    case MPU6050_RANGE_8G:             Serial.println("+/- 8 g"); break;
    case MPU6050_RANGE_4G:             Serial.println("+/- 4 g"); break;
    case MPU6050_RANGE_2G:             Serial.println("+/- 2 g"); break;
    }
    Serial.print(" * Accelerometer offsets: ");
    Serial.print(mpu->getAccelOffsetX());
    Serial.print(" / ");
    Serial.print(mpu->getAccelOffsetY());
    Serial.print(" / ");
    Serial.println(mpu->getAccelOffsetZ());
    Serial.println();
}

void setup(void) {
  Serial.begin(115200);
  watch = TTGOClass::getWatch();
  watch->begin();
  watch->openBL();
  tft = watch->tft;
  tft->setSwapBytes(false);
  Buffer = new TFT_eSprite(tft);
  mpu = watch->mpu;
  checkSettings();
  // int the starting position of all snowflakes
  for(int i=0; i < flake_max; i++){
    // horizontally distributed
    flakeArray[i].x = random(SCREEN_WIDTH);
    // at the bottom of the screen
    flakeArray[i].y = SCREEN_HEIGHT-random(20);
    // individual speed for each snowflake
    flakeArray[i].speed = (random(80)+20)/100.0;
  }
  // electric-idea logo
  delay(1500);
  // Create a 16 bit sprite
  Buffer->setColorDepth(16);
  Buffer->createSprite(SCREEN_WIDTH, SCREEN_HEIGHT);
  // welcome text
  tft->setTextColor(TFT_WHITE);
  tft->setTextSize(1);
  tft->fillScreen(TFT_BLACK);
  delay(1500);
}

void loop() {
  // push the background image to the sprite
  Buffer->pushImage(0, 0, 240, 240, (uint16_t *)ChristmassThree);
  // get the acceleration data
  // values are in g (9.81 m/s2)
  Vector normAccel = mpu->readNormalizeAccel();
  accX = normAccel.YAxis;
  accY = normAccel.XAxis;
  // Draw the snowflakes
  for (int i=0; i < flake_max; i++)
  {
    // detect shaking
    if(fabs(accX) > 2 || fabs(accY) > 2){
      flakeArray[i].x = random(SCREEN_WIDTH);
      flakeArray[i].y = random(SCREEN_HEIGHT);
    }
    else {
      // use gravity vector for movement
      float dx = (accX*-10.0) + (round(accX)*random(5)) + (round(accY)*(random(10)-5));
      float dy = (accY*10.0) +  (round(accX)*random(5)) + (round(accY)*(random(10)-5));
      flakeArray[i].x = flakeArray[i].x + round(dx*flakeArray[i].speed);
      flakeArray[i].y = flakeArray[i].y + round(dy*flakeArray[i].speed);
    }
    // keep the snowflakes on the screen
    if(flakeArray[i].x < 0)
      flakeArray[i].x = 0;
    if(flakeArray[i].x > SCREEN_WIDTH)
      flakeArray[i].x = SCREEN_WIDTH;
    if(flakeArray[i].y < 0)
      flakeArray[i].y = 0;
    if(flakeArray[i].y > SCREEN_HEIGHT)
      flakeArray[i].y = SCREEN_HEIGHT;
    // push the snowflake to the sprite on top of the background image
    Buffer->drawXBitmap((int)(flakeArray[i].x-flakeWidth), 
                       (int)(flakeArray[i].y-flakeHeight), 
                       snowflake, flakeWidth, flakeHeight, TFT_WHITE);
  }
  // After all snowflakes are drawn, pus the sprite
  // to TFT screen CGRAM at coordinate x,y (top left corner)
  // Specify what colour is to be treated as transparent.
  Buffer->pushSprite(0, 0, TFT_TRANSPARENT);
  delay(20);
}

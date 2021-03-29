/*====================================================================================

  This example draws a jpeg image in a Sprite then plot a rotated copy of the Sprite
  to the TFT.

  The jpeg used in in the sketch Data folder (presss Ctrl+K to see folder)

  The jpeg must be uploaded to the ESP8266 or ESP32 SPIFFS by using the Tools menu
  sketch data upload option of the Arduino IDE. If you do not have that option it can
  be added. Close the Serial Monitor window before uploading to avoid an error message!

  To add the upload option for the ESP8266 see:
  http://www.esp8266.com/viewtopic.php?f=32&t=10081
  https://github.com/esp8266/arduino-esp8266fs-plugin/releases

  To add the upload option for the ESP32 see:
  https://github.com/me-no-dev/arduino-esp32fs-plugin

  Created by Bodmer 6/1/19 as an example to the TFT_eSPI library:
  https://github.com/Bodmer/TFT_eSPI

  Extension funtions in the TFT_eFEX library are used to list SPIFFS files and render
  the jpeg to the TFT and to the Sprite:
  https://github.com/Bodmer/TFT_eFEX

  To render the Jpeg image the JPEGDecoder library is needed, this can be obtained
  with the IDE library manager, or downloaded from here:
  https://github.com/Bodmer/JPEGDecoder

  ==================================================================================*/

//====================================================================================
//                                  Libraries
//====================================================================================
// Call up the SPIFFS FLASH filing system, this is part of the ESP Core
#define FS_NO_GLOBALS
#include <FS.h>
#include "Button2.h";
#include "SPIFFS.h" // Needed for ESP32 only

#define BUTTON_PIN  36
Button2 button = Button2(BUTTON_PIN);

// https://github.com/Bodmer/TFT_eSPI
#include <TFT_eSPI.h>                 // Hardware-specific library
TFT_eSPI tft = TFT_eSPI();            // Invoke custom library
TFT_eSprite spr = TFT_eSprite(&tft);  // Create Sprite object "spr" with pointer to "tft" object

// https://github.com/Bodmer/TFT_eFEX
#include <TFT_eFEX.h>                 // Include the function extension library
TFT_eFEX fex = TFT_eFEX(&tft);        // Create TFT_eFX object "fex" with pointer to "tft" object

int start_angle = 0;

//====================================================================================
//                                    Setup
//====================================================================================
void setup() {
  button.setPressedHandler(pressed);
  Serial.begin(250000); // Used for messages

  tft.begin();
  tft.setRotation(0);  // 0 & 2 Portrait. 1 & 3 landscape
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);

  // Create a sprite to hold the jpeg (or part of it)
  spr.createSprite(240, 240);
  spr.setSwapBytes(false);

  // Initialise SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS initialisation failed!");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  Serial.println("\r\nInitialisation done.\r\n");

  // Lists the files so you can see what is in the SPIFFS
  fex.listSPIFFS();

  // Note the / before the SPIFFS file name must be present, this means the file is in
  // the root directory of the SPIFFS, e.g. "/tiger.jpg" for a file called "tiger.jpg"

  // Send jpeg info to serial port
  fex.jpegInfo("/Fortune.jpg");

  // Draw jpeg iamge in Sprite spr at 0,0
  fex.drawJpeg("/Fortune.jpg", 0 , 0, &spr);

  
  //tft.fillScreen(random(0xFFFF));
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Do you want to know what fortune", 120, 100, 2);
  tft.drawString("has in store for you?", 120, 120, 2);
  tft.drawString("Just push the button...", 120, 140, 2);
  

  // Set the TFT pivot point to the centre of the screen
  tft.setPivot(120, 120);

  // Set Sprite pivot point to centre of Sprite
  spr.setPivot(120, 120);

}

//====================================================================================
//                                    Loop
//====================================================================================
void loop() {
  button.loop();
}
//====================================================================================

void pressed(Button2& btn) {
  int defence = random(72, 240) * 5;
  int stop_angle = start_angle + defence;
  for (int16_t angle = start_angle; angle <= stop_angle; angle += 5) {
    spr.pushRotated(angle);
    tft.fillTriangle(220, 120, 240, 110, 240, 130, TFT_RED);
      delay(20);
  }
  start_angle = stop_angle;
}


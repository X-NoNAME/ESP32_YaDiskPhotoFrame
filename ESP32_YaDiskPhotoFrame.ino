#include "SPIFFS.h"
#include <WiFi.h>
#include <HTTPClient.h>

// Include SPIFFS
#define FS_NO_GLOBALS
#include <FS.h>

// Include the jpeg decoder library
#include <TJpg_Decoder.h>
#include <ArduinoJson.h>

// Include the TFT library https://github.com/Bodmer/TFT_eSPI
#include "SPI.h"
#include <TFT_eSPI.h>              // Hardware-specific library


// Chenge next 2 lines to suit your WiFi network
#define WIFI_SSID "........"
#define PASSWORD "......"

// Define Token, folder with images, number of images and image size
#define YA_TOKEN "......" //You can take temporarry token from here "https://yandex.ru/dev/disk/poligon/"
#define YA_FOLDER "disk:/Фотокамера/"
#define YA_FOLDER_SIZE 35127
#define YA_IMAGE_SIZE "320x240" //should be same as TFT display

#define CHANGE_IMAGE_INTERVAL 30000 //30 sec

TFT_eSPI tft = TFT_eSPI();


// This next function will be called during decoding of the jpeg file to
// render each block to the TFT.  If you use a different TFT library
// you will need to adapt this function to suit.
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
  if ( y >= tft.height() ) return 0;
  tft.pushImage(x, y, w, h, bitmap);
  return 1;
}

void setup()
{
  Serial.begin(115200);

  // Initialise SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS initialisation failed!");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  Serial.println("\r\nInitialisation done.");

  // Initialise the TFT
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
  TJpgDec.setJpgScale(1);

  // The byte order can be swapped (set true for TFT_eSPI)
  TJpgDec.setSwapBytes(true);

  // The decoder must be given the exact name of the rendering function above
  TJpgDec.setCallback(tft_output);

  WiFi.begin(WIFI_SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
}

void loop()
{
  if (SPIFFS.exists("/image.jpg") == true) {
    SPIFFS.remove("/image.jpg");
  }

  // Fetch the jpg file from the specified URL, examples only, from imgur
  String image_url = yandexGetUrl();
  Serial.println("URL: " + image_url);
  if (image_url == "") {
    delay(1000);
    return;
  }
  bool loaded_ok = gdownloadImageToFile(image_url, "/image.jpg"); // Note name preceded with "/"
  if(!loaded_ok){
    Serial.println("Couldn't load file: " + image_url);
    delay(1000);
    return;
  }

  tft.fillScreen(TFT_BLACK);
  TJpgDec.drawFsJpg(0, 0, "/image.jpg");

  // Wait CHANGE_IMAGE_INTERVAL ms for next image
  delay(CHANGE_IMAGE_INTERVAL);
}

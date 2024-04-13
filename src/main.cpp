#include <Arduino.h>
#include <WiFiMulti.h>
#include <Audio.h>
#include "common.h"
#include "modules/rfid.h"
#include "modules/wifi.h"
#include "modules/audio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Try max SPI clock for an SD. Reduce SPI_CLOCK if errors occur.
#define SPI_CLOCK SD_SCK_MHZ(50)
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK)
//#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)

#define CLOCK_MHZ 1000000UL * 40

#define I2S_DOUT      40
#define I2S_BCLK      41
#define I2S_LRC       42

#define NUM_LEDS  1
#define LED_PIN   38

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void dump_byte_array(byte *buffer, byte bufferSize);

/**
 * Initialize.
 */
void setup() {

  Serial.begin(115200);


  // Wait for USB Serial
  while (!Serial) {
    yield();
  }

  Serial.write("start\n");

  init_wifi();

  strip.begin();
  strip.setBrightness(50);
  strip.show();

  init_rfid(&strip);
  init_audio();

//  WiFi.mode(WIFI_STA);
//  wifiMulti.addAP(ssid.c_str(), password.c_str());
//  wifiMulti.run();
//  if(WiFi.status() != WL_CONNECTED){
//    WiFi.disconnect(true);
//    wifiMulti.run();
//  }

  Serial.println("Starting RFID task");
  xTaskCreatePinnedToCore( rfid_task, "rfid_task", 1024*4, NULL, 2 | portPRIVILEGE_BIT, NULL, 1);
  Serial.println("Starting audio task");
  xTaskCreate( audio_task, "audio_task", 1024*8, NULL, 2 | portPRIVILEGE_BIT, NULL);

}

/**
 * Main loop.
 */
void loop() {}





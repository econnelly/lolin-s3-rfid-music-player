#include <Arduino.h>
#include <WiFiMulti.h>
#include <Audio.h>
#include "main.h"
#include "common.h"
#include "modules/rfid.h"
#include "modules/wifi.h"
#include "modules/audio.h"
#include "modules/input.h"

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

shared_model model;

int stop_button_state = 0;

/**
 * Initialize.
 */
void setup() {

  Serial.begin(115200);


  // Wait for USB Serial
  // while (!Serial) {
  //   yield();
  // }

  Serial.write("start\n");

  init_main();
  init_input();
  init_wifi();

  strip.begin();
  strip.setBrightness(50);
  strip.show();

  init_rfid(&strip);
  init_audio();

  Serial.println("Starting input task");
  xTaskCreate( input_task, "audio_task", 1024*8, (void *) &model, 2 | portPRIVILEGE_BIT, NULL);
  Serial.println("Starting RFID task");
  xTaskCreatePinnedToCore( rfid_task, "rfid_task", 1024*4, (void *) &model, 2 | portPRIVILEGE_BIT, NULL, 1);
  Serial.println("Starting audio task");
  xTaskCreate( audio_task, "audio_task", 1024*8, (void *) &model, 2 | portPRIVILEGE_BIT, NULL);

}

void init_main() {
  model.audio_playback_semaphore = xSemaphoreCreateMutex();
}

/**
 * Main loop.
 */
void loop() {}
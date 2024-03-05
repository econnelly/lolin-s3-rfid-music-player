#include <Arduino.h>
#include <WiFiMulti.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_NeoPixel.h>
#include <Audio.h>
#include "sd_wrap.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

const uint8_t SD_CS_PIN = 46;

// Try max SPI clock for an SD. Reduce SPI_CLOCK if errors occur.
#define SPI_CLOCK SD_SCK_MHZ(50)
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK)
//#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)

#define CLOCK_MHZ 1000000UL * 40


#define RST_PIN                 5         // Configurable, see typical pin layout above
#define MFRC522_SS_PIN          7         // Configurable, take a unused pin, only HIGH/LOW required, must be different to SS 2
#define MFRC522_SCK             6
#define MFRC522_MOSI            16
#define MFRC522_MISO            15

#define I2S_DOUT      40
#define I2S_BCLK      41
#define I2S_LRC       42

#define NUM_LEDS  1
#define LED_PIN   38

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
MFRC522 mfrc522(MFRC522_SS_PIN, RST_PIN);   // Create MFRC522 instance.
WiFiMulti wifiMulti;
String ssid =     "spaz";
String password = "connelly3720232";

SPIClass AUDIO_SPI(HSPI);

void dump_byte_array(byte *buffer, byte bufferSize);

[[noreturn]] void audio_task(void *param);

[[noreturn]] void rfid_task(void *param);

/**
 * Initialize.
 */
void setup() {

  Serial.begin(115200);
  Serial.write("start\n");

  strip.begin();
  strip.setBrightness(50);
  strip.show();

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(ssid.c_str(), password.c_str());
  wifiMulti.run();
  if(WiFi.status() != WL_CONNECTED){
    WiFi.disconnect(true);
    wifiMulti.run();
  }

  // Wait for USB Serial
  while (!Serial) {
    yield();
  }

  Serial.println("Starting RFID task");
  xTaskCreatePinnedToCore( rfid_task, "rfid_task", 1024*4, NULL, 2 | portPRIVILEGE_BIT, NULL, 1);
  Serial.println("Starting audio task");
  xTaskCreate( audio_task, "audio_task", 1024*8, NULL, 2 | portPRIVILEGE_BIT, NULL);

}

/**
 * Main loop.
 */
void loop() {}

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

[[noreturn]] void audio_task(void *param) {
//  while(true) {
//    Serial.println("audio_task");
//    vTaskDelay(1000);
//  }

  AUDIO_SPI.begin(SCK, MISO, MOSI, SD_CS_PIN);

  Serial.println("Delaying 5 seconds...");
  vTaskDelay(5000);
  Serial.println("Done.");

  Serial.print("Initializing SD card");
//  while(!SDFAT.begin(SD_CONFIG)) {
  while(!SDFAT.begin(SD_CS_PIN, AUDIO_SPI, SD_SCK_MHZ(50), "/sd", 5)) {
    Serial.print(".");
    vTaskDelay(50);
  }

  Serial.println();

  if(!SDFAT.exists("lava.mp3")) {
    Serial.println("FILE NOT FOUND");
  }

  Audio audio;

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(5);

//  bool audioPlaying = audio.connecttohost("http://stream2.dancewave.online:8080/dance.ogg");
  bool audioPlaying = audio.connecttoFS(SDFAT, "/lava.mp3");
  if (audioPlaying) {
    Serial.println("Audio should be playing");
    for (;;) {
      audio.loop();
      if (!audio.isRunning()) {
        Serial.println("Audio is NOT playing");
        vTaskDelay(1000);
      } else {
        vTaskDelay(5);
      }
    }
  } else {
    Serial.println("Error playing audio");
  }
}

[[noreturn]] void rfid_task(void *param) {

  SPI.begin(MFRC522_SCK, MFRC522_MISO, MFRC522_MOSI, MFRC522_SS_PIN);

  mfrc522.PCD_Init();
  mfrc522.PCD_DumpVersionToSerial();

  for (;;) {
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      Serial.print(F("Card UID:"));
      dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
      Serial.println();
      MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
      Serial.println(mfrc522.PICC_GetTypeName(piccType));
      // Halt PICC
      mfrc522.PICC_HaltA();
      // Stop encryption on PCD
      mfrc522.PCD_StopCrypto1();

      strip.setPixelColor(0, 0xFF0000);
      strip.show();
      vTaskDelay(500);
      strip.setPixelColor(0, 0x000000);
      strip.show();

    } else {
      vTaskDelay(100);
    }
  }
}

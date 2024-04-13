//
// Created by eli on 4/13/2024.
//

#include "modules/audio.h"

SPIClass AUDIO_SPI(HSPI);

void init_audio() {
  Serial.println("Initializing Audio...");
  AUDIO_SPI.begin(SCK, MISO, MOSI, SD_CS_PIN);

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
}

void audio_task(void *param) {

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
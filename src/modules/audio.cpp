//
// Created by eli on 4/13/2024.
//

#include "modules/audio.h"
#include "common.h"

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
}

[[noreturn]] void audio_task(void *model) {

  auto *m = (shared_model *) model;
  SemaphoreHandle_t audio_playback_semaphore = m->audio_playback_semaphore;
  Audio audio;
  bool audioPlaying = false;

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(3);

  for(;;) {

    if (m->playback_command != NONE) {
      while (xSemaphoreTake(audio_playback_semaphore, (TickType_t) 10) != pdTRUE) {
        Serial.println("[audio] Can't take semaphore... waiting...");
      }

      playback_command_enum command = m->playback_command;
      int8_t vol = audio.getVolume();

      if (command == VOL_DOWN) {
        vol = max(3, vol - 1);
        audio.setVolume(vol);
        Serial.write("Volume: ");
        Serial.write((int) audio.getVolume());
        Serial.write("\n");
      } else if (command == VOL_UP) {
        vol = min(vol + 1, (int) audio.maxVolume());
        audio.setVolume(vol);
        Serial.write("Volume: ");
        Serial.write((int) audio.getVolume());
        Serial.write("\n");
      } else if (command == STOP) {
        audio.stopSong();
        Serial.write("Stopping song.\n");
      }

      m->playback_command = NONE;

      xSemaphoreGive(audio_playback_semaphore);


    }

//  bool audioPlaying = audio.connecttohost("http://stream2.dancewave.online:8080/dance.ogg");
    if (m->card_id != 0) {
      while (xSemaphoreTake(audio_playback_semaphore, (TickType_t) 10) != pdTRUE) {
        Serial.println("[audio] Can't take semaphore... waiting...");
      }

      String file_name = String("/");
      file_name.concat(m->playback_file_name);

      m->playback_file_name.clear();

      if(audioPlaying) {
        audio.stopSong();
      }

      if (SDFAT.exists(file_name)) {
        audioPlaying = audio.connecttoFS(SDFAT, file_name.c_str());
      } else {
        audioPlaying = audio.connecttoFS(SDFAT, "/missing.mp3");
      }

      file_name.clear();
      m->card_id = 0;

      m->playback_state = PLAYING;

      xSemaphoreGive(audio_playback_semaphore);
    }

    if (audioPlaying) {
      audio.loop();
      if (!audio.isRunning()) {
        audio.stopSong();
        audioPlaying = false;
        vTaskDelay(1000);
        continue;
      } else {
        vTaskDelay(5);
      }
    } else {
      vTaskDelay(5);
    }
  }
}
//
// Created by eli on 4/2/2024.
//

#ifndef LOLIN_S3_RFID_MUSIC_PLAYER_COMMON_H
#define LOLIN_S3_RFID_MUSIC_PLAYER_COMMON_H

#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include <WString.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "utils/sd_wrap.h"

enum playback_state_enum {STOPPED, PAUSED, PLAYING, WAITING};
enum playback_command_enum {NONE, PLAY, PAUSE, STOP, VOL_UP, VOL_DOWN};
struct shared_model_t {
  enum playback_state_enum playback_state = STOPPED;
  enum playback_command_enum playback_command = NONE;
  int32_t card_id = 0;
  String playback_file_name = String("test");
  SemaphoreHandle_t audio_playback_semaphore;
};

typedef struct shared_model_t shared_model;

#endif //LOLIN_S3_RFID_MUSIC_PLAYER_COMMON_H

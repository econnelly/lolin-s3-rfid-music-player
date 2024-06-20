//
// Created by eli on 4/13/2024.
//

#include <Audio.h>
#include "common.h"

#ifndef LOLIN_S3_RFID_MUSIC_PLAYER_AUDIO_H
#define LOLIN_S3_RFID_MUSIC_PLAYER_AUDIO_H

// Try max SPI clock for an SD. Reduce SPI_CLOCK if errors occur.
#define SPI_CLOCK SD_SCK_MHZ(50)
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK)
//#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)

#define CLOCK_MHZ 1000000UL * 40

#define I2S_DOUT      40
#define I2S_BCLK      41
#define I2S_LRC       42

const uint8_t SD_CS_PIN = 46;

void init_audio();

[[noreturn]] void audio_task(void *model);


#endif //LOLIN_S3_RFID_MUSIC_PLAYER_AUDIO_H

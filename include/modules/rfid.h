//
// Created by eli on 4/2/2024.
//

#ifndef LOLIN_S3_RFID_MUSIC_PLAYER_RFID_H
#define LOLIN_S3_RFID_MUSIC_PLAYER_RFID_H

#include <Adafruit_PN532.h>
#include "common.h"

#define RST_PIN     5         // Configurable, see typical pin layout above
#define PN532_SCK   6
#define PN532_MOSI  16
#define PN532_SS    7
#define PN532_MISO  15

#define NFC_BLOCK_SIZE 16

void init_rfid(Adafruit_NeoPixel *s);
uint8_t read_rfid(uint8_t key[6], uint8_t uid[7], uint8_t uidLength, char* out);
void dump_byte_array(byte *buffer, byte bufferSize);

[[noreturn]] void rfid_task(void *model);

#endif //LOLIN_S3_RFID_MUSIC_PLAYER_RFID_H

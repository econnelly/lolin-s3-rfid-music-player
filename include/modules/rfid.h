//
// Created by eli on 4/2/2024.
//

#ifndef LOLIN_S3_RFID_MUSIC_PLAYER_RFID_H
#define LOLIN_S3_RFID_MUSIC_PLAYER_RFID_H

#include "MFRC522.h"
#include "common.h"

#define RST_PIN                 5         // Configurable, see typical pin layout above
#define MFRC522_SS_PIN          7         // Configurable, take a unused pin, only HIGH/LOW required, must be different to SS 2
#define MFRC522_SCK             6
#define MFRC522_MOSI            16
#define MFRC522_MISO            15

void init_rfid(Adafruit_NeoPixel *s);
MFRC522::MIFARE_Key get_rfid_key();
void dump_byte_array(byte *buffer, byte bufferSize);
void rfid_task(void *param);

#endif //LOLIN_S3_RFID_MUSIC_PLAYER_RFID_H

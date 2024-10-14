//
// Created by eli on 4/2/2024.
//

#include "common.h"
#include "modules/rfid.h"

Adafruit_NeoPixel *s;
Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

uint8_t rfid_key[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uint8_t filename_rfid_header[4] = {0x54, 0x02, 0x65, 0x6E};

void init_rfid(Adafruit_NeoPixel *strip) {
  s = strip;
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
}

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

[[noreturn]] void rfid_task(void *model) {

  Serial.println("Called rfid_task");

  auto *m = (shared_model *) model;
  SemaphoreHandle_t audio_playback_semaphore = m->audio_playback_semaphore;
  Serial.println(m->playback_file_name);

  byte block;
  byte len;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  char rfid_string[16*10];

  bool running = false;

  for (;;) {

    bool stringEnded = false;
    bool isString = false;

    if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
      s->setPixelColor(0, 0xFF0000);
      s->show();
      vTaskDelay(500);
      s->setPixelColor(0, 0x000000);
      s->show();

      Serial.println("Found an ISO14443A card");
      Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
      Serial.print("  UID Value: ");
      nfc.PrintHex(uid, uidLength);
      Serial.println("");

      // byte buffer1[18];

      block = 4;
      len = 18;

//       char* value;
//       for (uint8_t i = 0; i < 16; i++)
//       {
//         value += (char)buffer1[i];
//       }
// //      value.trim();
//       Serial.print(value);

      if (uidLength == 4) {
      // We probably have a Mifare Classic card ...
      Serial.println("Seems to be a Mifare Classic card (4 byte UID)");

      uint8_t nfcType = read_rfid(rfid_key, uid, uidLength, rfid_string);
      if (nfcType > 0) {
        Serial.println(rfid_string);
      } else {
        Serial.println("NO STRING FOUND");
        continue;
      }
      
    } else {
      Serial.println("Ooops ... this doesn't seem to be a Mifare Classic card!");
    }

    if (stringEnded) {
      Serial.println(rfid_string);
    }

      while(xSemaphoreTake(audio_playback_semaphore, (TickType_t) 10) != pdTRUE) {
        Serial.println("[rfid] Can't take semaphore... waiting...");
      }

      Serial.println("Updating shared model object");

      String file_name = String(rfid_string);
      file_name.toUpperCase();

      m->card_id = 0;
      m->playback_file_name.clear();
      m->playback_file_name.concat(file_name);
      // m->playback_file_name.concat(".mp3");
      xSemaphoreGive(audio_playback_semaphore);

      vTaskDelay(1000);

    } else {
      vTaskDelay(25);
    }
  }
}

uint8_t read_rfid(uint8_t key[6], uint8_t uid[7], uint8_t uidLength, char* out) {
  // Now we try to go through all 16 sectors (each having 4 blocks)
      // authenticating each sector, and then dumping the blocks
      uint8_t data[NFC_BLOCK_SIZE];                         // Array to store block data during reads
      uint8_t dataPos = 9;
      uint8_t stringPos = 0;
      bool stringEnded = false;
      bool isString = false;
      bool authenticated = false;
      bool success = false;
      for (int currentblock = 4; currentblock < 24; currentblock++) {
        // Check if this is a new block so that we can reauthenticate
        if (nfc.mifareclassic_IsFirstBlock(currentblock)) authenticated = false;

        // If the sector hasn't been authenticated, do so first
        if (!authenticated) {
          if (currentblock == 0) {
              // This will be 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF for Mifare Classic (non-NDEF!)
              // or 0xA0 0xA1 0xA2 0xA3 0xA4 0xA5 for NDEF formatted cards using key a,
              // but keyb should be the same for both (0xFF 0xFF 0xFF 0xFF 0xFF 0xFF)
              success = nfc.mifareclassic_AuthenticateBlock (uid, uidLength, currentblock, 1, key);
          } else {
              // This will be 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF for Mifare Classic (non-NDEF!)
              // or 0xD3 0xF7 0xD3 0xF7 0xD3 0xF7 for NDEF formatted cards using key a,
              // but keyb should be the same for both (0xFF 0xFF 0xFF 0xFF 0xFF 0xFF)
              success = nfc.mifareclassic_AuthenticateBlock (uid, uidLength, currentblock, 1, key);
          }
          
          if (success) {
            authenticated = true;
          } else {
            Serial.print("Authentication error: currentblock="); Serial.println(currentblock);
          }
        }
        if (authenticated) {
          // Authenticated ... we should be able to read the block now
          // Dump the data into the 'data' array
          success = nfc.mifareclassic_ReadDataBlock(currentblock, data);
          if (success) {
            if ((data[5] != filename_rfid_header[0]
             || data[6] != filename_rfid_header[1]
             || data[7] != filename_rfid_header[2]
             || data[8] != filename_rfid_header[3]) && !isString) {
              return 0;
            } else {
              isString = true;
            }

            if (isString && !stringEnded) {
              while (dataPos < NFC_BLOCK_SIZE) {
                if (data[dataPos] == '\0') {
                  out[stringPos-1] = '\0';
                  return 1;
                } else {
                  out[stringPos++] = data[dataPos++];
                }
              }

              dataPos = 0;
            } else {
              break;
            }
          }
          else {
            // Oops ... something happened
            Serial.print("Block ");Serial.print(currentblock, DEC);
            Serial.println(" unable to read this block");

            return 0;
          }
        } else {
          return 0;
        }
      }

    return 0;
}

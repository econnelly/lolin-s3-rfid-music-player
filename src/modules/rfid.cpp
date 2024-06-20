//
// Created by eli on 4/2/2024.
//

#include "common.h"
#include "modules/rfid.h"

Adafruit_NeoPixel *s;

MFRC522 mfrc522(MFRC522_SS_PIN, RST_PIN);   // Create MFRC522 instance.
MFRC522::StatusCode status;

void init_rfid(Adafruit_NeoPixel *strip) {
  s = strip;

  SPI.begin(MFRC522_SCK, MFRC522_MISO, MFRC522_MOSI, MFRC522_SS_PIN);
  mfrc522.PCD_Init();
  mfrc522.PCD_DumpVersionToSerial();
}

MFRC522::MIFARE_Key get_rfid_key() {

  // Prepare key - all keys are set to FFFFFFFFFFFF at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (unsigned char & i : key.keyByte) i = 0xFF;

  Serial.println("Key: ");
  dump_byte_array(key.keyByte, MFRC522::MF_KEY_SIZE);
  Serial.println();

  return key;
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

  auto *m = (shared_model *) model;
  SemaphoreHandle_t audio_playback_semaphore = m->audio_playback_semaphore;
  Serial.println(m->playback_file_name);

  MFRC522::MIFARE_Key key = get_rfid_key();

  byte block;
  byte len;

  for (;;) {

    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      s->setPixelColor(0, 0xFF0000);
      s->show();
      vTaskDelay(500);
      s->setPixelColor(0, 0x000000);
      s->show();

      Serial.print(F("Card UID:"));
      dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
      Serial.println();
      MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
      Serial.println(mfrc522.PICC_GetTypeName(piccType));

      byte buffer1[18];

      block = 4;
      len = 18;

//      mfrc522.MIFARE_UnbrickUidSector(true);

//      status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid));
//      if (status != MFRC522::STATUS_OK) {
//        Serial.print(F("Authentication failed: "));
//        Serial.println(mfrc522.GetStatusCodeName(status));
//        vTaskDelay(1000);
//        continue;
//      }
//
//      status = mfrc522.MIFARE_Read(block, buffer1, &len);
//      if (status != MFRC522::STATUS_OK) {
//        Serial.print(F("Reading failed: "));
//        Serial.println(mfrc522.GetStatusCodeName(status));
//        vTaskDelay(1000);
//        continue;
//      }

      char* value = "";
      for (uint8_t i = 0; i < 16; i++)
      {
        value += (char)buffer1[i];
      }
//      value.trim();
      Serial.print(value);

      // Halt PICC
      mfrc522.PICC_HaltA();
      // Stop encryption on PCD
      mfrc522.PCD_StopCrypto1();

      while(xSemaphoreTake(audio_playback_semaphore, (TickType_t) 10) != pdTRUE) {
        Serial.println("[rfid] Can't take semaphore... waiting...");
      }

      int32_t id = 0;
      for(uint8_t i = 0; i < mfrc522.uid.size; i++) {
        id += mfrc522.uid.uidByte[i] << i*8;
      }

      String file_name = String(id, HEX);
      file_name.toUpperCase();

      m->card_id = id;
      m->playback_file_name.clear();
      m->playback_file_name.concat(file_name);
      m->playback_file_name.concat(".mp3");
      xSemaphoreGive(audio_playback_semaphore);

    } else {
      vTaskDelay(25);
    }
  }
}

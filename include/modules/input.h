#ifndef LOLIN_S3_RFID_MUSIC_PLAYER_INPUT_H
#define LOLIN_S3_RFID_MUSIC_PLAYER_INPUT_H

#define STOP_BUTTON_PIN     8
#define VOLUP_BUTTON_PIN    9
#define VOLDOWN_BUTTON_PIN  18

void init_input();
void input_task(void *model);
int read_input();

#endif //LOLIN_S3_RFID_MUSIC_PLAYER_INPUT_H
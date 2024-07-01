#include "modules/input.h"
#include "common.h"

void init_input() {
    pinMode(STOP_BUTTON_PIN, INPUT_PULLUP);
    pinMode(VOLUP_BUTTON_PIN, INPUT_PULLUP);
    pinMode(VOLDOWN_BUTTON_PIN, INPUT_PULLUP);
}

[[noreturn]] void input_task(void *model) {
    auto *m = (shared_model *) model;
    SemaphoreHandle_t audio_playback_semaphore = m->audio_playback_semaphore;
    int current_stop_button_state = 0;
    int current_volup_button_state = 0;
    int current_voldown_button_state = 0;

    int last_stop_button_state = 0;
    int last_volup_button_state = 0;
    int last_voldown_button_state = 0;

    for(;;) {
        last_stop_button_state = current_stop_button_state;
        current_stop_button_state = digitalRead(STOP_BUTTON_PIN);
        if (current_stop_button_state == 0 && last_stop_button_state > 0) {
            while (xSemaphoreTake(m->audio_playback_semaphore, (TickType_t) 10) != pdTRUE) {
                Serial.println("[audio] Can't take semaphore... waiting...");
            }
            m->playback_command = STOP;
            xSemaphoreGive(m->audio_playback_semaphore);
        }

        last_volup_button_state = current_volup_button_state;
        current_volup_button_state = digitalRead(VOLUP_BUTTON_PIN);
        if (current_volup_button_state == 0 && last_volup_button_state > 0) {
            Serial.println("Sending VOL_UP command");
            while (xSemaphoreTake(m->audio_playback_semaphore, (TickType_t) 10) != pdTRUE) {
                Serial.println("[audio] Can't take semaphore... waiting...");
            }
            m->playback_command = VOL_UP;
            xSemaphoreGive(m->audio_playback_semaphore);
        }

        last_voldown_button_state = current_voldown_button_state;
        current_voldown_button_state = digitalRead(VOLDOWN_BUTTON_PIN);
        if (current_voldown_button_state == 0 && last_voldown_button_state > 0) {
            Serial.println("Sending VOL_DOWN command");
            while (xSemaphoreTake(m->audio_playback_semaphore, (TickType_t) 10) != pdTRUE) {
                Serial.println("[audio] Can't take semaphore... waiting...");
            }
            m->playback_command = VOL_DOWN;
            xSemaphoreGive(m->audio_playback_semaphore);
        }
    }
}
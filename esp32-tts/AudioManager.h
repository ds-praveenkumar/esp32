#pragma once
#include <Arduino.h>

#define BTN_PIN 0

class AudioManagerClass {
public:
    bool init();
    void record();
    void stop_recording();
    bool is_recording();
    void play();
    void play_tone(int freq_hz, int duration_ms);

private:
    int16_t *buffer = nullptr;
    size_t num_samples = 0;
    size_t max_samples = 0;
    volatile bool recording = false;
};

extern AudioManagerClass AudioManager;

#include "AudioManager.h"
#include <Arduino.h>
#include <driver/i2s.h>
#include <math.h>

#define MIC_WS   15
#define MIC_SCK  16
#define MIC_SD   17

#define SPK_BCLK 4
#define SPK_LRC  3
#define SPK_DIN  7

#define SAMPLE_RATE    44100
#define RECORD_SECONDS 5
#define BUFFER_SAMPLES (SAMPLE_RATE * RECORD_SECONDS)

AudioManagerClass AudioManager;

bool AudioManagerClass::init()
{
    max_samples = BUFFER_SAMPLES;
    buffer = (int16_t *)ps_malloc(max_samples * sizeof(int16_t));
    if (!buffer) {
        Serial.println("PSRAM alloc failed!");
        return false;
    }
    Serial.printf("Buffer: %u bytes in PSRAM\n", max_samples * sizeof(int16_t));
    return true;
}

void AudioManagerClass::record()
{
    if (recording) return;

    i2s_config_t cfg = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 128,
        .use_apll = true,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0,
    };

    i2s_pin_config_t pins = {
        .mck_io_num = I2S_PIN_NO_CHANGE,
        .bck_io_num = MIC_SCK,
        .ws_io_num = MIC_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = MIC_SD,
    };

    if (i2s_driver_install(I2S_NUM_0, &cfg, 0, NULL) != ESP_OK) {
        Serial.println("I2S install failed");
        return;
    }
    i2s_set_pin(I2S_NUM_0, &pins);
    i2s_start(I2S_NUM_0);

    recording = true;
    num_samples = 0;

    while (digitalRead(BTN_PIN) == LOW) delay(10);

    while (recording && num_samples < max_samples) {
        size_t to_read = min((size_t)512, max_samples - num_samples);
        size_t bytes_read = 0;
        esp_err_t err = i2s_read(I2S_NUM_0, buffer + num_samples,
                                 to_read * sizeof(int16_t), &bytes_read,
                                 pdMS_TO_TICKS(100));
        if (err == ESP_OK) {
            num_samples += bytes_read / sizeof(int16_t);
        }
        if (digitalRead(BTN_PIN) == LOW) {
            delay(30);
            if (digitalRead(BTN_PIN) == LOW) {
                recording = false;
            }
        }
    }

    i2s_stop(I2S_NUM_0);
    i2s_driver_uninstall(I2S_NUM_0);
    recording = false;
    Serial.printf("Recorded %u samples\n", (unsigned)num_samples);
}

void AudioManagerClass::stop_recording()
{
    recording = false;
}

bool AudioManagerClass::is_recording()
{
    return recording;
}

void AudioManagerClass::play_tone(int freq_hz, int duration_ms)
{
    int samples = SAMPLE_RATE * duration_ms / 1000;
    int16_t *tone = (int16_t *)ps_malloc(samples * sizeof(int16_t));
    if (!tone) {
        Serial.println("Tone alloc failed");
        return;
    }
    for (int i = 0; i < samples; i++) {
        float t = (float)i / SAMPLE_RATE;
        tone[i] = (int16_t)(sin(2.0 * 3.14159 * freq_hz * t) * 20000);
    }

    i2s_config_t cfg = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 128,
        .use_apll = true,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0,
    };

    i2s_pin_config_t pins = {
        .mck_io_num = I2S_PIN_NO_CHANGE,
        .bck_io_num = SPK_BCLK,
        .ws_io_num = SPK_LRC,
        .data_out_num = SPK_DIN,
        .data_in_num = I2S_PIN_NO_CHANGE,
    };

    if (i2s_driver_install(I2S_NUM_1, &cfg, 0, NULL) != ESP_OK) {
        Serial.println("Tone I2S install failed");
        free(tone);
        return;
    }
    i2s_set_pin(I2S_NUM_1, &pins);
    i2s_start(I2S_NUM_1);

    size_t written = 0;
    while (written < (size_t)samples) {
        size_t chunk = min((size_t)512, (size_t)(samples - written));
        size_t bytes_sent = 0;
        i2s_write(I2S_NUM_1, tone + written, chunk * sizeof(int16_t),
                  &bytes_sent, portMAX_DELAY);
        written += bytes_sent / sizeof(int16_t);
    }

    i2s_stop(I2S_NUM_1);
    i2s_driver_uninstall(I2S_NUM_1);
    free(tone);
    Serial.println("Tone done");
}

void AudioManagerClass::play()
{
    if (num_samples == 0) {
        Serial.println("Nothing to play");
        return;
    }
    Serial.printf("Playing %u samples. First 5: %d %d %d %d %d\n",
        (unsigned)num_samples,
        buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);

    i2s_config_t cfg = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 128,
        .use_apll = true,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0,
    };

    i2s_pin_config_t pins = {
        .mck_io_num = I2S_PIN_NO_CHANGE,
        .bck_io_num = SPK_BCLK,
        .ws_io_num = SPK_LRC,
        .data_out_num = SPK_DIN,
        .data_in_num = I2S_PIN_NO_CHANGE,
    };

    if (i2s_driver_install(I2S_NUM_1, &cfg, 0, NULL) != ESP_OK) {
        Serial.println("SPK I2S install failed");
        return;
    }
    i2s_set_pin(I2S_NUM_1, &pins);
    i2s_start(I2S_NUM_1);

    size_t written = 0;
    int16_t gain_buf[512];
    while (written < num_samples) {
        size_t chunk = min((size_t)512, num_samples - written);
        for (size_t i = 0; i < chunk; i++) {
            int32_t s = buffer[written + i] * 16;
            if (s > 32767) s = 32767;
            if (s < -32768) s = -32768;
            gain_buf[i] = (int16_t)s;
        }
        size_t bytes_sent = 0;
        i2s_write(I2S_NUM_1, gain_buf, chunk * sizeof(int16_t),
                  &bytes_sent, portMAX_DELAY);
        written += bytes_sent / sizeof(int16_t);
    }

    i2s_stop(I2S_NUM_1);
    i2s_driver_uninstall(I2S_NUM_1);
    Serial.println("Playback done");
}

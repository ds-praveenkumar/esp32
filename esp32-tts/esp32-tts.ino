#include <Arduino.h>
#include <Wire.h>
#include "AudioManager.h"
#include "OledDisplay.h"
#include "WiFiManager.h"

// BTN_PIN defined in AudioManager.h

enum State { IDLE, RECORDING, PLAYING };
static State state = IDLE;
static unsigned long btn_debounce = 0;

static void handle_button()
{
    static int last = HIGH;
    int btn = digitalRead(BTN_PIN);
    if (last == HIGH && btn == LOW && millis() - btn_debounce > 300) {
        btn_debounce = millis();
        if (state == IDLE) {
            state = RECORDING;
        } else if (state == RECORDING) {
            AudioManager.stop_recording();
            state = PLAYING;
        }
    }
    last = btn;
}

static void scan_i2c()
{
    Serial.println("Scanning I2C...");
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printf("  Found device at 0x%02X\n", addr);
        }
    }
    Serial.println("Scan done");
}

void setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.println("ESP32-S3 Audio Recorder");

    if (!AudioManager.init()) {
        Serial.println("Audio init failed!");
        while (1) delay(100);
    }

    Wire.begin(18, 14);
    delay(100);
    scan_i2c();

    OledDisplay.init();
    OledDisplay.test_fill();
    OledDisplay.show("ESP32 Recorder", "Press BOOT");

    Serial.println("Connecting to WiFi...");
    OledDisplay.show("WiFi", "Connecting...");
    wifi_init();
    wifi_wait_connected(10000);
    OledDisplay.show("WiFi", wifi_is_connected() ? "Connected" : "Failed");
    delay(1000);

    Serial.println("Playing test tone (1kHz)...");
    AudioManager.play_tone(1000, 1000);
    Serial.println("Test tone done");

    pinMode(BTN_PIN, INPUT_PULLUP);
}

void loop()
{
    handle_button();

    switch (state) {
        case IDLE:
            break;

        case RECORDING:
            OledDisplay.show("Recording...", "");
            AudioManager.record();
            delay(200);
            state = PLAYING;
            break;

        case PLAYING:
            OledDisplay.show("Playing...", "");
            AudioManager.play();
            OledDisplay.show("Ready", "Press BOOT");
            state = IDLE;
            break;
    }

    delay(50);
}

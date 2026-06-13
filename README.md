# ESP32-S3 Audio Recorder

ESP32-S3 based audio recorder using INMP441 I2S microphone and MAX98357A I2S speaker amplifier, with SSD1306 OLED display.

Press BOOT to record, press again to play back.

## Features

- Record audio from INMP441 mic at 44.1kHz sample rate
- Playback through MAX98357A speaker amp (16x gain)
- 5-second recording buffer in PSRAM (~441KB)
- SSD1306 OLED display (128x64) showing status
- WiFi connectivity (STT/TTS ready)

## Pinout

### Microphone — INMP441

| INMP441 Pin    | ESP32-S3 GPIO |
|----------------|---------------|
| VDD            | 3.3V          |
| GND            | GND           |
| L/R            | GND           |
| WS (LRCLK)     | 15            |
| SCK (BCLK)     | 16            |
| SD (DOUT)      | 17            |

### Speaker Amp — MAX98357A

| MAX98357A Pin  | ESP32-S3 GPIO |
|----------------|---------------|
| VIN            | 3.3V or 5V    |
| GND            | GND           |
| BCLK           | 4             |
| LRC            | 3             |
| DIN            | 7             |

### OLED Display — SSD1306 (I2C)

| SSD1306 Pin    | ESP32-S3 GPIO |
|----------------|---------------|
| VDD            | 3.3V          |
| GND            | GND           |
| SDA            | 18            |
| SCK            | 14            |

### Button

| Function    | GPIO  |
|-------------|-------|
| BOOT button | 0     |(internal pull-up)

## Build & Flash

```bash
arduino-cli compile --fqbn esp32:esp32:esp32s3:PartitionScheme=no_ota,PSRAM=opi,FlashSize=16M esp32-tts
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32s3:PartitionScheme=no_ota,PSRAM=opi,FlashSize=16M esp32-tts
```

Monitor: `arduino-cli monitor -p /dev/ttyUSB0 -c baudrate=115200`

## WiFi

The sketch connects automatically on boot. OLED shows connection status.

Credentials go in `esp32-tts/secrets.h` (excluded from git via `.gitignore`):

```c
#define WIFI_SSID "your_ssid"
#define WIFI_PASS "your_password"
```

A template `secrets.h` is provided — edit with your own credentials.

## Dependencies

- `arduino-cli` with `esp32:esp32` core 3.3.8
- Libraries: `Adafruit SSD1306`, `Adafruit GFX` (bundled with core)

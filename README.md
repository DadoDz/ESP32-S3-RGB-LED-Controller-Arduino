# ESP32-S3 RGB LED Controller (Arduino)

Arduino IDE firmware that drives a WS2812 RGB LED on an ESP32-S3, controlled remotely via a Firebase Realtime Database node. Companion to the [ESP-IDF version](https://github.com/DadoDz/ESP32-S3-RGB-LED-Controller) — same concept, simpler code: one sketch file, no RTOS tasks, just `millis()` timing.

```
Firebase Realtime Database  <-- polled every 5s -->  ESP32-S3  --->  WS2812 LED
        (/led node)
```

Write `power` / `red` / `green` / `blue` / `brightness` to `/led` in your Firebase Realtime Database (Firebase console, curl, a companion app, whatever) and the board picks it up on the next poll.

## Features

- 🌈 Full RGB color + brightness control (0-100%)
- 🔌 Power on/off
- 📶 WiFi connect with status logging over Serial

## Hardware

- ESP32-S3 dev board
- WS2812 (NeoPixel) LED — GPIO 48 by default, matches the onboard LED on most ESP32-S3 devkits (change `LED_PIN` if yours is wired differently)

## Requirements

- [Arduino IDE](https://www.arduino.cc/en/software) with the `esp32` board package (Espressif) installed
- Board: **ESP32S3 Dev Module**
- Libraries (Library Manager):
  - **Adafruit NeoPixel**
  - **ArduinoJson**

## Setup

### 1. Firebase

1. Create a Firebase project and enable the **Realtime Database**.
2. Grab your database URL, e.g. `https://your-project-default-rtdb.firebaseio.com`.
3. The board reads from `<your-db-url>/led.json`.
4. No auth is used here, if your database is public, lock down its rules before relying on this for anything real.

Expected shape of the `/led` node:

```json
{
  "power": true,
  "red": 255,
  "green": 0,
  "blue": 0,
  "brightness": 100
}
```

### 2. Flash it

1. Copy `secrets.h.example` to `secrets.h` and fill in your WiFi SSID/password and Firebase URL.
2. Open `ESP32_S3_LED_Controller.ino` in Arduino IDE.
3. Select board **ESP32S3 Dev Module** and the right port.
4. Upload, then open Serial Monitor*.

## How it works

`loop()` checks the time since the last poll; every 5 seconds it GETs `/led.json`, parses the JSON, and pushes the resulting color/brightness to the pixel. No background tasks. The LED only gets updated right after a successful fetch.

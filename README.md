# kid.mp3  ## kitchen
IKEA DUKTIG re-brain

Create a wifi.mp3  ## secret.h file in the src directory - It should include:
```
#define SECRET.mp3  ## SSID "your_ssid"
#define SECRET.mp3  ## PASS "your_password"
```

Change the IP address of your ESP32 in the platformio.ini file


Parts:
* ESP32-VROOM-32
* MP3-TF-16P
* Switches with built-in LEDs
* Speaker
* micro SD card

Circuit diagram
![Circuit diagram](docs/kid.mp3  ## kitchen_circuit-v1.png)

Test breadboard
![Test Breadboard](docs/kid-kitchen-test-board.png)

The SD card should contain these files:

```
─ 01  ## Eng
│   ├── 001.mp3  ## Red
│   ├── 002.mp3  ## Green
│   ├── 003.mp3  ## Blue
│   ├── 004.mp3  ## Yellow
│   ├── 005.mp3  ## Cyan
│   ├── 006.mp3  ## Magenta
│   └── 007.mp3  ## White
├── 02  ## Spa
│   ├── 001.mp3  ## Rojo
│   ├── 002.mp3  ## Verde
│   ├── 003.mp3  ## Azul
│   ├── 004.mp3  ## Amarillo
│   ├── 005.mp3  ## Cian
│   ├── 006.mp3  ## Magenta
│   └── 007.mp3  ## Blanco
├── 97  ## Languages
│   ├── 001.mp3  ## English
│   └── 002.mp3  ## espanol
├── 98  ## buttons
│   ├── 001.mp3  ## button
│   ├── 002.mp3  ## button
│   ├── 003.mp3  ## button
│   ├── 004.mp3  ## button
│   ├── 005.mp3  ## tos_keypress1
│   ├── 006.mp3  ## tos_keypress2
│   ├── 007.mp3  ## tos_keypress3
│   └── 008.mp3  ## tos_keypress4
└── 99  ## Snd
    ├── 001.mp3  ## Vista
    ├── 004.mp3  ## sizzle
    ├── 006.mp3  ## frying_egg
    ├── 007.mp3  ## microwave
    ├── 008.mp3  ## timer_ding
    ├── 009.mp3  ## car_start
    └── 010.mp3  ## ants_marching
```
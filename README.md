# Dual ST7735 Animated Eyes on ESP32-S3 SuperMini

This project drives animated eye graphics on a 1.8 inch ST7735 128 x 160 TFT.
It is configured for an ESP32-S3 SuperMini with one display per board.

The current hardware model is:

- One ESP32-S3 SuperMini per eye
- One 1.8 inch ST7735 TFT per ESP32-S3
- Optional ESP-NOW broadcast sync between boards
- Left board can act as the motion/blink master
- Right board can act as a synced slave eye

## Known Working Display Wiring

Use this same wiring on both ESP32-S3 SuperMini boards:

| TFT pin | ESP32-S3 GPIO |
| --- | --- |
| MOSI / SDA | GPIO12 |
| SCLK / SCK | GPIO13 |
| DC / A0 | GPIO10 |
| CS | GPIO9 |
| RST / RESET | GPIO11 |
| BLK / LED | not GPIO controlled, set to `-1` |
| VCC | 3.3 V |
| GND | GND |

The display setup is in `User_Setup.h`.

Important working settings:

```cpp
#define ST7735_DRIVER
#define TFT_WIDTH  128
#define TFT_HEIGHT 160
#define ST7735_GREENTAB3

#define USE_HSPI_PORT
#define TFT_MOSI 12
#define TFT_SCLK 13
#define TFT_CS    9
#define TFT_DC   10
#define TFT_RST  11

#define TFT_RGB_ORDER TFT_RGB
#define SPI_FREQUENCY 27000000
```

`USE_HSPI_PORT` is important. Without it, this project previously crashed at
`tft.init()` on the ESP32-S3.

## PlatformIO Environments

The project uses the same source code for standalone, master, and slave builds.

| Environment | Purpose |
| --- | --- |
| `esp32-s3-supermini` | Standalone single eye, no ESP-NOW sync |
| `left_eye_master` | Left eye. Runs the animation and broadcasts sync packets |
| `right_eye_slave` | Right eye. Receives ESP-NOW packets and follows the master |

The mode is selected with build flags in `platformio.ini`:

```ini
EYE_SYNC_MODE=0  ; standalone
EYE_SYNC_MODE=1  ; ESP-NOW master
EYE_SYNC_MODE=2  ; ESP-NOW slave

EYE_SYNC_SIDE=0  ; left-style eyelid direction
EYE_SYNC_SIDE=1  ; right-style eyelid direction
```

The `right_eye_slave` environment uses `EYE_SYNC_SIDE=1`, so the eyelid render
is mirrored for the right eye. This is the "butterfly" behavior: the two eyes
use opposite eyelid directions rather than both behaving like left eyes.

## Flashing

The most reliable upload path for the tested ESP32-S3 SuperMini was COM13 with
USB reset handling. `platformio.ini` currently has:

```ini
upload_port = COM13
monitor_port = COM13
upload_speed = 115200
```

Flash standalone:

```powershell
pio run -e esp32-s3-supermini --target upload
```

Flash the left/master board:

```powershell
pio run -e left_eye_master --target upload
```

Flash the right/slave board:

```powershell
pio run -e right_eye_slave --target upload
```

If PlatformIO upload struggles with native USB reset, the known-good manual
factory-image flash pattern is:

```powershell
python "$env:USERPROFILE\.platformio\packages\tool-esptoolpy\esptool.py" `
  --chip esp32s3 --port COM13 --baud 115200 `
  --before usb-reset --after hard-reset `
  write-flash --flash-mode dio --flash-size 4MB --flash-freq 80m `
  0x0 .pio\build\left_eye_master\firmware.factory.bin
```

Change the build folder to `right_eye_slave` or `esp32-s3-supermini` when
flashing those environments manually.

## ESP-NOW Sync Behavior

ESP-NOW uses wireless broadcast. No router is needed and the boards do not need
to connect to Wi-Fi.

Master board:

- Generates the autonomous gaze motion
- Generates blink timing
- Renders its own eye
- Broadcasts final render values to nearby slaves

Slave board:

- Starts ESP-NOW receiver mode
- Draws a centered fallback right eye while waiting for packets
- Switches to synced frames when master packets arrive
- Uses `EYE_SYNC_SIDE=1` for right-eye eyelid direction

The master broadcasts final render values, not just a random seed. That avoids
drift between boards. The packet contains:

- sclera X/Y render offset
- iris scale
- upper eyelid threshold
- lower eyelid threshold
- sequence number

## Serial Monitor Proof

A working standalone or master boot should look like this:

```text
Starting
ESP-NOW master MAC: A0:F2:62:F3:9F:F8
Initialise eye objects
Create display #0
Initialising displays
54
54
```

The repeating numbers are FPS reports. On the tested master board, ESP-NOW
master mode ran at about 54 FPS.

A working slave boot should print its MAC and, if no master is currently
broadcasting, this message:

```text
ESP-NOW slave waiting for master packets
```

The slave should still draw a centered fallback eye while waiting.

## Troubleshooting

If the board boot loops with `Guru Meditation Error` after `Initialising displays`,
check that `User_Setup.h` still has:

```cpp
#define USE_HSPI_PORT
```

If the display is blank in slave mode:

- Confirm the `right_eye_slave` environment was flashed
- Confirm the display wiring matches the table above
- Confirm the master is powered and running `left_eye_master`
- Watch serial output for `ESP-NOW slave waiting for master packets`
- The slave should still show a centered fallback eye even without the master

If the wrong source file seems open in VS Code:

- The compiled source file is `src/dual_tft_7735_animated.cpp`
- There is no `src/main.cpp` in this project
- The Arduino-style helper file `eye_functions.ino` is included into the C++ file

If COM ports change:

```powershell
pio device list
```

Then update these in `platformio.ini`:

```ini
upload_port = COM13
monitor_port = COM13
```

## Files That Matter

| File | Purpose |
| --- | --- |
| `platformio.ini` | Environments, board settings, sync mode flags |
| `User_Setup.h` | TFT_eSPI display driver and GPIO pin setup |
| `config.h` | Eye count, CS pins, rotations, animation options |
| `src/dual_tft_7735_animated.cpp` | Main app, ESP-NOW setup, master/slave sync |
| `eye_functions.ino` | Eye rendering, blink/gaze math, draw loop |

## Current Stability Notes

- TFT SPI frequency is set to 27 MHz for stability.
- 40 MHz may work, but test for display corruption before keeping it.
- DMA is currently off. At roughly 54 FPS per eye, DMA is not needed yet.
- Flash is configured as 4 MB DIO to match the actual tested ESP32-S3 SuperMini.

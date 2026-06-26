# Dual ST7735 Animated Eyes on ESP32-S3 SuperMini

This project drives animated eye graphics on a 1.8 inch ST7735 128 x 160 TFT.
It is configured for an ESP32-S3 SuperMini with one display per board.

The current hardware model is:

- One ESP32-S3 SuperMini per eye
- One 1.8 inch ST7735 TFT per ESP32-S3
- Optional ESP-NOW broadcast sync between boards
- Left board can act as the motion/blink master for bench testing
- Right board can act as a synced slave eye
- Final robot layout can use a third ESP32-S3 SuperMini running WireClaw as the brain
- ESP32D servo controller connects to the WireClaw brain S3 over UART

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
| `left_eye_master` | Bench/test mode. Left eye runs animation and broadcasts sync packets |
| `left_eye_slave` | Final robot mode. Left eye receives ESP-NOW packets from the WireClaw brain |
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

## WireClaw Brain / ESP32D / Eye Sync Plan

The intended final robot layout uses three ESP32-S3 SuperMini boards plus the
ESP32D servo controller.

Recommended roles:

| Board | Job |
| --- | --- |
| WireClaw ESP32-S3 SuperMini | Main brain. Runs WireClaw, decides movement/expression, sends UART commands to ESP32D, and broadcasts ESP-NOW eye packets |
| ESP32D | Servo controller. Receives UART commands from WireClaw S3 and drives the servos |
| Left-eye ESP32-S3 SuperMini | Display-only eye board. Runs `left_eye_slave` and follows ESP-NOW packets |
| Right-eye ESP32-S3 SuperMini | Display-only eye board. Runs `right_eye_slave` and follows ESP-NOW packets |

Preferred signal flow:

```text
WireClaw brain S3 --UART--> ESP32D servo controller --> pan/tilt servos
WireClaw brain S3 --ESP-NOW broadcast--> left-eye S3 + right-eye S3
```

Do not make the ESP32D or the left eye relay ESP-NOW unless there is a strong
reason. The WireClaw brain already knows the intended movement and expression,
so it should send the same timed command directly to both eye display boards.

The WireClaw brain broadcasts eye packets on its connected Wi-Fi channel. The
eye slave builds use `EYE_SYNC_CHANNEL=0`, which makes them hop channels until
they hear the brain, then stay on that channel while packets are fresh.

The validated WireClaw UART bridge pins belong on the WireClaw brain S3, not
on the eye display boards:

| Signal | ESP32-S3 SuperMini | ESP32D |
| --- | --- | --- |
| UART TX from S3 | GPIO1 | GPIO16 / RX2 |
| UART RX to S3 | GPIO2 | GPIO17 / TX2 |
| Ground | GND | GND |

Wire TX to RX and RX to TX:

```text
WireClaw S3 GPIO1 TX  -> ESP32D GPIO16 RX2
WireClaw S3 GPIO2 RX  <- ESP32D GPIO17 TX2
WireClaw S3 GND       <-> ESP32D GND
```

Do not use GPIO20 for this bridge. On the ESP32-S3 SuperMini it is tied to
native USB and should stay free for flashing and serial monitor access.

This eye firmware is display firmware only. That is intentional and does not
block the final robot layout. The eye boards should not run WireClaw, Telegram,
UART, or servo code. The WireClaw brain owns those jobs and sends ESP-NOW
packets compatible with this eye firmware, then both eye boards run as slaves:

```powershell
pio run -e left_eye_slave --target upload
pio run -e right_eye_slave --target upload
```

The old `left_eye_master` environment is still useful for bench testing two eye
boards without the third WireClaw brain installed.

Current checkpoint note: the WireClaw brain now has a linked `jafr_look` tool
that sends UART neck targets and ESP-NOW eye target overrides together. This is
built but still needs all-up hardware verification. See `docs/EVAL_HANDOVER.md`
for the exact evaluation state.

## Full Firmware Bundle

The full JAFR firmware stack is kept in this repo:

| Path | Firmware |
| --- | --- |
| project root | Eye display firmware |
| `firmware/wireclaw-brain` | WireClaw brain ESP32-S3 firmware |
| `firmware/esp32d-servo-bridge` | ESP32D servo bridge firmware |

The subsystem boundary and roadmap are documented in `ARCHITECTURE.md`.

Review-facing subsystem docs:

| Document | Purpose |
| --- | --- |
| `docs/PINOUT.md` | Hardware pin source of truth |
| `docs/PROTOCOL.md` | ESP-NOW, UART, and linked command contracts |
| `docs/ROADMAP.md` | Milestones from current foundation to autonomy |
| `docs/CONFIGURATION_MAP.md` | Current answer to "which file do I edit?" |
| `docs/EVAL_HANDOVER.md` | Current test/evaluation handover |

Build everything from the project root:

```powershell
.\tools\build-all-firmware.ps1
```

Private WireClaw runtime data is backed up locally, outside git, at
`private/wireclaw-brain/data`. Use `.\tools\restore-private-wireclaw-data.ps1`
to copy it back into `firmware/wireclaw-brain/data` before rebuilding or
uploading LittleFS for a brand-new brain board.

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

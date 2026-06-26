# JAFR Evaluation And Handover

This document is the plain-English state of the four-MCU JAFR checkpoint.

## Short Answer

The project is not blocked by the phrase "eye firmware is display firmware
only." That phrase means the eye boards do not run WireClaw, Telegram, UART, or
servo logic. They only draw eyes.

The intended final signal path is:

```text
Telegram / WireClaw chat
        |
        v
WireClaw brain ESP32-S3 SuperMini
        |-- ESP-NOW broadcast --> left eye ESP32-S3 slave display
        |-- ESP-NOW broadcast --> right eye ESP32-S3 slave display
        |
        `-- UART serial_text --> ESP32D servo bridge --> pan/tilt servos
```

That architecture is correct and is already represented in the codebase.

What is already done:

- WireClaw brain firmware broadcasts ESP-NOW eye packets directly to both eye
  boards.
- Left and right eye slave firmware can receive those packets and render synced
  eyes.
- WireClaw brain has UART serial_text support on the ESP32-S3 SuperMini pins.
- ESP32D servo bridge firmware understands `PING`, `STATUS`, `HOME`, `PAN n`,
  `TILT n`, and `MOVE pan tilt`.
- The firmware projects are bundled under this repo.

What is not fully proven yet:

- Physical all-up test with brain + two eyes + ESP32D connected at the same
  time.
- Real servo movement, because the ESP32D bridge currently builds with servo
  output disabled for safe bench testing.
- Physical proof of automatic "look right" coupling. The firmware now has a
  `jafr_look` tool that sends both the UART `MOVE pan tilt` command and an
  ESP-NOW eye target override, but it still needs all-up hardware verification.

## Hardware Roles

| MCU | Firmware | Current job | Status |
| --- | --- | --- | --- |
| Brain ESP32-S3 SuperMini | `firmware/wireclaw-brain` | WireClaw, Telegram, ESP-NOW eye broadcast, UART commands to ESP32D | Built and previously flashed/boot-verified |
| Left eye ESP32-S3 SuperMini | project root `left_eye_slave` | Display-only left eye. Receives ESP-NOW packets | Built and previously display-verified |
| Right eye ESP32-S3 SuperMini | project root `right_eye_slave` | Display-only mirrored right eye. Receives ESP-NOW packets | Built and previously display-verified |
| ESP32D | `firmware/esp32d-servo-bridge` | UART servo bridge / muscle controller | Built, UART protocol implemented, physical servo movement untested |

## Current Behavior To Expect

### If only brain + two eye slaves are powered

Expected:

- Brain joins Wi-Fi.
- Brain prints `ESP-NOW eye sync: broadcasting on WiFi channel X`.
- Both eye boards channel-hop until they hear the brain.
- Both eyes blink together.
- Both eyes move together using the WireClaw brain's autonomous gaze generator.

This does not require the ESP32D.

### If brain + ESP32D are wired

Expected:

- WireClaw can send UART text to the ESP32D through the `c3servo` serial_text
  device.
- `PING` should return `PONG`.
- `STATUS` should return current pan/tilt/output state.
- `PAN n`, `TILT n`, and `MOVE pan tilt` should update the ESP32D internal
  target values.

Important: the ESP32D build currently has `SERVO_OUTPUT_ENABLE=0`, so servo PWM
output is disabled. This is intentional until the mechanics are mounted. With
that build, command parsing can work while servos do not physically move.

### If brain + eyes + ESP32D are all wired

Expected today:

- Eye movement is synced from the brain.
- ESP32D UART command path can be tested separately.
- Telegram/WireClaw can be asked to send serial commands such as `PING`,
  `STATUS`, or `MOVE 120 90`.

Expected after this implementation:

- A natural phrase like "look right" should cause WireClaw to use `jafr_look`,
  which drives both the ESP32D neck target and the ESP-NOW eye target together.

That coupling maps high-level gaze/neck intent to both:

```text
1. UART command to ESP32D, e.g. MOVE 120 90
2. ESP-NOW eye target override, e.g. eyes look right at the same time
```

## Wiring

### Brain ESP32-S3 SuperMini To ESP32D

| Signal | Brain S3 | ESP32D |
| --- | --- | --- |
| UART TX from brain | GPIO1 | GPIO16 / RX2 |
| UART RX to brain | GPIO2 | GPIO17 / TX2 |
| Ground | GND | GND |

Wire TX to RX and RX to TX:

```text
Brain GPIO1 TX  -> ESP32D GPIO16 RX2
Brain GPIO2 RX  <- ESP32D GPIO17 TX2
Brain GND       <-> ESP32D GND
```

Do not use GPIO20 on the ESP32-S3 SuperMini for this bridge. Keep it free for
native USB.

### Eye Displays

Known working ST7735 1.8 inch 128x160 TFT wiring:

```text
MOSI = GPIO12
SCLK = GPIO13
DC   = GPIO10
CS   = GPIO9
RST  = GPIO11
BLK  = -1 / not controlled by firmware
```

## Firmware Locations

| Purpose | Path | Environment |
| --- | --- | --- |
| Standalone eye test | repo root | `esp32-s3-supermini` |
| Left eye slave | repo root | `left_eye_slave` |
| Right eye slave | repo root | `right_eye_slave` |
| WireClaw brain | `firmware/wireclaw-brain` | `esp32-s3` |
| ESP32D servo bridge | `firmware/esp32d-servo-bridge` | `esp32d-servo-bridge` |

## Build

All firmware binaries have been built successfully on this machine. The helper
is:

```powershell
.\tools\build-all-firmware.ps1
```

PlatformIO currently prints slow/non-fatal dependency warnings when PyPI is
unreachable. The important output is whether each environment ends with
`SUCCESS` and produces `firmware.bin`.

Known produced binaries:

```text
.pio\build\esp32-s3-supermini\firmware.bin
.pio\build\left_eye_master\firmware.bin
.pio\build\left_eye_slave\firmware.bin
.pio\build\right_eye_slave\firmware.bin
firmware\wireclaw-brain\.pio\build\esp32-s3\firmware.bin
firmware\esp32d-servo-bridge\.pio\build\esp32d-servo-bridge\firmware.bin
```

## Flash Commands

Check ports first because COM numbers change:

```powershell
pio device list
```

Flash left eye:

```powershell
python -m platformio run -e left_eye_slave --target upload
```

Flash right eye:

```powershell
python -m platformio run -e right_eye_slave --target upload
```

Flash WireClaw brain firmware:

```powershell
cd firmware\wireclaw-brain
python -m platformio run -e esp32-s3 --target upload
```

Flash ESP32D servo bridge:

```powershell
cd firmware\esp32d-servo-bridge
python -m platformio run -e esp32d-servo-bridge --target upload
```

## Private WireClaw Data

The real WireClaw runtime data is backed up locally, but ignored by git:

```text
private/wireclaw-brain/data
```

It contains the real `config.json`, so do not commit it publicly.

To restore that private data into the bundled WireClaw project:

```powershell
.\tools\restore-private-wireclaw-data.ps1
```

Then upload LittleFS from `firmware/wireclaw-brain` if the brain filesystem
needs to be recreated.

## Evaluation Checklist

### 1. Brain Boot

Plug in the brain ESP32-S3 and open serial monitor at 115200.

Pass if serial shows:

```text
WireClaw v0.4.0
WiFi: IP = ...
WiFi: channel = ...
ESP-NOW eye sync: broadcasting on WiFi channel ...
Ready!
```

### 2. Eye Slave Sync

Power both eye boards with slave firmware.

Pass if:

- each TFT shows an eye, not just backlight;
- both eyes blink together;
- both eyes move together;
- right eye is mirrored/butterflied relative to left.

Useful slave serial lines:

```text
ESP-NOW slave channel hop enabled
ESP-NOW slave receiving on channel X
```

### 3. Brain To ESP32D UART

Wire brain GPIO1/GPIO2/GND to ESP32D GPIO16/GPIO17/GND.

From Telegram/WireClaw, ask it to send serial text:

```text
PING
STATUS
MOVE 120 90
```

Pass if ESP32D replies:

```text
PONG
STATUS pan=... tilt=... output=...
OK MOVE pan=120 tilt=90
```

### 4. Servo Output

Do this only after the servos are mechanically safe.

Current safe build:

```ini
SERVO_OUTPUT_ENABLE=0
```

For real servo movement, change the ESP32D bridge build flag to:

```ini
SERVO_OUTPUT_ENABLE=1
```

Then rebuild and flash the ESP32D. Test `HOME`, then small movements first.

### 5. Linked Neck And Eye Movement

This is the next software feature, not a proven checkpoint feature.

Goal:

```text
User says "look right"
WireClaw brain sends UART MOVE command to ESP32D
WireClaw brain also overrides ESP-NOW eye target to the right
Both happen together
```

Implementation options:

- Add a WireClaw tool such as `jafr_look(pan, tilt, eye_x, eye_y)`.
- Or add a serial command convention and an ESP-NOW override state in
  `firmware/wireclaw-brain/src/main.cpp`.

The current brain eye generator is autonomous. It is not yet controlled by
Telegram movement phrases.

## JAFR Telegram Commands

Ask WireClaw:

```text
show JAFR commands
look left
look right
look up
look down
center
home
look up right
move neck to pan 120 tilt 90
send PING to ESP32D
read c3servo
```

The compact UART command box is returned by the `jafr_help` tool:

```text
ESP32D UART: PING, STATUS, HOME, PAN n, TILT n, MOVE pan tilt, ENABLE, DISABLE.
Linked: jafr_look direction=center/left/right/up/down/up_left/up_right/down_left/down_right/home.
```

## Handover Prompt For Another Model

Use this if another model/provider must continue:

```text
We have a PlatformIO repo at:
E:\------------------working----------Main\dual_tft_7735_animated-------------------working\dual_tft_7735_animated

It contains a four-MCU JAFR robot checkpoint:
1. repo root eye firmware for ESP32-S3 SuperMini + ST7735 TFT.
2. firmware/wireclaw-brain for ESP32-S3 SuperMini WireClaw brain.
3. firmware/esp32d-servo-bridge for ESP32D servo controller.

Do not replace TFT_eSPI settings casually. The eye display wiring is:
MOSI 12, SCLK 13, DC 10, CS 9, RST 11, BLK unused.

The intended architecture is:
WireClaw brain -> ESP-NOW broadcast -> both eye slaves.
WireClaw brain GPIO1 TX -> ESP32D GPIO16 RX2.
WireClaw brain GPIO2 RX <- ESP32D GPIO17 TX2.
Shared GND.

Current status:
- Eye slave firmware builds and has shown realistic eyes.
- WireClaw brain builds and broadcasts ESP-NOW autonomous eye packets.
- ESP32D bridge builds and supports PING, STATUS, HOME, PAN n, TILT n, MOVE pan tilt.
- ESP32D servo PWM is disabled by SERVO_OUTPUT_ENABLE=0 until mechanics are safe.
- Private WireClaw data is local/ignored under private/wireclaw-brain/data.

Do not commit private/wireclaw-brain/data.

Recently added feature:
The WireClaw brain has a jafr_look tool. It sends both:
1. UART MOVE pan tilt to ESP32D
2. ESP-NOW eye target override to both eyes
Next work is hardware verification and tuning pan/tilt/eye_x/eye_y mappings.

Before editing, run:
git status --short --branch
rg -n "eyeSyncTick|serial_send|SERVO_OUTPUT_ENABLE|MOVE pan" firmware src README.md
```

# Configuration Map

This project still has configuration in several files because it grew from an
eye sketch into a multi-MCU robot platform. This map is the current answer to
"which file do I edit?"

## Eye Display Hardware

File: `User_Setup.h`

Controls:

- ST7735 driver selection;
- TFT dimensions;
- SPI pins;
- SPI frequency;
- color order.

Do not casually change this file. It contains the known working ESP32-S3
SuperMini + ST7735 setup.

## Eye Firmware Mode

File: root `platformio.ini`

Controls:

- standalone eye;
- bench master;
- left slave;
- right slave;
- ESP-NOW mode flags.

Important envs:

```text
esp32-s3-supermini
left_eye_master
left_eye_slave
right_eye_slave
```

## Eye Behavior Constants

File: `config.h`

Controls:

- eye table;
- blink/wink pins;
- joystick/iris options;
- default eye behavior flags.

## Brain Firmware

File: `firmware/wireclaw-brain/platformio.ini`

Controls:

- WireClaw brain board target;
- LittleFS partition;
- monitor settings.

Private runtime data:

```text
private/wireclaw-brain/data
```

Public examples:

```text
firmware/wireclaw-brain/data/config.json.example
firmware/wireclaw-brain/data/config.json.openrouter.example
firmware/wireclaw-brain/data/config.json.ollama.example
```

Do not commit real secrets.

## ESP32D Servo Bridge

File: `firmware/esp32d-servo-bridge/platformio.ini`

Controls:

- UART baud and pins;
- servo pins;
- servo min/max/home values;
- whether servo output is enabled.

Safe default:

```ini
SERVO_OUTPUT_ENABLE=0
```

Enable only after mechanics are safe:

```ini
SERVO_OUTPUT_ENABLE=1
```

## Review-Driven Future Cleanup

The long-term target is a `settings/` or shared header layout for stable pin and
robot constants. Until that refactor is safe, this file is the configuration
index and `docs/PINOUT.md` is the hardware source of truth.


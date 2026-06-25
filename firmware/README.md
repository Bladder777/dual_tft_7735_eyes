# JAFR Firmware Bundle

This folder keeps the non-eye firmware next to the eye firmware so the whole
hardware stack can be rebuilt from one project folder.

## Projects

| Path | Board | Purpose |
| --- | --- | --- |
| project root | ESP32-S3 SuperMini + ST7735 | Eye display firmware. Build `left_eye_slave` and `right_eye_slave` for the mounted robot eyes. |
| `firmware/wireclaw-brain` | ESP32-S3 SuperMini | WireClaw brain. Sends UART commands to ESP32D and broadcasts ESP-NOW eye packets. |
| `firmware/esp32d-servo-bridge` | ESP32D / ESP32 DevKit | Servo bridge firmware. Receives UART commands from the brain and drives servos when enabled. |

## Build Everything

From the project root:

```powershell
.\tools\build-all-firmware.ps1
```

That builds:

- standalone eye test firmware
- left eye master test firmware
- left eye slave firmware
- right eye slave firmware
- WireClaw brain firmware
- ESP32D servo bridge firmware

## Flashing Notes

COM ports can change. Check the current port first:

```powershell
pio device list
```

The WireClaw brain copy does not include private `data/config.json`. That is
intentional. Firmware-only reflashes preserve the existing LittleFS config on
the brain. For a brand-new brain, use the setup portal or copy one of the
example config files and fill in the private values locally.

This workstation also has an ignored local backup of the real WireClaw data at:

```text
private/wireclaw-brain/data
```

That folder is not committed to git because it contains Wi-Fi/API/Telegram
configuration. If the brain filesystem must be rebuilt, restore the private
data into the bundled WireClaw project with:

```powershell
.\tools\restore-private-wireclaw-data.ps1
```

Then upload LittleFS from `firmware/wireclaw-brain`.

The ESP32D bridge currently builds with `SERVO_OUTPUT_ENABLE=0`, so it is safe
to flash before the servos are attached. Enable servo output only when the
mechanics are mounted and ready.

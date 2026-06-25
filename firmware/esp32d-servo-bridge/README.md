# JAFR ESP32D Servo Bridge

Firmware for the ESP32D servo controller that sits between the WireClaw brain
ESP32-S3 SuperMini and the pan/tilt servos.

## Wiring

Brain S3 to ESP32D:

```text
Brain S3 GPIO1 TX  -> ESP32D GPIO16 RX2
Brain S3 GPIO2 RX  <- ESP32D GPIO17 TX2
Brain S3 GND       <-> ESP32D GND
```

Servo outputs, when enabled:

```text
PAN servo signal  -> ESP32D GPIO25
TILT servo signal -> ESP32D GPIO26
Servo power       -> external 5 V supply
Servo GND         -> same ground as ESP32D and brain S3
```

Do not power servos from the ESP32 board.

## Safe Default

`SERVO_OUTPUT_ENABLE=0` in `platformio.ini`.

That means the firmware parses commands and replies over UART, but it does not
attach servos or drive PWM outputs. This is the safe mode for testing without
servos attached.

Tomorrow, after the servos and external power are wired, change:

```ini
-D SERVO_OUTPUT_ENABLE=1
```

## Commands

All commands are newline-delimited text at 9600 baud on ESP32D UART2.

```text
PING
STATUS
HOME
PAN 90
TILT 90
MOVE 90 90
ENABLE
DISABLE
HELP
```

Expected replies:

```text
PONG
STATUS pan=90 tilt=90 output=off
OK PAN 90
OK TILT 90
OK MOVE pan=90 tilt=90
ERR ...
```

## Build

```powershell
pio run -e esp32d-servo-bridge
```


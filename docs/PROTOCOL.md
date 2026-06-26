# JAF-R Protocols

This document defines the current communication contracts between subsystems.
Leaf controllers should stay simple; the brain coordinates behavior.

## Brain To Eye Boards

Transport: ESP-NOW broadcast.

Sender: `firmware/wireclaw-brain`

Receivers:

- root firmware env `left_eye_slave`
- root firmware env `right_eye_slave`

Packet intent:

```text
magic/version
side
sequence
scleraX
scleraY
irisScale
upperThreshold
lowerThreshold
```

Current constants:

```text
magic   = 0xE773
version = 1
```

Behavior:

- Brain broadcasts on its current Wi-Fi channel.
- Eye slaves use `EYE_SYNC_CHANNEL=0` and hop channels until packets arrive.
- If packets are fresh, slaves render packet values.
- If packets time out, slaves show centered fallback eye.

Review invariant:

The eye boards must not depend on Telegram, WireClaw, UART, servo state, camera
state, or cloud access. They render what the brain broadcasts.

## Brain To ESP32D

Transport: UART serial text.

Baud: 9600.

Wiring:

```text
Brain GPIO1 TX  -> ESP32D GPIO16 RX2
Brain GPIO2 RX  <- ESP32D GPIO17 TX2
Brain GND       <-> ESP32D GND
```

ESP32D command set:

```text
PING
STATUS
HOME
PAN n
TILT n
MOVE pan tilt
ENABLE
DISABLE
HELP
```

Expected replies:

```text
PONG
STATUS pan=90 tilt=90 output=off attached=no
OK HOME pan=90 tilt=90
OK PAN 120
OK TILT 80
OK MOVE pan=120 tilt=90
OK OUTPUT OFF
ERR ...
```

The WireClaw brain should register the ESP32D as:

```text
name: c3servo
type: serial_text
baud: 9600
```

## Linked Command Contract

Tool: `jafr_look`

Owner: brain.

Purpose: one high-level command controls both eyes and neck.

Actions:

```text
1. Override ESP-NOW eye target for a short hold window.
2. Send UART MOVE pan tilt to ESP32D.
```

Supported directions:

```text
center
home
left
right
up
down
up_left
up_right
down_left
down_right
```

Custom target:

```text
jafr_look(pan=120, tilt=90, eye_x=844, eye_y=512, hold_ms=1800)
```

The command can still move eyes if UART is not active. Tool output reports
whether neck UART was sent.

## Command Cheat Sheet

Tool: `jafr_help`

Returns:

```text
ESP32D UART: PING, STATUS, HOME, PAN n, TILT n, MOVE pan tilt, ENABLE, DISABLE.
Linked: jafr_look direction=center/left/right/up/down/up_left/up_right/down_left/down_right/home.
```

This is intentionally not stored in `/memory.txt`; it is firmware behavior.


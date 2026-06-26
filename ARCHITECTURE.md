# JAF-R Architecture

JAF-R is moving from separate firmware sketches into a modular robotics
platform. The system uses dedicated microcontrollers for brain, eyes, and
motion so each subsystem can stay simple and reliable.

## Current Platform Boundary

```text
Telegram / WireClaw chat
        |
        v
Brain ESP32-S3 SuperMini
        |-- ESP-NOW broadcast --> Left eye ESP32-S3 + TFT
        |-- ESP-NOW broadcast --> Right eye ESP32-S3 + TFT
        |
        `-- UART serial_text --> ESP32D servo bridge --> neck servos
```

## Subsystems

| Subsystem | Current owner | Responsibility | Current status |
| --- | --- | --- | --- |
| Brain | ESP32-S3 SuperMini running WireClaw | Telegram, LLM/tool loop, high-level intent, ESP-NOW eye sync, UART commands to motion controller | Implemented, needs all-up hardware verification |
| Eyes | Two ESP32-S3 SuperMini boards with ST7735 TFTs | Render display-only eye frames from ESP-NOW packets | Implemented and visually verified |
| Neck | ESP32D servo bridge | Receive UART commands and drive pan/tilt servos | UART protocol implemented; servo output still safe-disabled |
| Vision | Future camera subsystem | Camera capture, target detection, face/object tracking | Not started |
| Speech | Future audio subsystem | Wake/listen/speak pipeline | Not started |
| Navigation | Future mobility subsystem | Base movement, obstacle checks, movement goals | Not started |
| Memory/personality | WireClaw files plus future store | Persistent facts, preferences, behavior style | Basic WireClaw memory exists |

## Contracts

### Brain To Eyes

Transport: ESP-NOW broadcast.

The brain sends render-ready packets to both eye boards. Eye boards do not run
WireClaw, Telegram, UART, servo code, camera code, or decision logic.

Current packet intent:

```text
magic/version
sequence
scleraX/scleraY
irisScale
upper/lower eyelid thresholds
```

The right eye firmware mirrors eyelid direction with `EYE_SYNC_SIDE=1`.

### Brain To Neck

Transport: UART serial_text.

Wiring:

```text
Brain GPIO1 TX  -> ESP32D GPIO16 RX2
Brain GPIO2 RX  <- ESP32D GPIO17 TX2
Brain GND       <-> ESP32D GND
```

ESP32D command contract:

```text
PING
STATUS
HOME
PAN n
TILT n
MOVE pan tilt
ENABLE
DISABLE
```

Current safe build has servo output disabled:

```ini
SERVO_OUTPUT_ENABLE=0
```

Set it to `1` only when the mechanics are mounted and safe.

### Linked Eye + Neck Movement

The brain owns coordination. The eye boards and ESP32D do not coordinate with
each other directly.

Current linked tool:

```text
jafr_look(direction=...)
```

It does both:

```text
1. UART MOVE pan tilt to ESP32D
2. ESP-NOW eye target override to both eye boards
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

Custom targets can use:

```text
jafr_look(pan=120, tilt=90, eye_x=844, eye_y=512)
```

## Firmware Layout

| Path | Purpose |
| --- | --- |
| project root | Eye firmware for standalone, left slave, right slave, and bench master |
| `firmware/wireclaw-brain` | Brain firmware |
| `firmware/esp32d-servo-bridge` | Neck/muscle bridge firmware |
| `private/wireclaw-brain/data` | Local ignored backup of real WireClaw runtime data |
| `docs/EVAL_HANDOVER.md` | Current evaluation state and handover notes |
| `docs/PINOUT.md` | Hardware pin source of truth |
| `docs/PROTOCOL.md` | ESP-NOW, UART, and linked command contracts |
| `docs/ROADMAP.md` | Milestone plan |
| `docs/CONFIGURATION_MAP.md` | Where each kind of configuration currently lives |

## Roadmap

### Milestone 1: Foundation

Status: mostly complete.

- Realistic eyes on TFTs.
- ESP-NOW eye sync from brain to both eyes.
- UART bridge from brain to ESP32D.
- ESP32D command protocol.
- Linked `jafr_look` command path.

Remaining validation:

- Flash latest brain firmware and updated LittleFS prompt.
- All-up test with brain + both eyes + ESP32D.
- Verify `jafr_help`, `look left`, `look right`, `PING`, and `STATUS`.

### Milestone 2: Motion Safety

- Mount neck hardware.
- Enable servo output.
- Test `HOME`.
- Test small pan/tilt moves.
- Tune pan/tilt ranges and eye target mapping.

### Milestone 3: Expression Engine

Define named expressions as brain-level state:

```text
neutral
curious
happy
sleepy
alert
confused
annoyed
thinking
```

Expression state should map to:

- blink rate;
- eyelid openness;
- iris scale;
- gaze behavior;
- optional neck pose.

### Milestone 4: Vision

Add a camera subsystem with a narrow contract:

```text
vision target: none | face | object
target position: x/y/confidence
tracking mode: idle | follow | inspect
```

The brain should consume target coordinates and convert them into `jafr_look`
or future smoother tracking commands.

### Milestone 5: Voice

Add speech as a separate pipeline:

```text
mic input -> speech-to-text -> WireClaw intent -> action -> text-to-speech
```

Keep voice independent from eye/neck firmware.

### Milestone 6: Autonomous Behavior

Autonomy should be policy/state in the brain, not hidden inside device firmware.

Examples:

- idle scanning;
- look toward speaker;
- blink when thinking;
- glance toward motion;
- return to neutral after action.

### Milestone 7: Memory And Personality

Keep memory layered:

- short-term runtime state;
- persistent user facts;
- robot personality settings;
- skill/task history.

Do not store secrets in normal memory. Keep secrets in ignored private config or
encrypted backup.

## Development Rule

Before adding a feature, decide which subsystem owns it and what contract it
uses. Avoid making eyes, neck, vision, or speech talk directly to each other.
The brain coordinates; the leaf controllers execute.

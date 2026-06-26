# JAF-R Roadmap

This roadmap keeps feature work aligned with subsystem boundaries. Do not add a
feature until its owner and interface are clear.

## Current Foundation

Status: implemented, hardware verification still in progress.

- Realistic TFT eyes.
- Separate left and right eye ESP32-S3 boards.
- WireClaw brain ESP32-S3.
- ESP-NOW eye packet broadcast from brain.
- ESP32D UART servo bridge.
- `jafr_help` command cheat sheet.
- `jafr_look` linked eye+neck command path.

## Milestone 1: All-Up Verification

Goal: prove the current four-MCU foundation works together.

Checklist:

- Brain boots and joins Wi-Fi.
- Telegram reaches WireClaw.
- Brain prints ESP-NOW broadcast line.
- Left eye receives packets.
- Right eye receives packets and mirrors correctly.
- ESP32D replies to `PING`.
- ESP32D replies to `STATUS`.
- `jafr_look(direction=right)` moves eyes and sends UART `MOVE`.

Exit criteria:

- Evidence recorded in `docs/EVAL_HANDOVER.md` or a dated test log.

## Milestone 2: Neck Motion Safety

Goal: enable real pan/tilt movement without damaging hardware.

Steps:

- Mount servos.
- Provide external servo power.
- Confirm shared ground.
- Rebuild ESP32D with `SERVO_OUTPUT_ENABLE=1`.
- Test `HOME`.
- Test small range moves.
- Tune min/max degrees.

Exit criteria:

- `HOME`, `PAN`, `TILT`, and `MOVE` physically work.
- Safe min/max values are documented.

## Milestone 3: Expression Engine

Goal: move from raw eye positions to named expression states.

Initial expressions:

- neutral
- curious
- happy
- sleepy
- alert
- confused
- thinking

Expression outputs:

- gaze behavior;
- blink timing;
- eyelid openness;
- iris scale;
- optional neck pose.

Owner: brain.

Eye boards still receive render-ready packets.

## Milestone 4: Camera / Vision

Goal: add perception without coupling camera logic into eye or servo firmware.

Interface:

```text
target_type = none | face | object | motion
target_x
target_y
confidence
tracking_mode = idle | follow | inspect
```

Brain consumes target state and decides whether to call linked movement.

## Milestone 5: Voice Pipeline

Goal: voice interaction as an input/output layer for WireClaw.

Pipeline:

```text
microphone -> speech-to-text -> WireClaw intent -> action -> text-to-speech
```

Voice should not directly command servos or eyes. It asks the brain.

## Milestone 6: Autonomous Behaviors

Goal: idle life and simple self-directed behavior.

Examples:

- idle scanning;
- blink while thinking;
- glance toward recent motion;
- return to neutral after command;
- look toward speaker;
- periodic self-status.

Autonomy remains a brain policy layer.

## Milestone 7: Memory And Personality

Goal: persistent identity and preferences without leaking secrets.

Layers:

- runtime state;
- short user facts;
- personality settings;
- long-term task history;
- private secrets/config.

Secrets belong in ignored private config or encrypted backup, not in normal
memory.


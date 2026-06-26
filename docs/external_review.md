# External Review Snapshot

Original assessment:

- Engineering score: 8.6 / 10
- JAF-R roadmap readiness: 9.2 / 10

Summary:

The project has moved beyond an Arduino demo and is becoming a hardware
platform. The strongest decision is separating hardware domains instead of
forcing one ESP32 to handle display, servos, Wi-Fi, Telegram, speech, sensors,
and future autonomy.

Current architecture:

```text
Brain / WireClaw ESP32-S3
  |-- UART    -> Servo Controller / ESP32D
  |-- ESP-NOW -> Left Eye ESP32-S3
  `-- ESP-NOW -> Right Eye ESP32-S3
```

## Strengths

### Hardware Separation

The project now separates:

- brain;
- motion;
- eyes.

That scales better than a single-controller design.

### Repository Layout

The repository is becoming understandable:

- firmware bundle;
- data/config examples;
- README;
- dedicated docs.

### Documentation

The README explains:

- hardware;
- wiring;
- build modes;
- display setup;
- PlatformIO environments.

### Forward Compatibility

The project describes a path from standalone eye testing to bench master/slave
testing and then final robot brain/slave layout.

## High-Priority Improvements From Review

### 1. Configuration Is Still Scattered

Config currently exists in multiple places:

- `config.h`
- `User_Setup.h`
- root `platformio.ini`
- firmware build flags

Review recommendation:

Eventually move toward a `settings/` structure:

```text
settings/display.h
settings/pins.h
settings/robot.h
settings/network.h
```

Current response:

- Added `docs/CONFIGURATION_MAP.md`.
- Added `docs/PINOUT.md`.
- Deferred source refactor until hardware validation is stable.

### 2. Eye Rendering Is Still Tightly Coupled

The eye code still resembles the original Adafruit architecture. It works, but
future expression work should separate:

```text
EyeRenderer
EyeController
Network
Expressions
```

Current response:

- Kept working eye firmware stable.
- Documented the future expression engine in `ARCHITECTURE.md` and
  `docs/ROADMAP.md`.

### 3. Pin Definitions Need One Source Of Truth

Review recommendation:

Centralize display, UART, servo, sensor, SPI, and I2C pins.

Current response:

- Added `docs/PINOUT.md` as the current hardware source of truth.
- Kept code-level consolidation as future work to avoid breaking validated
  hardware mappings.

### 4. Build Environments Should Become Self-Documenting

Review recommendation:

Future environment names:

```text
brain
left_eye
right_eye
servo_controller
display_test
factory_test
```

Current response:

- Existing environments remain to avoid disrupting known flash commands.
- Firmware layout is documented in `ARCHITECTURE.md`,
  `docs/CONFIGURATION_MAP.md`, and `docs/EVAL_HANDOVER.md`.

### 5. Add Protocol Documentation

Review recommendation:

Document the Brain <-> Servo <-> Eyes protocol.

Current response:

- Added `docs/PROTOCOL.md`.

## Roadmap Alignment

Current state:

- Eyes: implemented.
- Neck interface: implemented at UART protocol level.
- UART bridge: implemented.
- ESP-NOW direction: implemented.
- WireClaw integration: implemented.

Next logical milestones:

1. all-up hardware verification;
2. servo output and neck safety;
3. expression engine;
4. camera subsystem;
5. vision tracking;
6. voice pipeline;
7. autonomous behaviors;
8. memory/personality layer.

See `docs/ROADMAP.md`.

## Review Conclusion

The architecture of dedicated brain, eyes, and motion controllers is sound and
aligned with the long-term JAF-R vision. The most important transition is now
stable subsystem contracts, not feature sprawl.


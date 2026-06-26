# External Review Action Status

This file maps external review feedback to repo changes.

| Review item | Status | Evidence |
| --- | --- | --- |
| Create top-level architecture doc | Done | `ARCHITECTURE.md` |
| Define Brain/Eyes/Neck/Vision/Speech/Navigation responsibilities | Done | `ARCHITECTURE.md` |
| Define subsystem communication paths | Done | `ARCHITECTURE.md`, `docs/PROTOCOL.md` |
| Add protocol documentation | Done | `docs/PROTOCOL.md` |
| Add pin source of truth | Done | `docs/PINOUT.md` |
| Clarify scattered configuration | Done | `docs/CONFIGURATION_MAP.md` |
| Add roadmap | Done | `docs/ROADMAP.md` |
| Clarify linked eye + neck command state | Done | `docs/EVAL_HANDOVER.md`, `docs/PROTOCOL.md` |
| Preserve working hardware mappings | Done | No risky firmware pin refactor before validation |
| Avoid committing secrets | Done | Real data remains under ignored `private/` |

## Remaining Technical Debt

These are intentionally not rushed before hardware validation:

- Move hardware constants into shared code-level `settings/` headers.
- Rename PlatformIO environments to final product names.
- Split eye rendering into renderer/controller/expression modules.
- Add dated hardware test logs after all-up validation.

## Current Review Position

The repo now has the architecture and interface-contract layer requested by the
external review. The next score-limiting factor is not documentation structure;
it is physical verification evidence for the four-MCU system.


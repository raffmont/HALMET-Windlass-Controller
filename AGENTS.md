# Repository Guidelines

This repository contains PlatformIO firmware for a HALMET-based windlass chain
counter and guarded remote controller. Keep changes aligned with the
`hatlabs/HALMET-example-firmware` programming model unless a task explicitly
requests otherwise.

## Programming Model

- Use SensESP for WiFi onboarding, OTA, Signal K connection handling, web
  configuration, `SKOutput*` publishing, `SKValueListener` subscriptions, and
  cooperative scheduling through `event_loop()->onRepeat()`.
- Avoid replacing the firmware with a standalone Arduino `WebServer` or
  WebSocket loop. If a feature exists in another implementation, port the
  behavior into the SensESP graph/event-loop style used here.
- Keep hardware helpers in the existing HALMET files under `src/` when they
  match the board abstraction already present in the project.
- Keep long-lived configuration in SensESP `ConfigItem` objects. Use ESP32 NVS
  only for runtime counters or state that is not naturally a web UI setting.
- Keep anchor-watch and GPS behavior in the SensESP/event-loop model. Do not add
  a standalone GPS polling loop or web server for this feature.

## Safety

- Treat relay control as safety-critical.
- Never allow both UP and DOWN relays to be energized at the same time.
- STOP must remain immediate and idempotent.
- Preserve the dead-man timeout, no-pulse stall stop, near-zero retrieval
  limit, and manual-control mutual exclusion behavior.
- Anchor watch may publish alarms but must never command windlass relays.
- Do not document or implement any behavior that bypasses the original breaker,
  fuses, contactor protection, manual controls, or emergency stop.

## Build And Verification

- Primary verification command:

```bash
/Users/raffaelemontella/.platformio/penv/bin/platformio run
```

- Keep generated `.pio/` output out of source control.
- When touching firmware logic, update the relevant documentation in `docs/`
  and the main `README.md`.

## Documentation

- Keep docs describing the firmware that is actually present in this repository.
  Do not copy REST API, helper plugin, or tooling docs from other repositories
  unless those artifacts are also added here.
- Prefer concrete defaults and Signal K paths from `include/config.h` and
  `include/signalk_paths.h`.
- Use ASCII text unless an existing file already uses another character set.

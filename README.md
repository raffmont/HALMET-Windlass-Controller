# HALMET Windlass Controller

HALMET firmware for an anchor windlass chain counter and guarded remote
controller using the [HALMET microcontroller](https://shop.hatlabs.fi/products/halmet).
It requires electrical access to read the windlass UP/DOWN control signals and
the chain counter sensor.

The project follows the programming model of
[hatlabs/HALMET-example-firmware](https://github.com/hatlabs/HALMET-example-firmware):
SensESP provides onboarding, OTA, configuration, Signal K integration, and the
cooperative event loop.

The current firmware ports the windlass features from
[raffmont/halmet-windlass-controller-new](https://github.com/raffmont/halmet-windlass-controller-new)
without replacing the SensESP/HALMET structure.

## Features

- GP2-style chain pulse counting on HALMET D1.
- Default calibration: `1 pulse = 0.33 m`.
- Persistent pulse counter in ESP32 NVS.
- SensESP web configuration at `/Windlass/Configuration`.
- Configurable meters-per-pulse, debounce, safety timings, relay polarity,
  free-fall thresholds, seafloor thresholds, and Signal K paths.
- Manual UP/DOWN sense inputs.
- Guarded UP/DOWN relay outputs with mutual exclusion.
- Dead-man timeout for remote commands.
- No-pulse stall stop while remotely commanded.
- Near-zero retrieval limit.
- D4 local counter reset.
- Signal K status, fault, event, and command paths.
- Free-fall, anchor-detected, seafloor-detected, and anchor-alarm-suggested
  events.
- Anchor watch using local UART GNSS, Signal K `navigation.position`, or
  incoming NMEA 2000 position PGNs, with automatic deployment arming, automatic
  retrieval disarming, persistent centre/radius state, and Signal K status
  outputs.
- OLED status display when present, including anchor-watch state and selected
  position source. Display rows update only when their values change.
- NMEA 2000 windlass telemetry PGNs `128776`, `128777`, and `128778`.

## Repository Layout

```text
.
├── include/        Windlass configuration, default paths, and state model
├── src/            HALMET firmware and board helper code
├── docs/           Architecture, configuration, Signal K, NMEA 2000, safety
├── platformio.ini  PlatformIO build configuration
└── README.md
```

## Build

Install PlatformIO, then run:

```bash
platformio run
```

Upload:

```bash
platformio run -t upload
```

Serial monitor:

```bash
platformio device monitor
```

The project default environment is `halmet`.

## Default Hardware Mapping

| Function | HALMET / ESP32 |
| --- | --- |
| Chain pulse sensor | D1 / `GPIO23` |
| Manual UP sense | D2 / `GPIO25` |
| Manual DOWN sense | D3 / `GPIO27` |
| Counter reset | D4 / `GPIO26` |
| UP relay output | `GPIO17` |
| DOWN relay output | `GPIO16` |
| I2C SDA/SCL | `GPIO21` / `GPIO22` |
| NMEA 2000 CAN TX/RX | `GPIO19` / `GPIO18` |

The default relay configuration is for optocoupled high-level-trigger relay
modules: `relay_active_high = true`, GPIO HIGH energizes the relay input, and
GPIO LOW keeps it off. Keep `relay_active_high` enabled for those modules.

## Signal K Commands

The default remote command path is:

```text
anchoring.windlass.command.request
```

Accepted string values:

```text
up
down
stop
reset
zero
faults clear
```

`up` and `down` are dead-man commands and must be refreshed before
`command_deadman_ms` expires. A UI should send `stop` on release, page blur, or
network shutdown. Unknown command strings are rejected, stop the relays, and set
`FaultInvalidCommand`.

## Documentation

- [Getting started](docs/getting_started.md)
- [Architecture](docs/architecture.md)
- [SensESP configuration](docs/sensesp_configuration.md)
- [Signal K interface](docs/signalk.md)
- [Signal K web app plugin](signalk/README.md)
- [Free-fall and anchor events](docs/free_fall_anchor_events.md)
- [Anchor watch](docs/anchor_watch.md)
- [GPS/GNSS](docs/gps.md)
- [NMEA 2000](docs/nmea2000.md)
- [Hardware notes](docs/hardware.md)
- [Safety notes](docs/safety.md)
- [Commissioning checklist](docs/commissioning.md)

## Safety

This project can command high-current deck machinery. Keep the original windlass
breaker, fuses, contactor protection, manual controls, and emergency stop
authoritative. Bench-test with relay outputs disconnected before connecting the
real windlass control circuit.

The software is provided without warranty and must be validated by the installer
for the vessel, windlass, wiring, and operating procedures.

## Credits

Inspired by `raffmont/SensESP-Windlass-Controller`, the SensESP chain counter
examples, `raffmont/halmet-windlass-controller-new`, and
`hatlabs/HALMET-example-firmware`.

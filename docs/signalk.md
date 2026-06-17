# Signal K Interface

Signal K is the primary network interface. Values are published through SensESP
`SKOutput*` objects, and remote commands are consumed with `StringSKListener`.
Anchor watch can also subscribe to a Signal K position path when
`gps_position_source = signalk`.

## Default Paths

| Path | Type | Unit | Description |
| --- | --- | --- | --- |
| `anchoring.windlass.rode.length` | number | m | Deployed chain/rode length. |
| `anchoring.windlass.rode.speed` | number | m/s | Signed speed estimate. Positive is deploying, negative is retrieving. |
| `anchoring.windlass.direction` | string | - | `stopped`, `retrieving`, `deploying`, or `unknown`. |
| `anchoring.windlass.state` | string | - | Active command when commanded, otherwise direction. |
| `anchoring.windlass.command.status` | string | - | Applied command: `stop`, `up`, or `down`. |
| `anchoring.windlass.command.request` | string | - | Remote command subscription path. |
| `anchoring.windlass.counter.pulses` | integer | pulses | Raw net pulse count. |
| `anchoring.windlass.counter.metersPerPulse` | number | m/pulse | Active calibration. |
| `anchoring.windlass.faults` | integer | bitmask | Firmware fault flags. |
| `anchoring.windlass.mode` | string | - | `idle`, `retrieving`, `deploying`, `free_fall`, `seafloor`, or `fault`. |
| `anchoring.windlass.freeFall.detected` | boolean | - | Latched free-fall detection. |
| `anchoring.windlass.anchor.detected` | boolean | - | Latched anchor deployment event. |
| `anchoring.windlass.anchor.seafloorDetected` | boolean | - | Latched seafloor estimate. |
| `anchoring.windlass.event` | string | - | Latest event name. |
| `notifications.anchoring.windlass` | string | - | Advisory event/fault message. |
| `anchoring.anchorWatch.enabled` | boolean | - | Anchor-watch configuration state. |
| `anchoring.anchorWatch.autoArm` | boolean | - | Auto-arm configuration state. |
| `anchoring.anchorWatch.state` | string | - | `disabled`, `waitingForGps`, `ready`, `armed`, `alarm`, `suspended`, or `fault`. |
| `anchoring.anchorWatch.radius` | number | m | Active alarm radius. |
| `anchoring.anchorWatch.distance` | number | m | Current distance from stored centre. |
| `anchoring.anchorWatch.margin` | number | m | Distance minus radius. |
| `anchoring.anchorWatch.position.latitude` | number | deg | Stored anchor-watch centre latitude. |
| `anchoring.anchorWatch.position.longitude` | number | deg | Stored anchor-watch centre longitude. |
| `anchoring.anchorWatch.rodeLengthAtArm` | number | m | Deployed rode when the watch armed. |
| `anchoring.anchorWatch.gnss.present` | boolean | - | Selected position source has supplied a position. |
| `anchoring.anchorWatch.gnss.interface` | string | - | Current position source: `uart`, `signalk`, `nmea2000`, or `disabled`. |
| `anchoring.anchorWatch.gnss.fixValid` | boolean | - | Fix satisfies age, HDOP, satellite, and stability gates. |
| `anchoring.anchorWatch.gnss.hdop` | number | - | Current HDOP. |
| `anchoring.anchorWatch.gnss.satellites` | integer | - | Satellites reported by GGA, or the configured quality threshold for external sources. |
| `anchoring.anchorWatch.gnss.position.latitude` | number | deg | Current selected-source latitude. |
| `anchoring.anchorWatch.gnss.position.longitude` | number | deg | Current selected-source longitude. |
| `notifications.anchoring.anchorWatch` | string | - | Compact JSON notification payload for anchor watch. |

## Subscribed Position

When `gps_position_source = signalk`, the firmware subscribes to
`gps_sk_position_path`, which defaults to:

```text
navigation.position
```

The value must contain `latitude` and `longitude` in decimal degrees. The
selected position is then republished under the anchor-watch GNSS status paths.

## Commands

Send string values to `anchoring.windlass.command.request`:

```text
up
down
stop
reset
zero
faults clear
```

`up` and `down` are dead-man commands. A UI should refresh the command before
`command_deadman_ms` expires and send `stop` on button release, page blur,
network loss, or application shutdown.

`stop` is always accepted. `reset` and `zero` set the chain counter to zero.
`faults clear` clears the current fault bitmask.

Unknown command strings are rejected, stop the relays, and set
`FaultInvalidCommand`.

## Recommended UI Behavior

- Use press-and-hold controls for UP and DOWN.
- Refresh active commands every 300-500 ms.
- Always send STOP on release.
- Show the `faults`, `mode`, and `event` paths near controls.
- Do not provide unattended automatic windlass movement without separate,
  tested vessel-level safety logic.

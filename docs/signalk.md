# Signal K Interface

Signal K is the primary network interface. Values are published through SensESP
`SKOutput*` objects, and remote commands are consumed with `StringSKListener`.

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

## Recommended UI Behavior

- Use press-and-hold controls for UP and DOWN.
- Refresh active commands every 300-500 ms.
- Always send STOP on release.
- Show the `faults`, `mode`, and `event` paths near controls.
- Do not provide unattended automatic windlass movement without separate,
  tested vessel-level safety logic.

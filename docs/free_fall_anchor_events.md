# Free-Fall And Anchor Event Detection

The firmware detects deployment events from GP2 chain-counter pulses and the
current command/sense state.

## Events

| Event | Meaning |
| --- | --- |
| `anchor_detected` | Deployed length passed `anchor_detected_length_m` while deployment motion was present. |
| `free_fall_detected` | Chain paid out with no DOWN relay command and no manual DOWN sense input. |
| `seafloor_detected` | Deployment/free-fall was active and pulses stopped after the configured deployed length. |
| `anchor_alarm_suggested` | Published after seafloor detection as an operator prompt. |

## Detection Rules

```text
anchor_detected:
  rode_length_m >= anchor_detected_length_m
  and deployment motion is present

free_fall_detected:
  no DOWN relay command
  no manual DOWN sense input
  no UP relay command or manual UP sense input
  chain is paying out
  speed >= free_fall_min_speed_m_s
  uncommanded deployment pulses >= free_fall_min_pulses

seafloor_detected:
  anchor has been detected
  deployment/free-fall context is active
  rode_length_m >= seafloor_min_length_m
  no chain pulse for seafloor_no_pulse_ms
  active command is not UP
```

## Defaults

```text
free_fall_detection_enabled = true
seafloor_detection_enabled  = true
free_fall_min_speed_m_s     = 0.20
free_fall_min_pulses        = 2
seafloor_no_pulse_ms        = 1800
seafloor_min_length_m       = 1.00
anchor_detected_length_m    = 0.33
```

For the default GP2 calibration, one pulse is `0.33 m`. With
`free_fall_min_pulses = 2`, about `0.66 m` of uncommanded payout is required
before free-fall is latched.

## Limits

These events are heuristic. They infer chain movement and likely seabed contact
from pulse timing; they do not measure seabed contact directly. Validate the
thresholds during commissioning for the installed windlass, gypsy, rode, anchor,
and vessel behavior.

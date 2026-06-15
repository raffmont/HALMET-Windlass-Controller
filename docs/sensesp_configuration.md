# SensESP Configuration

The firmware registers one windlass-specific SensESP `ConfigItem`:

```text
/Windlass/Configuration
```

The object is implemented in `include/windlass_config.h` and persisted through
SensESP `FileSystemSaveable` storage. The pulse counter itself is stored in ESP32
NVS under the `windlass` namespace because it is runtime state rather than UI
configuration.

## Editable Fields

| Field | Default | Runtime effect |
| --- | ---: | --- |
| `meters_per_pulse` | `0.33` | Converts GP2 pulses to deployed metres. Applied live. |
| `chain_pulse_debounce_ms` | `75` | Debounces the GP2 dry contact. Copied to an ISR-safe volatile value. |
| `command_deadman_ms` | `1200` | Maximum age of a refreshed remote UP/DOWN command. |
| `stall_detect_ms` | `3500` | Stops a remote command if no pulses arrive while running. |
| `min_safe_length_m` | `0.50` | Prevents remote UP/retrieve below this deployed length. |
| `relay_active_high` | `true` | Selects relay module input polarity. |
| `free_fall_detection_enabled` | `true` | Enables uncommanded payout detection. |
| `seafloor_detection_enabled` | `true` | Enables pulse-stop seafloor estimate. |
| `free_fall_min_speed_m_s` | `0.20` | Minimum speed for free-fall detection. |
| `free_fall_min_pulses` | `2` | Minimum uncommanded pulses before latching free-fall. |
| `seafloor_no_pulse_ms` | `1800` | No-pulse duration used to estimate seafloor contact. |
| `seafloor_min_length_m` | `1.00` | Minimum deployed length before seafloor detection. |
| `anchor_detected_length_m` | `0.33` | Length threshold for initial anchor deployment. |

## Signal K Path Fields

The same configuration object stores editable Signal K paths:

```text
signalk_paths.rode_length
signalk_paths.rode_speed
signalk_paths.direction
signalk_paths.state
signalk_paths.command_status
signalk_paths.command_request
signalk_paths.pulses
signalk_paths.meters_per_pulse
signalk_paths.faults
signalk_paths.mode
signalk_paths.freefall_detected
signalk_paths.anchor_detected
signalk_paths.seafloor_detected
signalk_paths.event
signalk_paths.notification
```

Defaults are defined in `include/signalk_paths.h`.

## Operational Notes

- SensESP owns the web setup UI and Signal K connection workflow.
- Permanent calibration should be done through the SensESP configuration page.
- The D4 reset input and Signal K `reset`/`zero` commands set the pulse count to
  zero.
- Relay polarity must be tested with the windlass contactor disconnected before
  motor tests.

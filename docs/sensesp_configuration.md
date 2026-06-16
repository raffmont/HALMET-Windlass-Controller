# SensESP Configuration

The firmware registers one windlass-specific SensESP `ConfigItem`:

```text
/Windlass/Configuration
```

The object is implemented in `include/windlass_config.h` and persisted through
SensESP `FileSystemSaveable` storage. The web UI schema uses flat field names
because the SensESP configuration editor renders first-level scalar values. The
loader also accepts older nested `gps`, `anchor_watch`, and `signalk_paths`
configuration objects. The pulse counter itself is stored in ESP32 NVS under the
`windlass` namespace because it is runtime state rather than UI configuration.

## Editable Fields

| Field | Default | Runtime effect |
| --- | ---: | --- |
| `meters_per_pulse` | `0.33` | Converts GP2 pulses to deployed metres. Applied live. |
| `chain_pulse_debounce_ms` | `75` | Debounces the GP2 dry contact. Copied to an ISR-safe volatile value. |
| `command_deadman_ms` | `1200` | Maximum age of a refreshed remote UP/DOWN command. |
| `stall_detect_ms` | `3500` | Stops a remote command if no pulses arrive while running. |
| `min_safe_length_m` | `0.50` | Prevents remote UP/retrieve below this deployed length. |
| `relay_active_high` | `true` | Selects relay module input polarity. Default `true` is for high-level-trigger modules where GPIO HIGH energizes the relay input. |
| `free_fall_detection_enabled` | `true` | Enables uncommanded payout detection. |
| `seafloor_detection_enabled` | `true` | Enables pulse-stop seafloor estimate. |
| `free_fall_min_speed_m_s` | `0.20` | Minimum speed for free-fall detection. |
| `free_fall_min_pulses` | `2` | Minimum uncommanded pulses before latching free-fall. |
| `seafloor_no_pulse_ms` | `1800` | No-pulse duration used to estimate seafloor contact. |
| `seafloor_min_length_m` | `1.00` | Minimum deployed length before seafloor detection. |
| `anchor_detected_length_m` | `0.33` | Length threshold for initial anchor deployment. |

The `gps_*` and `anchor_watch_*` fields configure the standalone anchor-watch
feature. GPS defaults to `gps_rx_pin = -1`, so no UART is opened until a tested
spare pin is selected.

`gps_mode` is selected from `auto`, `uart`, `i2c`, and `disabled`.
`anchor_watch_anchor_position_strategy` is selected from `weighted_set_fix` and
`current_fix`.

Key anchor-watch defaults:

| Field | Default | Runtime effect |
| --- | ---: | --- |
| `gps_min_satellites` | `5` | Required satellite count before a fix is usable. |
| `gps_max_hdop` | `2.5` | Required horizontal dilution quality. |
| `gps_stable_samples` | `5` | Consecutive valid fixes before arming. |
| `anchor_watch_enabled` | `true` | Enables the state machine. |
| `anchor_watch_auto_arm` | `true` | Arms from deployment state and GPS quality. |
| `anchor_watch_deploy_threshold_m` | `5.0` | Deployed rode needed before arming. |
| `anchor_watch_onboard_threshold_m` | `0.5` | Rode length used for automatic disarm. |
| `anchor_watch_automatic_radius` | `true` | Calculates radius from rode, vessel, and GPS margins. |
| `anchor_watch_manual_radius_m` | `35.0` | Radius used when automatic radius is off. |

## Signal K Path Fields

The same configuration object stores editable Signal K paths:

```text
sk_rode_length
sk_rode_speed
sk_direction
sk_state
sk_command_status
sk_command_request
sk_pulses
sk_meters_per_pulse
sk_faults
sk_mode
sk_freefall_detected
sk_anchor_detected
sk_seafloor_detected
sk_event
sk_notification
```

Defaults are defined in `include/signalk_paths.h`.

## Operational Notes

- SensESP owns the web setup UI and Signal K connection workflow.
- Permanent calibration should be done through the SensESP configuration page.
- The D4 reset input and Signal K `reset`/`zero` commands set the pulse count to
  zero.
- For the default high-level-trigger relay module, keep `relay_active_high =
  true`. Relay polarity must still be tested with the windlass contactor
  disconnected before motor tests.

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

The attached SensESP setup capture shows these fields as one configuration form
under the title `HALMET Windlass Controller`. Boolean fields render as switches,
select fields render as drop-down controls, and all other fields render as text
or numeric inputs.

## Windlass Fields

| Field | Default | Runtime effect |
| --- | ---: | --- |
| `relay_active_high` | `true` | Selects relay module input polarity. Keep `true` for high-level-trigger modules where GPIO HIGH energizes the relay input. |
| `meters_per_pulse` | `0.33` | Converts GP2 pulses to deployed metres. Applied live. |
| `chain_pulse_debounce_ms` | `75` | Debounces the GP2 dry contact. Copied to an ISR-safe volatile value. |
| `command_deadman_ms` | `1200` | Maximum age of a refreshed remote UP/DOWN command. |
| `stall_detect_ms` | `3500` | Stops a remote command if no pulses arrive while running. |
| `min_safe_length_m` | `0.50` | Prevents remote UP/retrieve below this deployed length. |
| `free_fall_detection_enabled` | `true` | Enables uncommanded payout detection. |
| `seafloor_detection_enabled` | `true` | Enables pulse-stop seafloor estimate. |
| `free_fall_min_speed_m_s` | `0.20` | Minimum speed for free-fall detection. |
| `free_fall_min_pulses` | `2` | Minimum uncommanded pulses before latching free-fall. |
| `seafloor_no_pulse_ms` | `1800` | No-pulse duration used to estimate seafloor contact. |
| `seafloor_min_length_m` | `1.00` | Minimum deployed length before seafloor detection. |
| `anchor_detected_length_m` | `0.33` | Length threshold for initial anchor deployment. |

## GPS Fields

GPS defaults to `gps_rx_pin = -1`, so no UART is opened until a tested spare pin
is selected.

| Field | Default | Runtime effect |
| --- | ---: | --- |
| `gps_mode` | `auto` | Drop-down: `auto`, `uart`, `i2c`, or `disabled`. |
| `gps_rx_pin` | `-1` | GPS UART RX pin. `-1` disables local UART GPS. |
| `gps_tx_pin` | `-1` | Optional GPS UART TX pin. `-1` is receive-only. |
| `gps_baud` | `9600` | GPS baud. `0` scans common baud rates. |
| `gps_publish_navigation_position` | `false` | Publishes local GPS to `navigation.*` paths when enabled. |
| `gps_min_satellites` | `5` | Required satellite count before a fix is usable. |
| `gps_max_hdop` | `2.5` | Required horizontal dilution quality. |
| `gps_max_fix_age_ms` | `10000` | Maximum age of the last fix before it is stale. |
| `gps_stable_samples` | `5` | Consecutive valid fixes required before anchor-watch arming. |

## Anchor Watch Fields

Anchor watch may publish alarms and state, but it never commands windlass
relays.

| Field | Default | Runtime effect |
| --- | ---: | --- |
| `anchor_watch_enabled` | `true` | Enables the anchor-watch state machine. |
| `anchor_watch_auto_arm` | `true` | Arms from deployment state and GPS quality. |
| `anchor_watch_deploy_threshold_m` | `5.0` | Deployed rode needed before arming. |
| `anchor_watch_onboard_threshold_m` | `0.5` | Rode length used for automatic disarm. |
| `anchor_watch_manual_radius_m` | `35.0` | Radius used when automatic radius is off. |
| `anchor_watch_automatic_radius` | `true` | Calculates radius from rode, vessel, and GPS margins. |
| `anchor_watch_scope_multiplier` | `1.15` | Multiplier applied to deployed rode in automatic radius mode. |
| `anchor_watch_boat_length_m` | `10.0` | Vessel length contribution to automatic radius. |
| `anchor_watch_bow_offset_m` | `0.0` | GPS-to-bow offset contribution to automatic radius. |
| `anchor_watch_gps_error_margin_m` | `10.0` | GPS uncertainty margin added to automatic radius. |
| `anchor_watch_min_radius_m` | `25.0` | Lower bound for automatic radius. |
| `anchor_watch_arming_delay_ms` | `10000` | Delay before auto-arm after all arming conditions are true. |
| `anchor_watch_alarm_delay_ms` | `15000` | Delay before raising a radius alarm. |
| `anchor_watch_clear_delay_ms` | `30000` | Delay before clearing a radius alarm. |
| `anchor_watch_hysteresis_m` | `5.0` | Margin below radius required before alarm clear timing starts. |
| `anchor_watch_waypoint_enabled` | `true` | Records the current anchor-watch waypoint state locally. |
| `anchor_watch_waypoint_delete_on_disarm` | `false` | Deletes the saved waypoint state on disarm when enabled. |
| `anchor_watch_n2k_publish_gnss` | `false` | Publishes local GNSS rapid-update PGNs when enabled. |
| `anchor_watch_n2k_anchor_watch_as_active_waypoint` | `false` | Reserved active-waypoint publishing switch. |
| `anchor_watch_anchor_position_strategy` | `weighted_set_fix` | Drop-down: `weighted_set_fix` or `current_fix`. |
| `anchor_watch_waypoint_id` | `halmet-anchor-watch-current` | Identifier used for the saved anchor-watch waypoint state. |

## Signal K Path Fields

The same configuration object stores editable Signal K paths:

| Field | Default path |
| --- | --- |
| `sk_rode_length` | `anchoring.windlass.rode.length` |
| `sk_rode_speed` | `anchoring.windlass.rode.speed` |
| `sk_direction` | `anchoring.windlass.direction` |
| `sk_state` | `anchoring.windlass.state` |
| `sk_command_status` | `anchoring.windlass.command.status` |
| `sk_command_request` | `anchoring.windlass.command.request` |
| `sk_pulses` | `anchoring.windlass.counter.pulses` |
| `sk_meters_per_pulse` | `anchoring.windlass.counter.metersPerPulse` |
| `sk_faults` | `anchoring.windlass.faults` |
| `sk_mode` | `anchoring.windlass.mode` |
| `sk_freefall_detected` | `anchoring.windlass.freeFall.detected` |
| `sk_anchor_detected` | `anchoring.windlass.anchor.detected` |
| `sk_seafloor_detected` | `anchoring.windlass.anchor.seafloorDetected` |
| `sk_event` | `anchoring.windlass.event` |
| `sk_notification` | `notifications.anchoring.windlass` |

Defaults are defined in `include/signalk_paths.h`.

## Operational Notes

- SensESP owns the web setup UI and Signal K connection workflow.
- Press `Save` in the SensESP configuration page after changing settings.
- Permanent calibration should be done through the SensESP configuration page.
- The D4 reset input and Signal K `reset`/`zero` commands set the pulse count to
  zero.
- For the default high-level-trigger relay module, keep `relay_active_high =
  true`. Relay polarity must still be tested with the windlass contactor
  disconnected before motor tests.

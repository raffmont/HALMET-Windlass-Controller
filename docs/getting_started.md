# Getting Started

This guide covers installing, configuring, and using the HALMET Windlass
Controller firmware for the first time.

## 1. Prepare The Hardware

Before connecting the windlass control circuit, bench-test the controller with
relay outputs disconnected from the contactors.

Default firmware wiring:

| Function | HALMET / ESP32 pin |
| --- | --- |
| Chain pulse sensor | D1 / `GPIO23` |
| Manual UP sense | D2 / `GPIO25` |
| Manual DOWN sense | D3 / `GPIO27` |
| Counter reset | D4 / `GPIO26` |
| UP relay output | `GPIO17` |
| DOWN relay output | `GPIO16` |
| I2C SDA/SCL | `GPIO21` / `GPIO22` |
| NMEA 2000 CAN TX/RX | `GPIO19` / `GPIO18` |

The default relay setting is `relay_active_high = true`, intended for
high-level-trigger relay modules where GPIO HIGH energizes the relay input.
Verify relay polarity with the windlass contactor disconnected.

## 2. Install Build Tools

Install PlatformIO. From the repository root, build the default `halmet`
environment:

```bash
platformio run
```

Upload the firmware:

```bash
platformio run -t upload
```

Open the serial monitor when you need boot logs or the device IP address:

```bash
platformio device monitor
```

## 3. Complete SensESP Onboarding

On first boot, use the SensESP setup workflow to join the device to WiFi and
configure the Signal K server connection.

1. Power the HALMET controller.
2. Connect to the SensESP access point if the device is not already on WiFi.
3. Enter the vessel WiFi credentials.
4. Configure the Signal K server address and connection settings.
5. Reboot when the setup workflow asks for it.
6. Open the device web UI from the assigned IP address.

The top navigation should include `Status`, `System`, `WiFi`, `Signal K`, and
`Configuration`.

## 4. Configure Windlass Settings

Open the SensESP `Configuration` page and edit `HALMET Windlass Controller`.
Press `Save` after changes.

Start with these fields:

| Field | Suggested first value |
| --- | --- |
| `meters_per_pulse` | `0.33` for a GP2-style 33 cm pulse distance, then calibrate. |
| `relay_active_high` | Keep `true` for high-level-trigger relay modules. |
| `chain_pulse_debounce_ms` | Keep `75` unless pulse counts bounce or miss. |
| `command_deadman_ms` | Keep `1200` for guarded remote control. |
| `stall_detect_ms` | Keep `3500` until dockside testing confirms pulse timing. |
| `min_safe_length_m` | Keep `0.50` to prevent remote retrieval into the roller. |

For anchor-watch position, choose one source:

| Field | Suggested first value |
| --- | --- |
| `gps_position_source` | `local` for a directly wired receiver, `signalk` for server `navigation.position`, or `nmea2000` for incoming NMEA 2000 PGN `129025`. |
| `gps_mode` | `auto` |
| `gps_rx_pin` | Tested spare UART RX pin for `local`, or `-1` when using `signalk` or `nmea2000`. |
| `gps_tx_pin` | `-1` unless the receiver needs transmit wiring. |
| `gps_baud` | `9600`, or `0` to scan common baud rates. |
| `gps_sk_position_path` | `navigation.position` when using `signalk`. |
| `anchor_watch_enabled` | `true` when GPS wiring and fix quality are verified. |
| `anchor_watch_auto_arm` | `true` for automatic arming after deployment. |

See [SensESP configuration](sensesp_configuration.md) for the full field list.

## 5. Zero And Calibrate The Counter

1. Recover the anchor fully without over-tensioning the rode.
2. Trigger D4 or send the Signal K command `reset` or `zero`.
3. Deploy a known length of rode.
4. Compare the measured length with `anchoring.windlass.rode.length`.
5. Adjust `meters_per_pulse` in the configuration page.
6. Save and repeat until the displayed length matches the measured length.

## 6. Use Signal K Commands

The default remote command path is:

```text
anchoring.windlass.command.request
```

Accepted string commands:

```text
up
down
stop
reset
zero
faults clear
```

`up` and `down` are dead-man commands. A remote UI must refresh the active
command before `command_deadman_ms` expires and send `stop` on release, page
blur, network loss, or application shutdown.

## 7. Verify Safe Operation

With the motor disabled or contactors disconnected:

1. Confirm both relays are off at boot.
2. Confirm `stop` always de-energizes both relay outputs.
3. Confirm UP and DOWN are mutually exclusive.
4. Confirm D1 pulses increment while deploying and decrement while retrieving.
5. Confirm D4 resets the counter.
6. Confirm stale `up` and `down` commands stop after the dead-man timeout.
7. Confirm no-pulse stall detection stops remote movement.
8. Confirm remote retrieval stops near `min_safe_length_m`.

Only after bench and dockside tests pass should the relay outputs be connected
to the real windlass control circuit.

## 8. Operate Anchor Watch

Anchor watch uses the selected position source and rode length to publish state
and alarms. It does not command windlass relays.

1. Configure and verify `gps_position_source`.
2. Wait for `anchoring.anchorWatch.gnss.fixValid = true`.
3. Deploy more than `anchor_watch_deploy_threshold_m`.
4. Keep the vessel stable until `anchor_watch_arming_delay_ms` expires.
5. Watch `anchoring.anchorWatch.state`, `radius`, `distance`, and `margin`.
6. Retrieve below `anchor_watch_onboard_threshold_m` to auto-disarm.

Use anchor-watch alarms as advisory information only. Keep the vessel's normal
anchor watch, seamanship, and safety procedures in force.

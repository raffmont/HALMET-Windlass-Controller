# GPS/GNSS

The anchor-watch subsystem can use one of three position sources:

- `local`: an optional local UART NMEA 0183 GNSS receiver.
- `signalk`: the vessel position already available from the Signal K server.
- `nmea2000`: incoming NMEA 2000 position PGNs.

The local UART option is intended for standalone overnight use when Signal K or
WiFi is unavailable.

## Configuration

GPS settings live under `/Windlass/Configuration` with `gps_*` field names.

| Field | Default | Description |
| --- | ---: | --- |
| `gps_position_source` | `local` | `local`, `signalk`, or `nmea2000`. |
| `gps_mode` | `auto` | `auto`, `uart`, `i2c`, or `disabled`. Current local receiver parser implements UART NMEA 0183. |
| `gps_rx_pin` | `-1` | UART RX pin. `-1` leaves local GPS disabled. |
| `gps_tx_pin` | `-1` | Optional UART TX pin. `-1` is receive-only. |
| `gps_baud` | `9600` | Initial baud rate. Set `0` or use `auto` to scan common NMEA rates. |
| `gps_sk_position_path` | `navigation.position` | Signal K path used when `gps_position_source = signalk`. |
| `gps_min_satellites` | `5` | Minimum satellites before the fix can arm anchor watch. For Signal K and NMEA 2000 sources, the firmware maps an accepted position to this minimum. |
| `gps_max_hdop` | `2.5` | Maximum HDOP before the fix is considered poor. For Signal K and NMEA 2000 sources, the firmware maps an accepted position to this threshold. |
| `gps_max_fix_age_ms` | `10000` | Fixes older than this are stale. |
| `gps_stable_samples` | `5` | Consecutive good samples required before arming. |
| `gps_publish_navigation_position` | `false` | Reserved for publishing local GPS as vessel `navigation.*`. |

## Hardware Notes

No GPS RX pin is enabled by default because the note's example `GPIO34` is not
safe in this repository: the active HALMET mapping already uses D1 / `GPIO23`
for the chain sensor, and older defaults mention `GPIO34` as a possible sensor
pin. Select a spare exposed ESP32 GPIO for the exact HALMET revision and record
the tested wiring before enabling GPS.

The parser accepts standard `$GPRMC`, `$GNRMC`, `$GPGGA`, and `$GNGGA`
sentences, with GSA sentences recognized for presence detection. u-blox UBX and
I2C GNSS detection are not implemented yet.

## Signal K Position Source

Set `gps_position_source = signalk` to use position already available on the
Signal K server. The firmware subscribes to `gps_sk_position_path`, which
defaults to:

```text
navigation.position
```

The subscribed value must contain `latitude` and `longitude` in decimal degrees.

## NMEA 2000 Position Source

Set `gps_position_source = nmea2000` to use incoming NMEA 2000 position data.
The firmware consumes:

```text
129025 Position, Rapid Update
129026 COG and SOG, Rapid Update
```

PGN `129025` supplies latitude and longitude. PGN `129026` is optional and only
updates COG/SOG for publishing; anchor watch only requires position.

## Signal K Outputs

The selected position source and quality gate are published under:

```text
anchoring.anchorWatch.gnss.present
anchoring.anchorWatch.gnss.interface
anchoring.anchorWatch.gnss.fixValid
anchoring.anchorWatch.gnss.hdop
anchoring.anchorWatch.gnss.satellites
anchoring.anchorWatch.gnss.position.latitude
anchoring.anchorWatch.gnss.position.longitude
```

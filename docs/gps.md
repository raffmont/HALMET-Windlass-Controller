# GPS/GNSS

The anchor-watch subsystem can use a local UART NMEA 0183 GNSS receiver. This is
intended for standalone overnight use when Signal K or WiFi is unavailable.

## Configuration

GPS settings live under `/Windlass/Configuration` with `gps_*` field names.

| Field | Default | Description |
| --- | ---: | --- |
| `mode` | `auto` | `auto`, `uart`, `i2c`, or `disabled`. Current firmware implements UART parsing. |
| `rx_pin` | `-1` | UART RX pin. `-1` leaves GPS disabled. |
| `tx_pin` | `-1` | Optional UART TX pin. `-1` is receive-only. |
| `baud` | `9600` | Initial baud rate. Set `0` or use `auto` to scan common NMEA rates. |
| `min_satellites` | `5` | Minimum satellites before the fix can arm anchor watch. |
| `max_hdop` | `2.5` | Maximum HDOP before the fix is considered poor. |
| `max_fix_age_ms` | `10000` | Fixes older than this are stale. |
| `stable_samples` | `5` | Consecutive good samples required before arming. |
| `publish_navigation_position` | `false` | Reserved for publishing local GPS as vessel `navigation.*`. |

## Hardware Notes

No GPS RX pin is enabled by default because the note's example `GPIO34` is not
safe in this repository: the active HALMET mapping already uses D1 / `GPIO23`
for the chain sensor, and older defaults mention `GPIO34` as a possible sensor
pin. Select a spare exposed ESP32 GPIO for the exact HALMET revision and record
the tested wiring before enabling GPS.

The parser accepts standard `$GPRMC`, `$GNRMC`, `$GPGGA`, and `$GNGGA`
sentences, with GSA sentences recognized for presence detection. u-blox UBX and
I2C GNSS detection are not implemented yet.

## Signal K Outputs

The current local fix and quality gate are published under:

```text
anchoring.anchorWatch.gnss.present
anchoring.anchorWatch.gnss.interface
anchoring.anchorWatch.gnss.fixValid
anchoring.anchorWatch.gnss.hdop
anchoring.anchorWatch.gnss.satellites
anchoring.anchorWatch.gnss.position.latitude
anchoring.anchorWatch.gnss.position.longitude
```

# Hardware Notes

This repository targets HALMET / ESP32 hardware with a Quick Genius GP2 style
windlass installation.

## Default Inputs

| Function | HALMET input | ESP32 pin |
| --- | --- | --- |
| GP2 chain pulse sensor | D1 / In1 | `GPIO23` |
| Manual UP sense | D2 / In2 | `GPIO25` |
| Manual DOWN sense | D3 / In3 | `GPIO27` |
| Counter reset | D4 / In4 | `GPIO26` |

The GP2 sensor is assumed to be a normally-open dry contact that closes once per
gypsy turn. The default calibration is `0.33 m/pulse`.

## Default Relay Outputs

| Function | ESP32 pin |
| --- | --- |
| UP relay module input | `GPIO17` |
| DOWN relay module input | `GPIO16` |

The default firmware setting matches optocoupled high-level-trigger relay
modules: `relay_active_high = true`, GPIO HIGH energizes the selected relay
input, and GPIO LOW de-energizes it. Keep the relay board VCC/JD-VCC, input
ground, and contact ratings matched to the installation.

The firmware exposes `relay_active_high` in `/Windlass/Configuration` for other
relay boards. Test this with the windlass control circuit disconnected. A wrong
polarity setting can energize a relay unexpectedly.

## I2C And OLED

The OLED display follows the HALMET helper code:

```text
SDA = GPIO21
SCL = GPIO22
ADS1115 address = 0x4b
```

## CAN / NMEA 2000

```text
CAN TX = GPIO19
CAN RX = GPIO18
```

Keep NMEA 2000 wiring compliant with the vessel backbone requirements, including
drop length, power injection, shielding, and termination.

## Optional GPS/GNSS

Anchor watch can use an external UART NMEA 0183 GNSS receiver. GPS RX is
disabled by default (`gps.rx_pin = -1`) because safe spare GPIO availability
depends on the exact HALMET revision and installation. Do not reuse D1-D4, relay
outputs, I2C, or CAN pins.

Configure the selected RX pin, optional TX pin, and baud rate in
`/Windlass/Configuration` after verifying the pin is exposed and electrically
compatible with the receiver.

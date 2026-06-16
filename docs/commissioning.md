# Commissioning Checklist

## Before Installation

- [ ] Confirm the exact Quick GP2 wiring diagram for the installed model.
- [ ] Confirm chain sensor type, polarity, and pulse distance.
- [ ] Confirm HALMET board revision and input/output mapping.
- [ ] Confirm relay module voltage, current rating, isolation, and high-level
      trigger polarity.
- [ ] If using anchor watch, select and document a spare GPS UART RX pin for the
      exact HALMET revision.
- [ ] Confirm the NMEA 2000 backbone has correct power and termination.

## Firmware Setup

- [ ] Build with PlatformIO: `platformio run`.
- [ ] Configure WiFi and Signal K through the SensESP setup workflow.
- [ ] Confirm `/Windlass/Configuration` appears in the SensESP UI.
- [ ] Set `meters_per_pulse`; default GP2 value is `0.33`.
- [ ] Keep `relay_active_high = true` for high-level-trigger relay modules.
- [ ] Set the zero point with the anchor fully recovered but not over-tensioned.
- [ ] Confirm the OLED shows IP, mode, chain length, speed, anchor-watch state,
      and position source when fitted.
- [ ] If using GPS, set `gps_rx_pin`, confirm `gnss.present`, and wait for
      `gnss.fixValid` before testing anchor watch.

## Bench And Dockside Tests

- [ ] Relays remain off at boot and reset with relay inputs at GPIO LOW.
- [ ] D1 sensor pulses increment while deploying.
- [ ] D1 sensor pulses decrement while retrieving.
- [ ] D2 manual UP sense changes direction/mode.
- [ ] D3 manual DOWN sense changes direction/mode.
- [ ] D4 reset input zeros the counter.
- [ ] Signal K values update once per second.
- [ ] `up` and `down` commands stop when not refreshed.
- [ ] `stop` de-energizes both relays.
- [ ] NMEA 2000 PGNs are visible on the diagnostic tool or MFD.
- [ ] Free-fall and seafloor events are tested with the motor disabled.
- [ ] Anchor watch auto-arms only after the configured deployment threshold and
      usable GPS fix.
- [ ] Anchor watch auto-disarms when rode length returns below the on-board
      threshold.

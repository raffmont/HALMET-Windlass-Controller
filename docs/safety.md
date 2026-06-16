# Safety Notes

This project can command high-current deck machinery. Treat relay-control code,
wiring, and commissioning as safety critical.

## Non-Negotiable Protections

- Keep the original windlass breaker, fuses, contactor protection, and manual
  controls.
- Do not route motor current through HALMET.
- Use isolated relay drivers or a relay module suitable for the contactor/control
  circuit.
- The default configuration is for high-level-trigger relay inputs: GPIO HIGH
  energizes a relay and GPIO LOW leaves it de-energized.
- Wire remote outputs as normally-open dry contacts in parallel with the
  original UP/DOWN controls.
- Ensure both relays are off during boot and reset.
- Keep a physical emergency stop or breaker reachable during all tests.
- Do not rely on free-fall or seafloor detection as a certified measurement.

## Firmware Safety Behavior

- Both relays are de-energized before changing direction.
- Remote UP is blocked below `min_safe_length_m`.
- Remote UP/DOWN commands must be refreshed before `command_deadman_ms`.
- No pulses while remotely commanded stops the relays after `stall_detect_ms`.
- Manual UP or DOWN activity blocks remote relay commands.
- Simultaneous manual UP and DOWN sense stops relays and raises a fault.
- STOP is accepted immediately.
- Anchor watch never energizes relays. It only observes chain/GNSS state and
  publishes local/Signal K alarm status.
- Do not treat anchor-watch GPS alarms as a substitute for watchkeeping, anchor
  gear inspection, or a certified navigation alarm.

## Test Sequence

1. Build firmware and review pin assignments.
2. Bench-test with LEDs instead of relay coils.
3. Test relay polarity with windlass control wiring disconnected.
4. Test pulse counting by manually actuating the GP2 sensor input.
5. Test manual UP/DOWN sense lines without energizing the motor.
6. Test Signal K publishing and command behavior.
7. Test NMEA 2000 visibility with a diagnostic tool or MFD.
8. Test on the vessel with the windlass motor breaker off.
9. Perform first motor tests dockside with two operators and breaker access.

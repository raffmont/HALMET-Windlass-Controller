# Architecture

The HALMET node is the source of truth for chain count, windlass direction,
local safety interlocks, relay actuation, Signal K publishing, and NMEA 2000
telemetry.

The implementation intentionally follows the HALMET example firmware style:
SensESP owns onboarding, OTA, configuration, Signal K outputs/listeners, and the
cooperative event loop. The windlass behavior is implemented as ordinary C++
state handling scheduled from `event_loop()->onRepeat()`.

```text
GP2 sensor -> HALMET D1 interrupt -> pulse counter -> chain model
                                                   -> Signal K outputs
                                                   -> NMEA 2000 PGNs

Signal K command listener -> guarded command logic -> UP/DOWN relay outputs
Manual UP/DOWN sense -----> direction and safety model
```

## Data Flow

1. The D1 pulse interrupt updates the persistent pulse counter directionally.
2. Manual UP/DOWN sense inputs and active remote commands determine direction.
3. The chain model derives deployed rode length and speed.
4. Event detection latches anchor deployment, free-fall, and seafloor estimates.
5. SensESP `SKOutput*` objects publish Signal K state.
6. The NMEA2000 library sends windlass PGNs from scheduled callbacks.
7. The OLED display shows IP, mode, deployed chain, and speed when present.

## Preserved HALMET/SensESP Pattern

- `SensESPAppBuilder` creates the app.
- `ConfigItem` exposes `/Windlass/Configuration`.
- `StringSKListener` receives remote commands.
- `SKOutputFloat`, `SKOutputString`, `SKOutputInt`, and `SKOutputBool` publish
  values.
- `event_loop()->onRepeat()` schedules controller updates, Signal K publishing,
  NMEA 2000 parsing, NMEA 2000 publishing, and display refresh.
- `while (true) { loop(); }` is retained at the end of setup, as in the HALMET
  example firmware, to keep SensESP-owned objects alive.

## Failure Behavior

| Condition | Expected behavior |
| --- | --- |
| UP and DOWN both sensed | Stop relays and raise `FaultMutuallyExclusiveCommand`. |
| Remote command not refreshed | Stop relays and raise `FaultCommandTimeout`. |
| No chain pulses while commanded | Stop relays and raise `FaultNoPulsesWhileRunning`. |
| Retrieval near zero | Stop relays and raise `FaultNearZeroLimit`. |
| NMEA 2000 open fails | Continue Signal K/local behavior and raise `FaultNmea2000Error`. |
| Reboot/reset | Relays are configured off before normal operation continues. |

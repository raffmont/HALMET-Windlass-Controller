# Anchor Watch

Anchor watch adds a standalone local alarm model on top of the existing chain
counter and windlass state machine. It does not command the windlass and does
not bypass any breaker, fuse, contactor, manual control, or emergency stop.

## State Machine

The firmware uses these states:

```text
disabled
waitingForGps
ready
armed
alarm
suspended
fault
```

At boot, a previously armed watch is restored as `suspended` until the selected
position source has a usable fix. This prevents a reboot from silently
forgetting the saved anchor centre while still avoiding an immediate
stale-position alarm.

## Automatic Arming And Disarming

With `anchor_watch_enabled` and `anchor_watch_auto_arm` true, the watch arms
after all of these conditions remain true for `arming_delay_ms`:

- The selected position source has a usable fix.
- Deployed rode is at least `deploy_threshold_m`.
- The windlass state indicates deployment, free-fall, seafloor detection, or
  anchor detection.

The watch disarms automatically when the deployed rode is at or below
`onboard_threshold_m` and the windlass is retrieving or stopped. Disarming clears
active local alarm notifications but retains the last saved anchor centre unless
future code implements remote resource deletion.

## Anchor Centre And Radius

During deployment, the firmware captures the latest stable position fix after
the deployment threshold. On arm it uses that deployment candidate as the
estimated anchor-watch centre; if no candidate exists, it falls back to the
current position fix. The stored strategy string defaults to `weighted_set_fix`,
but the current implementation is a conservative latest-good-deployment-fix
approximation.

Automatic radius is:

```text
max(min_radius_m,
    rode_length_m * scope_multiplier
    + boat_length_m
    + gps_error_margin_m
    + bow_offset_m)
```

Defaults are `scope_multiplier = 1.15`, `boat_length_m = 10.0`,
`gps_error_margin_m = 10.0`, `bow_offset_m = 0.0`, and `min_radius_m = 25.0`.
Set `automatic_radius` false to use `manual_radius_m`.

## Alarm Behavior

The alarm distance is the haversine distance between the current selected
position source and the stored anchor centre.

- Alarm becomes active when distance exceeds radius for `alarm_delay_ms`.
- Alarm clears when distance is at least `hysteresis_m` inside the radius for
  `clear_delay_ms`.
- Position-source loss while armed emits a warning notification.

## Persistence

The anchor centre, radius, rode length at arm, and last state are stored in ESP32
NVS in the existing `windlass` namespace. This is runtime state rather than
SensESP UI configuration.

The `waypoint_enabled` setting currently records the waypoint locally and
publishes anchor-watch position fields. Signal K Resources API PUT/DELETE calls
are not implemented in this firmware because SensESP owns the server connection
and this repository does not yet contain a REST client for resources.

## Signal K Outputs

```text
anchoring.anchorWatch.enabled
anchoring.anchorWatch.autoArm
anchoring.anchorWatch.state
anchoring.anchorWatch.radius
anchoring.anchorWatch.distance
anchoring.anchorWatch.margin
anchoring.anchorWatch.position.latitude
anchoring.anchorWatch.position.longitude
anchoring.anchorWatch.rodeLengthAtArm
notifications.anchoring.anchorWatch
```

The notification output is a compact JSON string containing the specific
notification path, state, method, and message. Radius alarms use
`notifications.anchoring.anchorWatch.radiusExceeded`; GPS loss uses
`notifications.anchoring.anchorWatch.gpsLost`.

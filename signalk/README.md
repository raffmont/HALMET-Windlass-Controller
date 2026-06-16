# HALMET Windlass Signal K Plugin

This folder contains a Signal K Node Server plugin and web app for HALMET
windlass control and diagnostics.

The app is intentionally guarded:

- UP and DOWN are press-and-hold controls.
- The browser refreshes a held command while the button is pressed.
- The plugin republishes held commands at a short interval for the firmware
  dead-man timer.
- STOP is sent on release, cancel, page blur, and unload.
- If the browser stops refreshing a hold, the plugin sends STOP.
- The plugin only sends the firmware-supported command strings.

Anchor watch is diagnostic only. The web app displays anchor-watch state,
radius, distance, margin, GNSS quality, and selected position source, but it
does not command anchor-watch movement or bypass any windlass safety hardware.

## Files

```text
signalk/
  index.js              Signal K plugin entry point and command API
  package.json          Plugin manifest and web app metadata
  public/index.html     Web app shell
  public/app.js         Signal K stream client and command handling
  public/styles.css     Responsive app styling
```

## Install

From the Signal K server machine, copy or symlink this `signalk` directory into
the Signal K server plugin directory. Common locations are:

```text
~/.signalk/node_modules/signalk-halmet-windlass
/home/pi/.signalk/node_modules/signalk-halmet-windlass
```

For development from this repository, a symlink is usually easiest:

```bash
cd ~/.signalk/node_modules
ln -s /path/to/HALMET-Windlass-Controller/signalk signalk-halmet-windlass
```

Restart Signal K Node Server after installing the plugin.

## Enable

1. Open the Signal K admin UI.
2. Go to **Server > Plugin Config**.
3. Enable **HALMET Windlass**.
4. Confirm the command request path is:

```text
anchoring.windlass.command.request
```

5. Save and restart the server if prompted.

The web app is available from the Signal K web apps list as **HALMET
Windlass**. It can also be opened directly at:

```text
/plugins/halmet-windlass/public/
```

## Plugin Options

| Option | Default | Description |
| --- | --- | --- |
| `commandRequestPath` | `anchoring.windlass.command.request` | Signal K path consumed by the firmware `StringSKListener`. |
| `refreshMs` | `400` | Interval used by the plugin to repeat held `up` or `down` commands. |
| `serverDeadmanMs` | `1500` | Maximum age of the browser hold heartbeat before the plugin sends `stop`. |

Keep `refreshMs` shorter than the firmware `command_deadman_ms` setting. The
firmware still remains the final safety authority and will stop when its
dead-man command refresh expires.

## Web App Controls

| Control | Signal K command value |
| --- | --- |
| UP hold | `up` |
| DOWN hold | `down` |
| STOP | `stop` |
| Zero Counter | `zero` |
| Reset Counter | `reset` |
| Clear Faults | `faults clear` |

The app displays live values from the firmware's default Signal K paths,
including windlass mode, command status, rode length, rode speed, faults,
counter pulses, event latches, anchor-watch state, and GNSS diagnostics.

## Signal K Paths

The plugin defaults match `include/signalk_paths.h` in this repository. If the
firmware paths are changed in the SensESP web UI, update this plugin's command
request path in Signal K plugin config. The web app diagnostics expect the
default published paths.

## Safety Notes

Use this app only with the original breaker, fuses, contactor protection,
manual controls, and emergency stop in service. Do not use the web app as a
substitute for the vessel's normal windlass controls or watchkeeping.

Before operating a real windlass, test on the bench or with the windlass
breaker open and confirm:

- STOP is immediate.
- UP and DOWN cannot remain active after release.
- Page close, network loss, and Signal K restart cause the firmware to stop
  through the dead-man timeout.
- Fault and event diagnostics match the firmware status paths.

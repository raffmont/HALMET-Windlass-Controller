'use strict';

const path = require('path');
const express = require('express');

const DEFAULT_PATHS = {
  commandRequest: 'anchoring.windlass.command.request',
  commandStatus: 'anchoring.windlass.command.status',
  rodeLength: 'anchoring.windlass.rode.length',
  rodeSpeed: 'anchoring.windlass.rode.speed',
  direction: 'anchoring.windlass.direction',
  state: 'anchoring.windlass.state',
  mode: 'anchoring.windlass.mode',
  faults: 'anchoring.windlass.faults',
  pulses: 'anchoring.windlass.counter.pulses',
  metersPerPulse: 'anchoring.windlass.counter.metersPerPulse',
  freeFallDetected: 'anchoring.windlass.freeFall.detected',
  anchorDetected: 'anchoring.windlass.anchor.detected',
  seafloorDetected: 'anchoring.windlass.anchor.seafloorDetected',
  event: 'anchoring.windlass.event',
  notification: 'notifications.anchoring.windlass',
  anchorWatchEnabled: 'anchoring.anchorWatch.enabled',
  anchorWatchAutoArm: 'anchoring.anchorWatch.autoArm',
  anchorWatchState: 'anchoring.anchorWatch.state',
  anchorWatchRadius: 'anchoring.anchorWatch.radius',
  anchorWatchDistance: 'anchoring.anchorWatch.distance',
  anchorWatchMargin: 'anchoring.anchorWatch.margin',
  anchorWatchRodeAtArm: 'anchoring.anchorWatch.rodeLengthAtArm',
  gnssPresent: 'anchoring.anchorWatch.gnss.present',
  gnssInterface: 'anchoring.anchorWatch.gnss.interface',
  gnssFixValid: 'anchoring.anchorWatch.gnss.fixValid',
  gnssHdop: 'anchoring.anchorWatch.gnss.hdop',
  gnssSatellites: 'anchoring.anchorWatch.gnss.satellites',
  gnssLatitude: 'anchoring.anchorWatch.gnss.position.latitude',
  gnssLongitude: 'anchoring.anchorWatch.gnss.position.longitude',
  anchorWatchNotification: 'notifications.anchoring.anchorWatch'
};

const MOVEMENT_COMMANDS = new Set(['up', 'down']);
const SINGLE_COMMANDS = new Set(['stop', 'reset', 'zero', 'faults clear']);

module.exports = function halmetWindlassPlugin(app) {
  let options = {};
  let holdTimer = null;
  let heldCommand = null;
  let lastHoldRefresh = 0;

  const plugin = {
    id: 'halmet-windlass',
    name: 'HALMET Windlass',
    description: 'Guarded windlass control and diagnostics web app for HALMET firmware.'
  };

  plugin.schema = {
    type: 'object',
    required: ['commandRequestPath'],
    properties: {
      commandRequestPath: {
        type: 'string',
        title: 'Command request path',
        default: DEFAULT_PATHS.commandRequest
      },
      refreshMs: {
        type: 'number',
        title: 'Held command refresh interval (ms)',
        default: 400,
        minimum: 100,
        maximum: 1000
      },
      serverDeadmanMs: {
        type: 'number',
        title: 'Server-side hold dead-man timeout (ms)',
        default: 1500,
        minimum: 500,
        maximum: 5000
      }
    }
  };

  plugin.start = function start(pluginOptions) {
    options = {
      commandRequestPath: DEFAULT_PATHS.commandRequest,
      refreshMs: 400,
      serverDeadmanMs: 1500,
      ...pluginOptions
    };
    app.debug('HALMET Windlass plugin started');
  };

  plugin.stop = function stop() {
    stopHold();
    app.debug('HALMET Windlass plugin stopped');
  };

  plugin.registerWithRouter = function registerWithRouter(router) {
    router.use('/public', express.static(path.join(__dirname, 'public')));
    router.get('/', (_req, res) => res.redirect('public/'));
    router.get('/api/config', (_req, res) => {
      res.json({
        pluginId: plugin.id,
        paths: {
          ...DEFAULT_PATHS,
          commandRequest: options.commandRequestPath || DEFAULT_PATHS.commandRequest
        },
        refreshMs: options.refreshMs,
        serverDeadmanMs: options.serverDeadmanMs
      });
    });
    router.post('/api/command', express.json(), (req, res) => {
      const command = normalizeCommand(req.body && req.body.command);
      if (!SINGLE_COMMANDS.has(command)) {
        res.status(400).json({ error: 'Unsupported single command.' });
        return;
      }
      if (command === 'stop') stopHold(false);
      sendCommand(command);
      res.json({ command });
    });
    router.post('/api/hold/start', express.json(), (req, res) => {
      const command = normalizeCommand(req.body && req.body.command);
      if (!MOVEMENT_COMMANDS.has(command)) {
        res.status(400).json({ error: 'Hold command must be up or down.' });
        return;
      }
      startHold(command);
      res.json({ command });
    });
    router.post('/api/hold/refresh', express.json(), (req, res) => {
      const command = normalizeCommand(req.body && req.body.command);
      if (!heldCommand || command !== heldCommand) {
        res.status(409).json({ error: 'No matching held command.' });
        return;
      }
      lastHoldRefresh = Date.now();
      res.json({ command });
    });
    router.post('/api/hold/stop', express.json(), (_req, res) => {
      stopHold();
      res.json({ command: 'stop' });
    });
  };

  function normalizeCommand(command) {
    return String(command || '').trim().toLowerCase();
  }

  function sendCommand(command) {
    app.handleMessage(plugin.id, {
      updates: [
        {
          values: [
            {
              path: options.commandRequestPath || DEFAULT_PATHS.commandRequest,
              value: command
            }
          ]
        }
      ]
    });
  }

  function startHold(command) {
    stopHold(false);
    heldCommand = command;
    lastHoldRefresh = Date.now();
    sendCommand(command);
    holdTimer = setInterval(() => {
      if (!heldCommand) return;
      if (Date.now() - lastHoldRefresh > options.serverDeadmanMs) {
        stopHold();
        return;
      }
      sendCommand(heldCommand);
    }, options.refreshMs);
  }

  function stopHold(sendStop = true) {
    if (holdTimer) clearInterval(holdTimer);
    holdTimer = null;
    heldCommand = null;
    lastHoldRefresh = 0;
    if (sendStop) sendCommand('stop');
  }

  return plugin;
};

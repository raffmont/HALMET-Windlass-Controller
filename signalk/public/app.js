(async function () {
  const state = {};
  const fields = new Map();
  let config = null;
  let socket = null;
  let activeHold = null;
  let holdRefreshTimer = null;

  document.querySelectorAll('[data-field]').forEach((node) => {
    fields.set(node.dataset.field, node);
  });

  const connection = document.getElementById('connection');
  const notifications = document.getElementById('notifications');

  try {
    config = await fetchJson('../api/config');
    renderAll();
    connectStream();
  } catch (error) {
    connection.textContent = `Plugin API unavailable: ${error.message}`;
  }

  document.getElementById('stopButton').addEventListener('click', stopHold);
  document.querySelectorAll('.hold').forEach((button) => {
    const command = button.dataset.command;
    button.addEventListener('pointerdown', (event) => {
      event.preventDefault();
      startHold(command, button);
    });
    button.addEventListener('pointerup', stopHold);
    button.addEventListener('pointercancel', stopHold);
    button.addEventListener('pointerleave', stopHold);
  });
  document.querySelectorAll('[data-single-command]').forEach((button) => {
    button.addEventListener('click', () => {
      sendSingle(button.dataset.singleCommand);
    });
  });
  window.addEventListener('blur', stopHold);
  window.addEventListener('beforeunload', stopHoldBeacon);

  async function fetchJson(url, options) {
    const response = await fetch(url, options);
    if (!response.ok) throw new Error(`${response.status} ${response.statusText}`);
    return response.json();
  }

  async function postJson(url, body) {
    return fetchJson(url, {
      method: 'POST',
      headers: { 'content-type': 'application/json' },
      body: JSON.stringify(body || {})
    });
  }

  async function sendSingle(command) {
    try {
      await postJson('../api/command', { command });
    } catch (error) {
      connection.textContent = `Command failed: ${error.message}`;
    }
  }

  async function startHold(command, button) {
    if (activeHold) await stopHold();
    activeHold = { command, button };
    button.classList.add('active');
    try {
      await postJson('../api/hold/start', { command });
      holdRefreshTimer = setInterval(() => refreshHold(command), Math.max(200, config.refreshMs));
    } catch (error) {
      button.classList.remove('active');
      activeHold = null;
      connection.textContent = `Hold failed: ${error.message}`;
    }
  }

  async function stopHold() {
    const previous = activeHold;
    activeHold = null;
    if (holdRefreshTimer) clearInterval(holdRefreshTimer);
    holdRefreshTimer = null;
    if (previous && previous.button) previous.button.classList.remove('active');
    try {
      await postJson('../api/hold/stop');
    } catch (error) {
      connection.textContent = `Stop failed: ${error.message}`;
    }
  }

  function stopHoldBeacon() {
    if (holdRefreshTimer) clearInterval(holdRefreshTimer);
    navigator.sendBeacon('../api/hold/stop', new Blob(['{}'], { type: 'application/json' }));
  }

  async function refreshHold(command) {
    if (!activeHold || activeHold.command !== command) return;
    try {
      await postJson('../api/hold/refresh', { command });
    } catch (error) {
      connection.textContent = `Hold refresh failed: ${error.message}`;
      await stopHold();
    }
  }

  function connectStream() {
    const wsProto = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
    socket = new WebSocket(`${wsProto}//${window.location.host}/signalk/v1/stream?subscribe=none`);

    socket.addEventListener('open', () => {
      connection.textContent = 'Connected';
      socket.send(JSON.stringify({
        context: 'vessels.self',
        subscribe: Object.values(config.paths).map((path) => ({
          path,
          period: 1000,
          policy: 'instant'
        }))
      }));
    });

    socket.addEventListener('message', (event) => {
      const delta = JSON.parse(event.data);
      (delta.updates || []).forEach((update) => {
        (update.values || []).forEach(({ path, value }) => {
          const key = keyForPath(path);
          if (!key) return;
          state[key] = value;
          renderField(key, value);
        });
      });
    });

    socket.addEventListener('close', () => {
      connection.textContent = 'Disconnected, retrying';
      setTimeout(connectStream, 2000);
    });

    socket.addEventListener('error', () => {
      connection.textContent = 'Stream error';
      socket.close();
    });
  }

  function keyForPath(path) {
    return Object.entries(config.paths).find(([, value]) => value === path)?.[0];
  }

  function renderAll() {
    Object.keys(config.paths).forEach((key) => renderField(key, state[key]));
  }

  function renderField(key, value) {
    if (key === 'notification' || key === 'anchorWatchNotification') {
      const windlass = formatValue(state.notification);
      const watch = formatValue(state.anchorWatchNotification);
      notifications.textContent = `Windlass: ${windlass}\nAnchor watch: ${watch}`;
      return;
    }
    const node = fields.get(key);
    if (!node) return;
    node.textContent = formatValue(value, key);
  }

  function formatValue(value, key) {
    if (value === undefined || value === null || value === '') return '--';
    if (typeof value === 'number') {
      if (key && key.toLowerCase().includes('latitude')) return value.toFixed(6);
      if (key && key.toLowerCase().includes('longitude')) return value.toFixed(6);
      return Math.abs(value) >= 100 ? value.toFixed(0) : value.toFixed(2);
    }
    if (typeof value === 'boolean') return value ? 'yes' : 'no';
    if (typeof value === 'object') return JSON.stringify(value, null, 2);
    return String(value);
  }
})();

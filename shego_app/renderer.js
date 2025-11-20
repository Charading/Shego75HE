const btn = document.getElementById('toggle');
const status = document.getElementById('status');
const output = document.getElementById('output');

// DOM helpers
const $ = (id) => document.getElementById(id);

// Feature constants
// Send 0x30 (INJECT_LED_TOGGLE) to have firmware tap LED_TOG keycode.
const CMD_LED_TOGGLE = 0x30;

// Logging
function log(message, type = 'info') {
  const logDiv = $('log');
  const time = new Date().toLocaleTimeString();
  const entry = document.createElement('div');
  entry.className = `log-entry log-${type}`;
  entry.innerHTML = `<span class="log-time">[${time}]</span>${message}`;
  logDiv.insertBefore(entry, logDiv.firstChild);
  
  // Keep only last 100 entries
  while (logDiv.children.length > 100) {
    logDiv.removeChild(logDiv.lastChild);
  }
}

// Get device IDs from inputs
function getDeviceIds() {
  const vendorHex = $('vendorId').value.trim();
  const productHex = $('productId').value.trim();
  
  if (!vendorHex || !productHex) {
    throw new Error('Vendor ID and Product ID are required');
  }
  
  const vendorId = parseInt(vendorHex, 16);
  const productId = parseInt(productHex, 16);
  
  if (isNaN(vendorId) || isNaN(productId)) {
    throw new Error('Invalid hex values for Vendor/Product ID');
  }
  
  return { vendorId, productId };
}

// Toggle LEDs via LED_TOG injection
$('toggleGpio').addEventListener('click', async () => {
  try {
    log('Sending LED_TOG (0x30) command...', 'info');
    const { vendorId, productId } = getDeviceIds();
    const result = await window.api.sendCommand({
      vendorId,
      productId,
      commandByte: CMD_LED_TOGGLE
    });
    console.log('sendCommand result:', result);
    
    if (result && result.success) {
      const response = result.response || {};
      const dev = result.device || {};
      const respSuccess = response.success ? '✓' : '✗';
      const cmdHex = (typeof response.cmd === 'number') ? `0x${response.cmd.toString(16).padStart(2, '0')}` : 'n/a';
      const cmdChar = (typeof response.cmd === 'number' && response.cmd >= 32 && response.cmd < 127) ? String.fromCharCode(response.cmd) : '?';
      const stateStr = typeof response.state === 'number' ? response.state : 'n/a';
      const rawPreview = Array.isArray(response.raw) ? response.raw.slice(0, 12).map(n => (typeof n === 'number' ? n.toString(16).padStart(2, '0') : '??')).join(' ') : 'n/a';
      const usagePageHex = (dev && typeof dev.usagePage === 'number') ? `0x${dev.usagePage.toString(16).toUpperCase()}` : 'n/a';
      const usageHex = (dev && typeof dev.usage === 'number') ? `0x${dev.usage.toString(16).toUpperCase()}` : 'n/a';
      const devInfo = dev && dev.path ? `path=${dev.path} usagePage=${usagePageHex} usage=${usageHex} iface=${dev.interface}` : 'device info unavailable';
      const attemptNote = response.attempt ? ` [attempt=${response.attempt}]` : '';
      log(`${respSuccess} LED_TOG response: success=${response.success} cmd=${cmdHex}('${cmdChar}') state=${stateStr}${attemptNote}`, response.success ? 'success' : 'error');
      if (result.sent) {
        const sent = Array.isArray(result.sent) ? result.sent : [];
        const sentHex = sent.slice(0, 12).map(n => n.toString(16).padStart(2, '0')).join(' ');
        log(`↳ Sent: ${sentHex}`, 'info');
      }
      if (result.fallback) {
        log(`↳ Fallback used: ${result.fallback}`, 'info');
      }
      if (result.stdout) {
        log(`↳ python stdout: ${result.stdout.trim()}`, 'info');
      }
      if (result.stderr) {
        log(`↳ python stderr: ${result.stderr.trim()}`, 'error');
      }
      log(`↳ Raw: ${rawPreview}`, 'info');
      log(`↳ Device: ${devInfo}`, 'info');
    } else {
      log(`✗ Failed: ${result.error}`, 'error');
    }
  } catch (error) {
    log(`✗ Error: ${error.message}`, 'error');
  }
});

// List HID devices
$('listDevices').addEventListener('click', async () => {
  try {
    log('Listing HID devices...', 'info');
    const result = await window.api.listDevices();
    
    if (result.success) {
      const { vendorId, productId } = getDeviceIds();
      const matches = result.devices.filter(d => d.vendorId === vendorId && d.productId === productId);

      if (matches.length === 0) {
        log(`No devices found for VID=0x${vendorId.toString(16).toUpperCase()} PID=0x${productId.toString(16).toUpperCase()}`, 'info');
      } else {
        log(`Found ${matches.length} interface(s) for the target device:`, 'success');
        matches.forEach(d => {
          const usagePageHex = d.usagePage !== undefined ? `0x${d.usagePage.toString(16).toUpperCase()}` : 'n/a';
          const usageHex = d.usage !== undefined ? `0x${d.usage.toString(16).toUpperCase()}` : 'n/a';
          log(`  • if=${d.interface} usagePage=${usagePageHex} usage=${usageHex} path=${d.path}`, 'info');
        });
      }
    } else {
      log(`✗ Failed to list devices: ${result.error}`, 'error');
    }
  } catch (error) {
    log(`✗ Error: ${error.message}`, 'error');
  }
});

// Console Logic
const consoleOutput = $('consoleOutput');
let consoleActive = false;

function appendConsole(text) {
  const span = document.createElement('span');
  span.textContent = text + '\n'; // Add newline for readability if chunks come in lines
  consoleOutput.appendChild(span);
  consoleOutput.scrollTop = consoleOutput.scrollHeight;
  
  // Limit lines
  while (consoleOutput.childNodes.length > 500) {
    consoleOutput.removeChild(consoleOutput.firstChild);
  }
}

$('startConsole').addEventListener('click', () => {
  if (consoleActive) return;
  window.api.startConsole();
  consoleActive = true;
  $('startConsole').disabled = true;
  $('stopConsole').disabled = false;
  log('Console started', 'info');
});

$('stopConsole').addEventListener('click', () => {
  if (!consoleActive) return;
  window.api.stopConsole();
  consoleActive = false;
  $('startConsole').disabled = false;
  $('stopConsole').disabled = true;
  log('Console stopped', 'info');
});

$('clearConsole').addEventListener('click', () => {
  consoleOutput.innerHTML = '';
});

// Listen for data
if (window.api.onConsoleData) {
  window.api.onConsoleData((data) => {
    // Handle incoming data (might be partial lines)
    // For simplicity, just append. 
    // If data contains newlines, split? 
    // The python script prints lines, so data likely contains newlines.
    // We can just append textContent.
    const span = document.createElement('span');
    span.innerText = data; 
    consoleOutput.appendChild(span);
    consoleOutput.scrollTop = consoleOutput.scrollHeight;
  });
}

// Initial log message
log('Application ready. Click "Toggle LEDs" to send LED_TOG.', 'success');

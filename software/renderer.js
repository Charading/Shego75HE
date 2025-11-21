// Tab switching
const tabButtons = document.querySelectorAll('.tab-button');
const tabContents = document.querySelectorAll('.tab-content');

tabButtons.forEach(button => {
  button.addEventListener('click', () => {
    const tabName = button.dataset.tab;
    
    // Update active states
    tabButtons.forEach(btn => btn.classList.remove('active'));
    tabContents.forEach(content => content.classList.remove('active'));
    
    button.classList.add('active');
    document.getElementById(`${tabName}-tab`).classList.add('active');
  });
});

// GIF Upload functionality
let selectedGif = null;

const selectGifBtn = document.getElementById('select-gif-btn');
const gifInfo = document.getElementById('gif-info');
const gifPreview = document.getElementById('gif-preview');
const statusMessage = document.getElementById('status-message');

const sendScreenBtn = document.getElementById('send-screen');
const sendSdBtn = document.getElementById('send-sd');

selectGifBtn.addEventListener('click', async () => {
  const result = await window.electronAPI.selectGif();
  
  if (result) {
    selectedGif = result;
    
    // Display file info
    gifInfo.innerHTML = `
      <strong>${result.name}</strong><br>
      Size: ${(result.size / 1024).toFixed(2)} KB
    `;
    
    // Display preview
    gifPreview.innerHTML = `<img src="file://${result.path}" alt="GIF Preview">`;
    
    // Enable destination buttons
    sendScreenBtn.disabled = false;
    sendSdBtn.disabled = false;
    
    statusMessage.textContent = '';
  }
});

// Send GIF handlers
async function sendGif(destination) {
  if (!selectedGif) return;
  
  statusMessage.textContent = `Sending to ${destination}...`;
  statusMessage.className = 'status-message info';
  
  const result = await window.electronAPI.sendGif(destination, selectedGif);
  
  if (result.success) {
    statusMessage.textContent = result.message;
    statusMessage.className = 'status-message success';
  } else {
    statusMessage.textContent = `Error: ${result.message}`;
    statusMessage.className = 'status-message error';
  }
}

sendScreenBtn.addEventListener('click', () => sendGif('Screen'));
sendSdBtn.addEventListener('click', () => sendGif('SD'));

// Keyboard Layout functionality
const keyboardLayout = document.getElementById('keyboard-layout');
const keyEditor = document.getElementById('key-editor');
const currentKeyLabel = document.getElementById('current-key-label');
const thresholdSlider = document.getElementById('threshold-slider');
const thresholdValue = document.getElementById('threshold-value');
const applyThresholdBtn = document.getElementById('apply-threshold');
const cancelEditBtn = document.getElementById('cancel-edit');

let currentEditingKey = null;
let keyThresholds = {};

// Standard 75% keyboard layout
const keyLayout = [
  // Row 1
  ['Esc', 'F1', 'F2', 'F3', 'F4', 'F5', 'F6', 'F7', 'F8', 'F9', 'F10', 'F11', 'F12', 'Del', 'Home'],
  // Row 2
  ['`', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 'Backspace', 'End'],
  // Row 3
  ['Tab', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\\', 'PgUp'],
  // Row 4
  ['Caps', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', "'", 'Enter', 'PgDn'],
  // Row 5
  ['Shift', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 'Shift', '↑'],
  // Row 6
  ['Ctrl', 'Win', 'Alt', 'Space', 'Alt', 'Fn', '←', '↓', '→']
];

// Generate keyboard
function generateKeyboard() {
  keyboardLayout.innerHTML = '';
  
  keyLayout.forEach((row, rowIndex) => {
    const rowDiv = document.createElement('div');
    rowDiv.className = 'keyboard-row';
    
    row.forEach((key, keyIndex) => {
      const keyButton = document.createElement('button');
      keyButton.className = 'key';
      keyButton.textContent = key;
      keyButton.dataset.keyId = `${rowIndex}-${keyIndex}`;
      keyButton.dataset.keyLabel = key;
      
      // Add special classes for larger keys
      if (key === 'Backspace' || key === 'Enter') {
        keyButton.classList.add('key-large');
      } else if (key === 'Tab' || key === 'Caps' || key === 'Shift') {
        keyButton.classList.add('key-medium');
      } else if (key === 'Space') {
        keyButton.classList.add('key-space');
      }
      
      // Initialize default threshold
      if (!keyThresholds[keyButton.dataset.keyId]) {
        keyThresholds[keyButton.dataset.keyId] = 2.0;
      }
      
      // Add click handler
      keyButton.addEventListener('click', () => openKeyEditor(keyButton));
      
      rowDiv.appendChild(keyButton);
    });
    
    keyboardLayout.appendChild(rowDiv);
  });
}

function openKeyEditor(keyElement) {
  currentEditingKey = keyElement;
  const keyId = keyElement.dataset.keyId;
  const keyLabel = keyElement.dataset.keyLabel;
  const currentThreshold = keyThresholds[keyId] || 2.0;
  
  currentKeyLabel.textContent = keyLabel;
  thresholdSlider.value = currentThreshold;
  thresholdValue.textContent = currentThreshold.toFixed(1);
  
  keyEditor.classList.remove('hidden');
  
  // Highlight the selected key
  document.querySelectorAll('.key').forEach(k => k.classList.remove('selected'));
  keyElement.classList.add('selected');
}

thresholdSlider.addEventListener('input', (e) => {
  thresholdValue.textContent = parseFloat(e.target.value).toFixed(1);
});

applyThresholdBtn.addEventListener('click', async () => {
  if (!currentEditingKey) return;
  
  const keyId = currentEditingKey.dataset.keyId;
  const newThreshold = parseFloat(thresholdSlider.value);
  
  // Send to keyboard
  const result = await window.electronAPI.updateActuation(keyId, newThreshold);
  
  if (result.success) {
    keyThresholds[keyId] = newThreshold;
    currentEditingKey.classList.add('modified');
    keyEditor.classList.add('hidden');
    currentEditingKey.classList.remove('selected');
    currentEditingKey = null;
  } else {
    alert('Failed to update actuation threshold');
  }
});

cancelEditBtn.addEventListener('click', () => {
  keyEditor.classList.add('hidden');
  if (currentEditingKey) {
    currentEditingKey.classList.remove('selected');
  }
  currentEditingKey = null;
});

// Initialize keyboard on load
generateKeyboard();

// Settings tab handlers
const toggleLedBtn = document.getElementById('toggle-led');
const settingsStatus = document.getElementById('settings-status');
const settingsConsole = document.getElementById('settings-console-log');
const clearConsoleBtn = document.getElementById('clear-settings-console');
const MAX_CONSOLE_ENTRIES = 120;

function appendConsoleLine(message, type = 'info') {
  if (!settingsConsole) return;
  const entry = document.createElement('div');
  entry.className = 'log-entry';
  if (type === 'error') entry.classList.add('log-error');
  else if (type === 'success') entry.classList.add('log-success');
  // timestamp span matches activity log markup
  const ts = document.createElement('span');
  ts.className = 'log-time';
  ts.textContent = `[${new Date().toLocaleTimeString()}] `;
  entry.appendChild(ts);
  const text = document.createElement('span');
  text.textContent = message;
  entry.appendChild(text);
  settingsConsole.insertBefore(entry, settingsConsole.firstChild);

  while (settingsConsole.children.length > MAX_CONSOLE_ENTRIES) {
    settingsConsole.removeChild(settingsConsole.lastChild);
  }
}

if (clearConsoleBtn) {
  clearConsoleBtn.addEventListener('click', () => {
    settingsConsole.innerHTML = '';
  });
}

function formatHexPreview(data, limit = 16) {
  if (!data) return '';
  const arr = Array.from(data);
  const preview = arr.slice(0, limit).map(b => b.toString(16).padStart(2, '0')).join(' ');
  const suffix = arr.length > limit ? ' ...' : '';
  return `${preview}${suffix}`;
}

if (toggleLedBtn) {
  toggleLedBtn.addEventListener('click', async () => {
    settingsStatus.textContent = 'Sending LED toggle...';
    settingsStatus.className = 'status-message info';
    const result = await window.electronAPI.toggleLed();
    if (result && result.success) {
      const extra = result.stdout ? ` (${result.stdout})` : '';
      settingsStatus.textContent = `LED toggled${extra}`;
      settingsStatus.className = 'status-message success';
      if (result.stdout) {
        appendConsoleLine(result.stdout, 'success');
      } else {
        appendConsoleLine('LED toggle command sent.', 'success');
      }
    } else {
      const msg = result && result.message ? result.message : 'Unknown error';
      settingsStatus.textContent = `Failed: ${msg}`;
      settingsStatus.className = 'status-message error';
      appendConsoleLine(`Toggle failed: ${msg}`, 'error');
    }
  });
}

if (window.electronAPI && window.electronAPI.onHidStatus) {
  window.electronAPI.onHidStatus((payload) => {
    if (!payload) return;
    appendConsoleLine(`Status ${payload.code}: ${payload.message}`, payload.code === 0 ? 'success' : 'info');
  });
}

if (window.electronAPI && window.electronAPI.onHidData) {
  window.electronAPI.onHidData((buffer) => {
    appendConsoleLine(`HID data: ${formatHexPreview(buffer)}`, 'data');
  });
}

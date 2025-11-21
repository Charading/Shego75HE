let HID = null;
try {
  HID = require('node-hid');
} catch (e) {
  console.warn('node-hid module not available; device listing will be empty.');
}

const { execFile } = require('child_process');
const { promisify } = require('util');
const execFileAsync = promisify(execFile);
const path = require('path');

const PYTHON_EXE = process.env.PYTHON || 'python';
const PYTHON_SCRIPT = path.join(__dirname, 'scripts', 'toggle_led.py');

function listDevices() {
  if (!HID) return [];
  try {
    return HID.devices();
  } catch (err) {
    console.warn('listDevices failed:', err.message);
    return [];
  }
}

function findDevice(vendorId, productId) {
  if (!HID) return null;
  const devices = HID.devices();
  return devices.find(d => d.vendorId === vendorId && d.productId === productId) || null;
}

function summarizeDevice(dev) {
  if (!dev) return null;
  return {
    path: dev.path,
    vendorId: dev.vendorId,
    productId: dev.productId,
    interface: dev.interface,
    usagePage: dev.usagePage,
    usage: dev.usage,
    manufacturer: dev.manufacturer,
    product: dev.product
  };
}

async function sendCommand(vendorId, productId, commandByte) {
  const vidHex = vendorId.toString(16).padStart(4, '0').toUpperCase();
  const pidHex = productId.toString(16).padStart(4, '0').toUpperCase();
  const cmdHex = commandByte.toString(16).padStart(2, '0').toUpperCase();

  const args = [
    PYTHON_SCRIPT,
    '--vid', vidHex,
    '--pid', pidHex,
    '--cmd', cmdHex,
  ];

  let stdout = '';
  let stderr = '';
  try {
    const result = await execFileAsync(PYTHON_EXE, args, { cwd: path.dirname(PYTHON_SCRIPT) });
    stdout = result.stdout ? result.stdout.toString() : '';
    stderr = result.stderr ? result.stderr.toString() : '';
  } catch (error) {
    const errStdErr = error.stderr ? error.stderr.toString() : '';
    throw new Error(`Python RawHID send failed: ${error.message}\n${errStdErr}`);
  }

  return {
    success: true,
    response: null,
    device: summarizeDevice(findDevice(vendorId, productId)),
    fallback: 'python',
    stdout,
    stderr
  };
}

module.exports = {
  listDevices,
  findDevice,
  sendCommand
};

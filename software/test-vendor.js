#!/usr/bin/env node
// test-vendor.js - Test vendor bulk endpoint communication

const fs = require('fs');
const path = require('path');
const config = require('./config');

let USB;
try {
  USB = require('usb');
  console.log('✅ USB library loaded');
} catch (err) {
  console.error('❌ USB library not found. Run: npm install usb');
  process.exit(1);
}

const VID = config.KEYBOARD_VID;
const PID = config.KEYBOARD_PID;

function findVendorInterface() {
  const device = USB.findByIds(VID, PID);
  if (!device) {
    throw new Error(`Device not found (VID: 0x${VID.toString(16)}, PID: 0x${PID.toString(16)})`);
  }

  device.open();
  
  // Look for vendor interface (class 0xFF)
  let vendorIface = null;
  for (let i = 0; i < device.interfaces.length; i++) {
    const iface = device.interfaces[i];
    console.log(`Interface ${i}: class=0x${iface.descriptor.bInterfaceClass.toString(16)}, subclass=0x${iface.descriptor.bInterfaceSubClass.toString(16)}`);
    
    if (iface.descriptor.bInterfaceClass === 0xFF) {
      vendorIface = iface;
      console.log(`✅ Found vendor interface: ${i}`);
      break;
    }
  }

  if (!vendorIface) {
    device.close();
    throw new Error('Vendor interface not found. Make sure VENDOR_ENABLE is defined in firmware and rebuild.');
  }

  try {
    // Detach kernel driver if necessary (Linux/macOS).
    // On Windows libusb's isKernelDriverActive() may throw LIBUSB_ERROR_NOT_SUPPORTED;
    // call it in its own try/catch so we can proceed to claim() even when unsupported.
    try {
      if (vendorIface.isKernelDriverActive && vendorIface.isKernelDriverActive()) {
        try {
          vendorIface.detachKernelDriver();
        } catch (e) {
          console.warn('Warning: detachKernelDriver failed:', e && e.message ? e.message : e);
        }
      }
    } catch (e) {
      // isKernelDriverActive not supported on this platform/backend — ignore and continue
      console.warn('Notice: isKernelDriverActive() not supported:', e && e.message ? e.message : e);
    }

    vendorIface.claim();
  } catch (error) {
    device.close();
    throw new Error(`Failed to claim vendor interface: ${error.message}`);
  }

  // Find endpoints
  let outEp = null;
  let inEp = null;
  for (const endpoint of vendorIface.endpoints) {
    console.log(`Endpoint: address=0x${endpoint.address.toString(16)}, direction=${endpoint.direction}, type=${endpoint.transferType}`);
    
    if (endpoint.direction === 'out' && endpoint.transferType === USB.LIBUSB_TRANSFER_TYPE_BULK) {
      outEp = endpoint;
      console.log(`✅ Found vendor OUT endpoint: 0x${endpoint.address.toString(16)}`);
    } else if (endpoint.direction === 'in' && endpoint.transferType === USB.LIBUSB_TRANSFER_TYPE_BULK) {
      inEp = endpoint;
      console.log(`✅ Found vendor IN endpoint: 0x${endpoint.address.toString(16)}`);
    }
  }

  if (!outEp) {
    vendorIface.release();
    device.close();
    throw new Error('Vendor OUT endpoint not found');
  }

  return { device, interface: vendorIface, outEndpoint: outEp, inEndpoint: inEp };
}

function sendTestFrame(outEndpoint, reportId, payload = Buffer.alloc(0)) {
  const frame = Buffer.alloc(64);
  frame[0] = reportId;
  if (payload.length > 0) {
    payload.copy(frame, 1, 0, Math.min(payload.length, 63));
  }
  
  console.log(`📤 Sending: ID=0x${reportId.toString(16)}, payload=${payload.length} bytes`);
  
  return new Promise((resolve, reject) => {
    outEndpoint.transfer(frame, (error) => {
      if (error) {
        reject(error);
      } else {
        resolve();
      }
    });
  });
}

async function testBasicCommunication() {
  console.log('🧪 Testing basic vendor communication...\n');
  
  try {
    const { device, interface: vendorIface, outEndpoint, inEndpoint } = findVendorInterface();
    
    // We'll use one-shot IN transfers (transfer(length, callback)) instead of startPoll
    // to avoid pending-poll races on Windows/libusb.
    
    // Test 1: Send a simple status request and wait for a one-shot IN response
    console.log('Test 1: Status request');
    await sendTestFrame(outEndpoint, config.HID_REPORT_ID_STATUS);
    if (inEndpoint) {
      await new Promise((resolve) => {
        inEndpoint.transfer(64, (err, data) => {
          if (err) {
            console.warn('IN transfer error (status request):', err && err.message ? err.message : err);
            return resolve();
          }
          const arr = Array.from(data || []);
          if (arr.length > 0) console.log(`📨 Response: ${arr.slice(0, 8).map(b => '0x' + b.toString(16).padStart(2, '0')).join(' ')}${arr.length > 8 ? ' ...' : ''}`);
          resolve();
        });
      });
    }
    
    // Test 2: Send a fake START command (very small size)
    console.log('\nTest 2: Fake START command');
    const startPayload = Buffer.alloc(3);
    startPayload.writeUInt16LE(10, 0); // fake size = 10 bytes
    startPayload[2] = config.DEST_SCREEN; // destination
    await sendTestFrame(outEndpoint, config.CMD_START_GIF, startPayload);
    if (inEndpoint) {
      await new Promise((resolve) => {
        inEndpoint.transfer(64, (err, data) => {
          if (err) {
            console.warn('IN transfer error (start):', err && err.message ? err.message : err);
            return resolve();
          }
          const arr = Array.from(data || []);
          if (arr.length > 0) console.log(`📨 Response: ${arr.slice(0, 8).map(b => '0x' + b.toString(16).padStart(2, '0')).join(' ')}${arr.length > 8 ? ' ...' : ''}`);
          resolve();
        });
      });
    }
    
    // Test 3: Send an END command 
    console.log('\nTest 3: END command');
    const endPayload = Buffer.alloc(1);
    endPayload[0] = config.DEST_SCREEN;
    await sendTestFrame(outEndpoint, config.CMD_END_GIF, endPayload);
    if (inEndpoint) {
      await new Promise((resolve) => {
        inEndpoint.transfer(64, (err, data) => {
          if (err) {
            console.warn('IN transfer error (end):', err && err.message ? err.message : err);
            return resolve();
          }
          const arr = Array.from(data || []);
          if (arr.length > 0) console.log(`📨 Response: ${arr.slice(0, 8).map(b => '0x' + b.toString(16).padStart(2, '0')).join(' ')}${arr.length > 8 ? ' ...' : ''}`);
          resolve();
        });
      });
    }
    
    console.log('\n✅ Test completed! Check firmware debug output for I2C messages.');
    
    // Cleanup - stop polling and release cleanly before closing device
    if (inEndpoint) {
      try {
        inEndpoint.stopPoll();
      } catch (e) {
        console.warn('Warning: stopPoll() threw:', e && e.message ? e.message : e);
      }
      // Remove listeners so no callbacks keep the endpoint busy
      try { inEndpoint.removeAllListeners && inEndpoint.removeAllListeners('data'); } catch (_) {}
      // Allow any pending callbacks/transfers to finish
      await new Promise((resolve) => setTimeout(resolve, 1000));
    }

    // Release the interface (use callback form to ensure completion)
    await new Promise((resolve, reject) => {
      try {
        vendorIface.release(true, (err) => {
          if (err) return reject(err);
          resolve();
        });
      } catch (e) {
        // Some platforms may throw synchronously; try to continue
        console.warn('Warning: interface.release threw:', e && e.message ? e.message : e);
        resolve();
      }
    });

    try {
      device.close();
    } catch (e) {
      console.warn('Warning: device.close() failed:', e && e.message ? e.message : e);
    }
    
  } catch (error) {
    console.error('❌ Test failed:', error.message);
    console.error('\n💡 Troubleshooting:');
    console.error('1. Make sure firmware is built with VENDOR_ENABLE');
    console.error('2. On Windows, use Zadig to bind vendor interface to WinUSB driver');
    console.error('3. Check that device is properly enumerated with vendor interface');
    process.exit(1);
  }
}

if (require.main === module) {
  testBasicCommunication();
}
"""
Quick HID single-packet tester for Windows/macOS/Linux using hidapi.

Usage:
  python test_single_packet_py.py

Optional args:
  --vid  --pid  --interface  --report-size  --timeout

Install requirements:
  pip install hidapi

This script searches for a matching VID/PID (and interface if provided), opens
the device, writes a single START packet and then listens for up to --timeout
seconds for any input reports. Useful to detect whether the keyboard firmware
actually receives the packet (it will also show the low-level echo common on
Windows).

By default it uses VID=0xDEAD and PID=0xC0DE to match the project's config.js.
"""

import hid
import time
import argparse
import struct
import sys

DEFAULT_VID = 0xDEAD
DEFAULT_PID = 0xC0DE
DEFAULT_INTERFACE = 1
REPORT_SIZE = 32
TIMEOUT_S = 2.0

CMD_START_GIF = 0x10
CMD_GIF_DATA = 0x11
CMD_END_GIF = 0x12


def hexdump(b):
    return ' '.join('0x' + format(x, '02x') for x in b)


def find_device(vid, pid, interface=None):
    devs = hid.enumerate()
    matches = []
    for d in devs:
        if d['vendor_id'] == vid and d['product_id'] == pid:
            matches.append(d)
    if not matches:
        return None
    if interface is None:
        return matches[0]
    # prefer exact interface match
    for d in matches:
        # some platforms expose 'interface_number' key, some 'interface'
        iface = d.get('interface_number') if 'interface_number' in d else d.get('interface')
        if iface == interface:
            return d
    # fallback to first match
    return matches[0]


def build_start_packet(size=100, destination=1, report_size=REPORT_SIZE):
    # buffer length when writing to node-hid / hidapi on Windows is report_size + 1
    buf = bytearray(report_size + 1)
    buf[0] = 0x00  # Report ID required on Windows
    buf[1] = CMD_START_GIF
    # Write 2-byte size little-endian at payload offset 1..2 (firmware expects at payload[1], payload[2])
    buf[2] = size & 0xFF
    buf[3] = (size >> 8) & 0xFF
    buf[4] = destination & 0xFF
    return buf


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--vid', type=lambda x: int(x, 0), default=DEFAULT_VID)
    p.add_argument('--pid', type=lambda x: int(x, 0), default=DEFAULT_PID)
    p.add_argument('--interface', type=int, default=DEFAULT_INTERFACE)
    p.add_argument('--report-size', type=int, default=REPORT_SIZE)
    p.add_argument('--timeout', type=float, default=TIMEOUT_S)
    args = p.parse_args()

    print('=== Single Packet Test ===\n')
    print(f'Looking for VID: 0x{args.vid:04x}, PID: 0x{args.pid:04x}')
    devinfo = find_device(args.vid, args.pid, args.interface)
    if not devinfo:
        print('Keyboard not found. Devices enumerated:')
        for d in hid.enumerate():
            print(d)
        sys.exit(1)

    print('✅ Found keyboard:', devinfo.get('path') or devinfo.get('product_string'))

    # Open device
    dev = hid.device()
    try:
        # prefer open_path if available
        path = devinfo.get('path')
        if path:
            try:
                dev.open_path(path)
            except Exception:
                # fallback to open(vid,pid)
                dev.open(devinfo['vendor_id'], devinfo['product_id'])
        else:
            dev.open(devinfo['vendor_id'], devinfo['product_id'])
    except Exception as e:
        print('Failed to open device:', e)
        sys.exit(1)

    print('\n✅ Device opened')

    # Build and send a START packet
    pkt = build_start_packet(size=100, destination=1, report_size=args.report_size)
    print('\n📨 Sending START packet:')
    print('Report ID: 0x00 (Windows requirement)')
    print('Command:', hex(CMD_START_GIF), '(START_GIF)')
    print('Size: 100 bytes (test value)')
    print('Destination: 0x1 (Screen)')
    print('Full packet:', hexdump(pkt[:16]), '...')

    # write expects a bytes-like or list of ints
    try:
        dev.write(bytes(pkt))
    except Exception as e:
        # try list form
        try:
            dev.write(list(pkt))
        except Exception as ex:
            print('Write error:', e, ex)
            dev.close()
            sys.exit(1)

    print('\n✅ Packet sent!')
    print('\n⏳ Waiting', args.timeout, 'seconds for responses...\n')

    # Listen for up to timeout seconds
    dev.set_nonblocking(True)
    start = time.time()
    seen = 0
    while time.time() - start < args.timeout:
        try:
            data = dev.read(args.report_size + 1)
        except Exception:
            data = None
        if not data:
            time.sleep(0.01)
            continue
        seen += 1
        # data may be returned as list of ints or bytes
        if isinstance(data, list):
            b = bytes(data)
        elif isinstance(data, (bytes, bytearray)):
            b = bytes(data)
        else:
            b = bytes(data)

        # Normalize: if first byte is 0x00 (report id) strip it for display
        if len(b) > 0 and b[0] == 0x00:
            disp = b[1:]
        else:
            disp = b

        print(f'[RECEIVED #{seen}]')
        print('First 10 bytes:', hexdump(disp[:10]))

        # Heuristic: if the first payload byte equals our START command (0x10) it's an echo
        if len(disp) > 0 and disp[0] == CMD_START_GIF:
            print('Type: ECHO of START packet (this is normal on Windows)')
        else:
            print('Type: Incoming from device (may be status/ACK)')

    print('=== Test Complete ===\n')
    print('✅ Received', seen, 'packet(s) (likely an echo - normal on Windows)')
    print('🔎 Now check keyboard and ESP32 debug output!')

    dev.close()


if __name__ == '__main__':
    main()

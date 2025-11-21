#!/usr/bin/env python3
"""
Minimal RawHID toggle helper (clean, single-file).

Sends a 32-byte output report: [report_id, cmd, 0,0,...] and waits briefly
for a single non-echo reply. Prefers the RawHID interface (usage_page 0xFF60,
usage 0x61) when available.

Usage examples:
    python rgb_toggle.py            # send 'T' (toggle) to VID=DEAD PID=C0DE
    python rgb_toggle.py --cmd T    # explicit command (still uses DEAD/C0DE)
    python rgb_toggle.py --path "\\\\?\\HID#..."  # target a specific path
"""
import sys
import time
import argparse

try:
    import hid
except Exception as e:
    print('Missing dependency: python hid (hidapi).', e)
    sys.exit(2)

RAW_USAGE_PAGE = 0xFF60
RAW_USAGE = 0x61
PAD_SIZE = 32
APP_MAGIC = 0xA5


def open_device_by_path(path):
    d = hid.device()
    # Try common encodings/variants for Windows paths
    attempts = [path]
    if isinstance(path, str) and path.startswith('\\\\?\\'):
        attempts.append(path[4:])
    for p in attempts:
        try:
            d.open_path(p)
            return d
        except Exception:
            pass
        try:
            d.open_path(p.encode('utf-8'))
            return d
        except Exception:
            pass
    raise RuntimeError('Unable to open device path')


def find_preferred_interface(vid, pid):
    all_devs = list(hid.enumerate())
    vidpid = [d for d in all_devs if d['vendor_id'] == vid and d['product_id'] == pid]
    if not vidpid:
        return None
    preferred = [d for d in vidpid if d.get('usage_page') == RAW_USAGE_PAGE and d.get('usage') == RAW_USAGE]
    if preferred:
        return preferred[0]
    return vidpid[0]


def try_toggle(vid, pid, report_id, cmd_byte, path=None, timeout=2.0, mode='both'):
    if path:
        dev = open_device_by_path(path)
    else:
        devinfo = find_preferred_interface(vid, pid)
        if not devinfo:
            print('Device not found')
            return 2
        dev = hid.device()
        p = devinfo.get('path')
        try:
            dev.open_path(p)
        except Exception:
            try:
                dev.open_path(p.encode('utf-8'))
            except Exception:
                try:
                    dev.open_path(p.decode() if isinstance(p, (bytes, bytearray)) else p)
                except Exception as e:
                    print('Failed to open enumerated path:', e)
                    return 3

    try:
        # Print which device path we're using for easier debugging
        try:
            used_path = p if not path else path
        except Exception:
            used_path = path or getattr(dev, 'path', None)
        print('Using device path:', used_path)
        # Build a 32-byte payload (QMK Raw HID expects 32 bytes).
        # Place APP_MAGIC as the first command byte (data[0] on firmware side):
        # payload32: [report_id, APP_MAGIC, cmd, ...]
        payload32 = bytes([report_id, APP_MAGIC, cmd_byte] + [0] * (PAD_SIZE - 3))
        assert len(payload32) == PAD_SIZE

        # Some backends (notably hidapi on Windows) expect a leading extra
        # Report ID byte when calling write(), producing a 33-byte transfer.
        # Try the 33-byte style first (b'\x00'+payload32), then fall back to
        # the plain 32-byte payload if no response is seen.
        if mode == '33':
            attempts = [b"\x00" + payload32]
        elif mode == '32':
            attempts = [payload32]
        else:
            attempts = [b"\x00" + payload32, payload32]
        for attempt in attempts:
            print('TX:', ' '.join(f"{b:02X}" for b in attempt[:8]), '...')
            try:
                written = dev.write(attempt)
            except Exception as e:
                print('write() failed:', e)
                written = 0
            print('WROTE', written)

            dev.set_nonblocking(True)
            start = time.time()
            got_response = False
            while (time.time() - start) < timeout:
                try:
                    r = dev.read(64)
                except Exception:
                    r = None
                if r:
                    rb = bytes(r)
                    if rb.startswith(b"\xFF") and all(x == 0x00 for x in rb[1:]):
                        continue
                    # ignore echoes that match our transmitted payloads
                    if rb == attempt or rb[:len(attempt)] == attempt:
                        continue
                    print('RESP:', ' '.join(f"{b:02X}" for b in rb))
                    return 0
                time.sleep(0.02)
            # no response for this format - try next
        print('No response')
        return 1
    finally:
        try:
            dev.close()
        except Exception:
            pass


def parse_cmd_arg(s):
    if len(s) == 1:
        return ord(s)
    try:
        return int(s, 16)
    except Exception:
        try:
            return int(s)
        except Exception:
            return ord(s[0])


def main(argv):
    p = argparse.ArgumentParser()
    # VID/PID are fixed for this project (you requested always using DEAD/C0DE)
    # Do not require the user to specify them.
    p.add_argument('--path', default=None)
    p.add_argument('--reportid', default='00')
    p.add_argument('--cmd', default='T')
    p.add_argument('--timeout', default='2')
    p.add_argument('--mode', choices=['33', '32', 'both'], default='both', help='Which write format to try: 33-byte (leading 0x00), 32-byte, or both')
    p.add_argument('--vid', default=None)
    p.add_argument('--pid', default=None)
    args = p.parse_args(argv)

    # Hardcoded VID/PID (use these regardless of what the caller passes)
    # Accept --vid/--pid if provided by external callers, but ignore them
    # so the script always targets the intended device.
    vid = 0xDEAD
    pid = 0xC0DE

    try:
        report_id = int(args.reportid, 16)
    except Exception:
        report_id = int(args.reportid)

    cmd_byte = parse_cmd_arg(args.cmd)
    timeout = float(args.timeout)

    return try_toggle(vid, pid, report_id, cmd_byte, path=args.path, timeout=timeout, mode=args.mode)


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))

#!/usr/bin/env python3
"""
Simple RawHID sender for your QMK board.
Sends a 32-byte packet and DOES NOT expect an ACK.
"""

import sys
import argparse

import hid   # using hid.device(), not hid.Device()

CMD_SET_FEATURE = 0x10
RAW_USAGE_PAGE = 0xFF60
RAW_USAGE      = 0x61
PAD_SIZE       = 32
APP_MAGIC      = 0xA5


def list_devices():
    for d in hid.enumerate():
        print({
            'path': d['path'],
            'vendor_id': hex(d['vendor_id']),
            'product_id': hex(d['product_id']),
            'usage_page': d.get('usage_page'),
            'usage': d.get('usage'),
            'interface_number': d.get('interface_number')
        })


def open_device(path, vid, pid):
    # User explicitly provided a path
    if path:
        dev = hid.device()
        # open_path expects a bytes path on some platforms; try str then bytes
        try:
            dev.open_path(path)
        except Exception:
            try:
                dev.open_path(path.encode('utf-8'))
            except Exception as e:
                raise
        return dev

    # Auto-detect correct RawHID interface
    for d in hid.enumerate():
        if (d['vendor_id'] == vid and
            d['product_id'] == pid and
            d.get('usage_page') == RAW_USAGE_PAGE and
            d.get('usage') == RAW_USAGE):

            print("Opening RawHID interface at path:", d['path'])
            dev = hid.device()
            try:
                dev.open_path(d['path'])
            except Exception:
                dev.open_path(d['path'].encode('utf-8'))
            return dev

    raise RuntimeError("RawHID interface NOT found. Is RAW_ENABLE enabled?")


def send_only(path, vid, pid, report_id, cmd, feature, value):
    dev = open_device(path, vid, pid)

    # Match legacy packet shape: [report_id][magic][cmd][feature][value]...
    payload = [report_id, APP_MAGIC, cmd, feature, value]
    payload += [0] * (PAD_SIZE - len(payload))

    print("Sending:", " ".join(f"{b:02X}" for b in payload[:8]), "...")

    written = dev.write(bytes(payload))
    print("Bytes written:", written)

    dev.close()
    print("Done (no ACK expected)")


def main(argv):
    p = argparse.ArgumentParser()
    p.add_argument("--list", action="store_true", help="List devices")
    p.add_argument("--path", help="Open specific HID path")
    p.add_argument("--vid", default="DEAD")
    p.add_argument("--pid", default="C0DE")
    p.add_argument("--reportid", default="00")  # used as command byte in payload[0]
    p.add_argument("--cmd", default="00")
    p.add_argument("--feature", default="00")
    p.add_argument("--value", default="00")
    args = p.parse_args(argv)

    if args.list:
        list_devices()
        return 0

    send_only(
        args.path,
        int(args.vid, 16),
        int(args.pid, 16),
        int(args.reportid, 16),
        int(args.cmd, 16),
        int(args.feature, 16),
        int(args.value, 16),
    )

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

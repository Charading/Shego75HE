#!/usr/bin/env python3
"""RawHID command sender used by shego_app.

Node's HID stack cannot send our custom RawHID reports reliably, so this
Python helper (using hidapi) does all outbound commands.  It can toggle the
LED transistor (default command 0x30) or send arbitrary command bytes by
passing different arguments.
"""

import argparse
import sys
from typing import Iterable, List, Optional

import hid  # type: ignore

RAW_USAGE_PAGE = 0xFF60
RAW_USAGE = 0x61
PAD_SIZE = 32
APP_MAGIC = 0xA5


def format_device(d: dict) -> str:
    usage_page = d.get("usage_page")
    usage = d.get("usage")
    iface = d.get("interface_number")
    return (
        f"path={d.get('path')} "
        f"vid=0x{d.get('vendor_id'):04X} pid=0x{d.get('product_id'):04X} "
        f"usage_page={usage_page if usage_page is None else hex(usage_page)} "
        f"usage={usage if usage is None else hex(usage)} "
        f"iface={iface}"
    )


def list_devices() -> None:
    print("Available HID interfaces:")
    for dev in hid.enumerate():
        print("  -", format_device(dev))


def parse_hex_byte(value: str) -> int:
    value = value.strip().replace("0x", "")
    return int(value, 16) & 0xFF


def parse_extra(bytes_list: Iterable[str]) -> List[int]:
    extra: List[int] = []
    for token in bytes_list:
        token = token.strip()
        if not token:
            continue
        # allow comma-separated blobs
        parts = [p for p in token.replace(",", " ").split(" ") if p]
        for part in parts:
            extra.append(parse_hex_byte(part))
    return extra


def open_device(path: Optional[str], vid: int, pid: int, usage_page: int, usage: int):
    if path:
        dev = hid.device()
        try:
            dev.open_path(path)
        except Exception:
            dev.open_path(path.encode("utf-8"))
        return dev

    preferred = None
    fallback = None
    for entry in hid.enumerate():
        if entry.get("vendor_id") != vid or entry.get("product_id") != pid:
            continue
        if entry.get("usage_page") == usage_page and entry.get("usage") == usage:
            preferred = entry
            break
        if not fallback:
            fallback = entry

    target = preferred or fallback
    if not target:
        raise RuntimeError("Matching HID interface not found. Is RAW_ENABLE enabled?")

    dev = hid.device()
    try:
        dev.open_path(target["path"])
    except Exception:
        dev.open_path(target["path"].encode("utf-8"))
    return dev


def build_payload(report_id: int, cmd: int, feature: int, value: int, extra: List[int]) -> bytes:
    payload = [report_id & 0xFF, APP_MAGIC & 0xFF, cmd & 0xFF, feature & 0xFF, value & 0xFF]
    payload.extend(b & 0xFF for b in extra)
    if len(payload) < PAD_SIZE:
        payload.extend([0] * (PAD_SIZE - len(payload)))
    return bytes(payload[:PAD_SIZE])


def send_command(path: Optional[str], vid: int, pid: int, usage_page: int, usage: int,
                 report_id: int, cmd: int, feature: int, value: int, extra: List[int]) -> None:
    dev = open_device(path, vid, pid, usage_page, usage)
    try:
        payload = build_payload(report_id, cmd, feature, value, extra)
        print("Sending:", " ".join(f"{b:02X}" for b in payload[:8]), "...")
        written = dev.write(payload)
        print(f"Bytes written: {written}")
    finally:
        try:
            dev.close()
        except Exception:
            pass


def main(argv: List[str]) -> int:
    parser = argparse.ArgumentParser(description="Send RawHID commands (default LED toggle 0x30)")
    parser.add_argument("--list", action="store_true", help="List available HID interfaces")
    parser.add_argument("--path", help="Explicit HID device path", default=None)
    parser.add_argument("--vid", default="DEAD")
    parser.add_argument("--pid", default="C0DE")
    parser.add_argument("--usage-page", default=f"{RAW_USAGE_PAGE:04X}")
    parser.add_argument("--usage", default=f"{RAW_USAGE:02X}")
    parser.add_argument("--reportid", default="00")
    parser.add_argument("--cmd", default="30")
    parser.add_argument("--feature", default="00")
    parser.add_argument("--value", default="00")
    parser.add_argument("--extra", nargs="*", default=[], help="Additional hex bytes appended to payload")
    args = parser.parse_args(argv)

    if args.list:
        list_devices()
        return 0

    vid = int(args.vid, 16)
    pid = int(args.pid, 16)
    usage_page = int(args.usage_page, 16)
    usage = int(args.usage, 16)
    report_id = parse_hex_byte(args.reportid)
    cmd = parse_hex_byte(args.cmd)
    feature = parse_hex_byte(args.feature)
    value = parse_hex_byte(args.value)
    extra = parse_extra(args.extra)

    send_command(args.path, vid, pid, usage_page, usage, report_id, cmd, feature, value, extra)
    print("Done (no ACK expected)")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

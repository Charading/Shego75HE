import hid

VID = 0xDEAD
PID = 0xC0DE

print(f"Searching for HID interfaces for VID=0x{VID:04X}, PID=0x{PID:04X}\n")

devices = hid.enumerate(VID, PID)

if not devices:
    print("No HID interfaces found.")
    exit(0)

for d in devices:
    print("=== HID Interface ===")
    print(f"Path:        {d['path']}")
    print(f"Interface #: {d.get('interface_number')}")
    print(f"Usage Page:  0x{d.get('usage_page', 0):04X}")
    print(f"Usage:       0x{d.get('usage', 0):04X}")
    print(f"Manufacturer: {d.get('manufacturer_string')}")
    print(f"Product:      {d.get('product_string')}")
    print(f"Serial:       {d.get('serial_number')}")
    print()

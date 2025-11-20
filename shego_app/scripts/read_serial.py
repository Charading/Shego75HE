import serial
import serial.tools.list_ports
import sys
import time

def find_pico_port():
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        # Adjust VID:PID for your device if needed. 
        # Standard Pico is 2E8A:000A (MicroPython) or similar.
        # QMK often uses FEED:6060 or similar.
        # For now, we'll just look for a likely candidate or print all.
        if "2E8A" in p.hwid or "FEED" in p.hwid or "Pico" in p.description:
            return p.device
    return None

def main():
    port = None
    if len(sys.argv) > 1:
        port = sys.argv[1]
    
    if not port:
        print("Searching for device...")
        port = find_pico_port()
        
    if not port:
        print("No suitable serial port found.")
        # List all ports for debugging
        ports = list(serial.tools.list_ports.comports())
        for p in ports:
            print(f"  {p.device}: {p.description} [{p.hwid}]")
        return

    print(f"Connecting to {port}...")
    try:
        ser = serial.Serial(port, 115200, timeout=0.1)
        print(f"Connected to {port}. Listening...")
        while True:
            try:
                line = ser.readline()
                if line:
                    try:
                        print(line.decode('utf-8', errors='replace').strip())
                        sys.stdout.flush()
                    except Exception:
                        pass
            except serial.SerialException:
                print("Serial connection lost.")
                break
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    main()

"""Open COM5 after upload reset, wait for boot output (3s delay + I2C scan)."""
import serial, time

PORT = "COM5"
BAUD = 115200
CAPTURE_SECONDS = 25  # 3s delay + I2C scan + boot sequence + margin

# Wait for port after upload hard-reset
print(f"Waiting for {PORT}...", flush=True)
ser = None
for attempt in range(40):
    try:
        ser = serial.Serial(PORT, BAUD, timeout=1)
        print(f"Connected on attempt {attempt+1}", flush=True)
        break
    except Exception:
        time.sleep(0.5)

if ser is None:
    print("ERROR: Could not connect to COM5")
    exit(1)

ser.reset_input_buffer()
print(f"--- Capturing for {CAPTURE_SECONDS}s ---", flush=True)

start = time.time()
lines = []
while time.time() - start < CAPTURE_SECONDS:
    raw = ser.readline()
    if raw:
        line = raw.decode("utf-8", errors="replace").rstrip()
        lines.append(line)
        print(line, flush=True)

ser.close()

with open("i2c_scan_log.txt", "w", encoding="utf-8") as f:
    for line in lines:
        f.write(line + "\n")

print(f"\n--- Done. {len(lines)} lines captured to i2c_scan_log.txt ---")

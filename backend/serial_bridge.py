"""
PondSense Serial Bridge & Telemetry Parser
Listens to the Farmhouse Base Station Gateway over USB Serial (115200 baud),
validates telemetry JSON packets, logs them to SQLite, and triggers hypoxia alerts.
"""

import sys
import time
import json
import argparse
import random
from database import init_db, insert_telemetry, log_alert

# Safety Thresholds
DO_CRITICAL_THRESHOLD = 3.0    # mg/L (Asphyxiation danger)
DO_WARNING_THRESHOLD  = 4.5    # mg/L
PH_MIN_THRESHOLD      = 6.5
PH_MAX_THRESHOLD      = 8.5
TEMP_MIN_THRESHOLD    = 22.0   # °C
TEMP_MAX_THRESHOLD    = 34.0   # °C

def process_telemetry_line(raw_line: str):
    raw_line = raw_line.strip()
    if not raw_line or not raw_line.startswith("{") or not raw_line.endswith("}"):
        return

    try:
        data = json.loads(raw_line)
    except json.JSONDecodeError:
        print(f"[BRIDGE] Warning: Malformed JSON packet ignored: {raw_line}")
        return

    # Check required fields
    required = ["node_id", "msg_id", "water_temp", "ph", "turbidity", "do", "battery"]
    if not all(k in data for k in required):
        print(f"[BRIDGE] Warning: Missing telemetry keys in: {data}")
        return

    # Insert into SQLite Database
    record_id = insert_telemetry(data)
    print(f"[LOG #{record_id}] Node {data['node_id']} | Msg #{data['msg_id']} | "
          f"Temp: {data['water_temp']:.1f}°C | pH: {data['ph']:.2f} | "
          f"DO: {data['do']:.2f} mg/L | Turb: {data['turbidity']:.1f} NTU | Bat: {data['battery']:.2f}V")

    # Threshold Checking & Alert Triggering
    dissolved_oxygen = data["do"]
    ph = data["ph"]
    node_id = data["node_id"]

    if dissolved_oxygen < DO_CRITICAL_THRESHOLD:
        log_alert(
            node_id=node_id,
            parameter="Dissolved Oxygen",
            value=dissolved_oxygen,
            threshold=DO_CRITICAL_THRESHOLD,
            severity="CRITICAL",
            message=f"CRITICAL HYPOXIA HAZARD: DO level dropped to {dissolved_oxygen:.2f} mg/L! Turn on pond aerators immediately!"
        )
    elif dissolved_oxygen < DO_WARNING_THRESHOLD:
        log_alert(
            node_id=node_id,
            parameter="Dissolved Oxygen",
            value=dissolved_oxygen,
            threshold=DO_WARNING_THRESHOLD,
            severity="WARNING",
            message=f"Caution: DO declining to {dissolved_oxygen:.2f} mg/L."
        )

    if ph < PH_MIN_THRESHOLD or ph > PH_MAX_THRESHOLD:
        log_alert(
            node_id=node_id,
            parameter="pH Level",
            value=ph,
            threshold=PH_MIN_THRESHOLD if ph < PH_MIN_THRESHOLD else PH_MAX_THRESHOLD,
            severity="WARNING",
            message=f"Water pH drift: {ph:.2f} is outside optimal fish health range (6.5 - 8.5)."
        )

def run_simulation():
    """Generates realistic aquaculture 24-hour pond telemetry cycles for benchtop testing."""
    print("\n=======================================================")
    print(" PondSense Telemetry Simulation Mode Active            ")
    print(" Simulating 865 MHz LoRa Telemetry Packets into SQLite ")
    print("=======================================================\n")
    
    msg_id = 100
    while True:
        msg_id += 1
        # Realistic diurnal water parameters:
        # Water temp ~27C - 30C
        # pH ~7.2 - 7.8
        # DO ~4.5 - 6.5 mg/L (occasionally dipping to test alerts)
        water_temp = round(28.0 + random.uniform(-1.5, 1.5), 2)
        ph = round(7.4 + random.uniform(-0.4, 0.4), 2)
        turbidity = round(15.0 + random.uniform(-5.0, 10.0), 1)
        battery = round(4.05 - (msg_id * 0.0001), 2)

        # 10% chance to simulate overnight hypoxia dip to verify alert triggers
        if random.random() < 0.15:
            do_level = round(random.uniform(2.2, 2.9), 2)
            status = "CRITICAL_HYPOXIA"
        else:
            do_level = round(random.uniform(4.8, 6.8), 2)
            status = "NORMAL"

        sample = {
            "node_id": 1,
            "msg_id": msg_id,
            "water_temp": water_temp,
            "ph": ph,
            "turbidity": turbidity,
            "do": do_level,
            "battery": battery,
            "rssi": random.randint(-92, -78),
            "snr": round(random.uniform(6.0, 9.5), 1),
            "status": status
        }

        process_telemetry_line(json.dumps(sample))
        time.sleep(3)

def run_serial(port: str, baudrate: int = 115200):
    try:
        import serial
    except ImportError:
        print("[ERROR] pyserial is not installed. Install via: pip install pyserial")
        sys.exit(1)

    print(f"[BRIDGE] Opening USB Serial Port {port} at {baudrate} baud...")
    try:
        ser = serial.Serial(port, baudrate, timeout=1)
        print(f"[BRIDGE] Connected to Gateway on {port}. Awaiting telemetry...")
        while True:
            line = ser.readline().decode("utf-8", errors="ignore").strip()
            if line:
                process_telemetry_line(line)
    except serial.SerialException as e:
        print(f"[ERROR] Failed to open serial port {port}: {e}")
        print("[TIP] You can test the backend pipeline right now without hardware using: python serial_bridge.py --simulate")

if __name__ == "__main__":
    init_db()
    parser = argparse.ArgumentParser(description="PondSense Serial Gateway Bridge")
    parser.add_argument("--port", type=str, default="COM3", help="Serial port (e.g., COM3 on Windows or /dev/ttyUSB0 on Linux)")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument("--simulate", action="store_true", help="Run simulated telemetry generator for offline testing")
    args = parser.parse_args()

    if args.simulate:
        run_simulation()
    else:
        run_serial(args.port, args.baud)

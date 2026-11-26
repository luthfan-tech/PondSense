"""
PondSense Telemetry Database Interface
SQLite persistent storage for aquaculture telemetry & threshold alerts.
"""

import sqlite3
import os
from datetime import datetime

DB_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), "pondsense.db")

def get_connection():
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    return conn

def init_db():
    conn = get_connection()
    cursor = conn.cursor()

    # Telemetry historical log table
    cursor.execute("""
    CREATE TABLE IF NOT EXISTS telemetry (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
        node_id INTEGER NOT NULL,
        message_id INTEGER NOT NULL,
        water_temp REAL NOT NULL,
        ph REAL NOT NULL,
        turbidity REAL NOT NULL,
        dissolved_oxygen REAL NOT NULL,
        battery_voltage REAL NOT NULL,
        rssi INTEGER,
        snr REAL,
        status TEXT NOT NULL
    );
    """)

    # Alerts table for threshold violations
    cursor.execute("""
    CREATE TABLE IF NOT EXISTS alerts (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
        node_id INTEGER NOT NULL,
        parameter TEXT NOT NULL,
        value REAL NOT NULL,
        threshold REAL NOT NULL,
        severity TEXT NOT NULL,
        message TEXT NOT NULL,
        acknowledged INTEGER DEFAULT 0
    );
    """)

    cursor.execute("CREATE INDEX IF NOT EXISTS idx_telemetry_node_ts ON telemetry(node_id, timestamp);")
    cursor.execute("CREATE INDEX IF NOT EXISTS idx_alerts_unack ON alerts(acknowledged, timestamp);")

    conn.commit()
    conn.close()
    print(f"[DB] Initialized SQLite database at {DB_PATH}")

def insert_telemetry(data: dict) -> int:
    conn = get_connection()
    cursor = conn.cursor()
    cursor.execute("""
    INSERT INTO telemetry (
        node_id, message_id, water_temp, ph, turbidity,
        dissolved_oxygen, battery_voltage, rssi, snr, status
    ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    """, (
        data.get("node_id", 1),
        data.get("msg_id", 0),
        data.get("water_temp", 25.0),
        data.get("ph", 7.0),
        data.get("turbidity", 0.0),
        data.get("do", 5.0),
        data.get("battery", 4.0),
        data.get("rssi", -80),
        data.get("snr", 8.0),
        data.get("status", "NORMAL")
    ))
    record_id = cursor.lastrowid
    conn.commit()
    conn.close()
    return record_id

def log_alert(node_id: int, parameter: str, value: float, threshold: float, severity: str, message: str):
    conn = get_connection()
    cursor = conn.cursor()
    cursor.execute("""
    INSERT INTO alerts (node_id, parameter, value, threshold, severity, message)
    VALUES (?, ?, ?, ?, ?, ?)
    """, (node_id, parameter, value, threshold, severity, message))
    conn.commit()
    conn.close()
    print(f"[ALERT] Logged {severity} alert: {message}")

def get_latest_reading(node_id: int = 1) -> dict:
    conn = get_connection()
    cursor = conn.cursor()
    cursor.execute("""
    SELECT * FROM telemetry WHERE node_id = ? ORDER BY id DESC LIMIT 1
    """, (node_id,))
    row = cursor.fetchone()
    conn.close()
    return dict(row) if row else None

def get_historical_readings(node_id: int = 1, limit: int = 100) -> list:
    conn = get_connection()
    cursor = conn.cursor()
    cursor.execute("""
    SELECT * FROM telemetry WHERE node_id = ? ORDER BY id DESC LIMIT ?
    """, (node_id, limit))
    rows = cursor.fetchall()
    conn.close()
    return [dict(r) for r in reversed(rows)]

if __name__ == "__main__":
    init_db()

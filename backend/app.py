"""
PondSense REST API Server
Exposes aquaculture telemetry & alert feeds to the farmer mobile web dashboard.
"""

from flask import Flask, jsonify, request, send_from_directory
from flask_cors import CORS
import os
import database

app = Flask(__name__, static_folder="../dashboard", static_url_path="/")
CORS(app)

# Ensure database tables exist
database.init_db()

@app.route("/")
def serve_dashboard():
    return send_from_directory(app.static_folder, "index.html")

@app.route("/<path:path>")
def serve_static_files(path):
    return send_from_directory(app.static_folder, path)

@app.route("/api/health", methods=["GET"])
def healthcheck():
    conn = database.get_connection()
    cursor = conn.cursor()
    cursor.execute("SELECT COUNT(*) FROM telemetry")
    count = cursor.fetchone()[0]
    conn.close()
    return jsonify({
        "status": "ONLINE",
        "service": "PondSense Gateway API",
        "telemetry_records": count,
        "frequency": "865.0 MHz (India ISM)"
    })

@app.route("/api/telemetry/latest", methods=["GET"])
def get_latest():
    node_id = request.args.get("node_id", default=1, type=int)
    latest = database.get_latest_reading(node_id)
    if not latest:
        # Default fallback sample if no radio packets captured yet
        return jsonify({
            "node_id": node_id,
            "message_id": 0,
            "timestamp": "Waiting for RF telemetry...",
            "water_temp": 28.0,
            "ph": 7.4,
            "turbidity": 15.0,
            "dissolved_oxygen": 5.5,
            "battery_voltage": 4.10,
            "rssi": -85,
            "snr": 8.0,
            "status": "NORMAL"
        })
    return jsonify(latest)

@app.route("/api/telemetry/history", methods=["GET"])
def get_history():
    node_id = request.args.get("node_id", default=1, type=int)
    limit = request.args.get("limit", default=50, type=int)
    history = database.get_historical_readings(node_id, limit)
    return jsonify(history)

@app.route("/api/alerts", methods=["GET"])
def get_alerts():
    conn = database.get_connection()
    cursor = conn.cursor()
    cursor.execute("""
        SELECT * FROM alerts WHERE acknowledged = 0 ORDER BY id DESC LIMIT 10
    """)
    rows = cursor.fetchall()
    conn.close()
    return jsonify([dict(r) for r in rows])

@app.route("/api/alerts/<int:alert_id>/ack", methods=["POST"])
def acknowledge_alert(alert_id):
    conn = database.get_connection()
    cursor = conn.cursor()
    cursor.execute("UPDATE alerts SET acknowledged = 1 WHERE id = ?", (alert_id,))
    conn.commit()
    conn.close()
    return jsonify({"success": True, "alert_id": alert_id})

if __name__ == "__main__":
    print("[API] Starting PondSense Web Server on http://0.0.0.0:5000")
    app.run(host="0.0.0.0", port=5000, debug=True)

# Day 11: Telegram Emergency Alert Engine & Automated Test Suite

- **Date:** 28 November 2026
- **Phase:** Alert Dispatch System & Integration Verification (Python / Unittest)
- **Focus:** Implementing direct-to-phone emergency Telegram alert dispatching with rate-limiting cooldowns and authoring an 8-test unit verification suite for embedded algorithms and database pipelines.

---

## 1. Automated Telegram Hypoxia Alert Engine ([`backend/telegram_alerts.py`](../../backend/telegram_alerts.py))

A major challenge in smallholder aquaculture is that farmers are asleep during the lethal **2:00 AM – 6:00 AM hypoxia window**. Even with a web dashboard, an asleep farmer will not check a browser tab. PondSense bridges this gap with direct push notifications:

```
[ Sensor Reads DO < 3.0 mg/L ]
              │
              ▼
[ Rule Engine in serial_bridge.py ]
              │
              ▼
[ Cooldown Evaluator (telegram_alerts.py) ]
       ├── If alert sent < 5 mins ago ➔ Suppress duplicate notifications
       └── If new hazard event ➔ Format Urgent Aquaculture Dispatch
              │
              ▼
[ Telegram Bot API (HTTP POST) ]
              │
              ▼
[ Farmer Smartphone Notification (Vibrate + Audio Ping) ]
       "🚨 PONDSENSE CRITICAL EMERGENCY ALERT 🚨
        📍 Monitored Node: Pond #1 (Nursery)
        📊 Detected Condition: Dissolved Oxygen Crash (< 3.0 mg/L)
        📉 Measured Value: 2.38 mg/L
        🕒 Timestamp: 03:22 AM
        ⚠️ IMMEDIATE ACTION: Turn on pond aerators immediately!"
```

### Safety Features:
* **Hysteresis & Cooldown:** Enforces a 5-minute deadband between repeat dispatches to prevent flooding the farmer with 100 messages a minute while aerators run.
* **Zero-Crash Graceful Fallback:** If no Telegram Bot token is configured (e.g. offline benchtop development), the system automatically falls back to clean, formatted terminal logging without throwing exceptions or blocking the serial pipeline.

---

## 2. Automated Test Suite ([`tests/`](../../tests/))

To ensure rock-solid numerical accuracy and fail-safe operation before deploying in water, an automated test suite was constructed:

### A. Embedded Algorithms Suite ([`tests/test_telemetry.py`](../../tests/test_telemetry.py))
1. **CRC16-CCITT Test Vector:** Validates radio packet integrity calculation against the standard international test vector (`0x29B1` for `"123456789"`).
2. **Benson-Krause Oxygen Solubility:** Proves fresh water maximum oxygen capacity drops predictably with heat (`14.65 mg/L` at 0°C down to `7.56 mg/L` at 30°C).
3. **Nernst Slope Compensation:** Verifies temperature scaling of analog pH voltage response (`-59.16 mV/pH` at 25°C).
4. **TS-300B Turbidity Polynomial Curve:** Verifies clear water threshold clamping (`> 2.5V ➔ 0 NTU`) and high-silt response.

### B. Backend Pipeline Suite ([`tests/test_backend.py`](../../tests/test_backend.py))
1. **SQLite CRUD:** Tests thread-safe telemetry insertion, indexing, and latest-sample retrieval.
2. **Hypoxia Hazard Logging:** Proves unacknowledged critical alerts persist across server restarts.
3. **Telegram Dispatcher Mock:** Proves push notification payloads format accurately without network dependencies.

```bash
# Running the complete test suite:
python -m unittest discover -s tests -v

# Output:
# Ran 8 tests in 0.055s
# OK
```

---

## 3. Next Steps (Day 12)
* Begin **Phase 2: Enclosure 3D Print Preparation (Slicing & Toolpaths)**:
  * Export high-resolution `.STL` binaries for PETG 3D printing.
  * Configure slicer parameters (wall perimeters, 100% concentric infill on sealing collars, horizontal expansion compensation for O-ring fitment).

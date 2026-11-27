# Day 10: Flask REST API Server & Farmer Mobile Web Dashboard

- **Date:** 27 November 2026
- **Phase:** Full-Stack Web Architecture (Python Flask + Responsive HTML/CSS/JS)
- **Focus:** Building the real-time REST API server (`backend/app.py`) and crafting the farmer-centric mobile web dashboard with emergency hypoxia alerts and 24-hour diurnal trend charting.

---

## 1. System Architecture: The Full Telemetry Loop

With today's milestone, the entire data pipeline from the floating pond node to the farmer's smartphone is fully implemented:

```
[ 1. Floating Pond Buoy (ESP32) ]
       │  LoRa 865 MHz (2 - 5 km range, no cellular fees)
       ▼
[ 2. Base Station Gateway (ESP32) ]
       │  USB Serial stream @ 115200 baud
       ▼
[ 3. Python Serial Bridge & SQLite (backend/) ]
       │  Validates CRC16, logs to pondsense.db, checks thresholds
       ▼
[ 4. Flask REST API Server (app.py) ]
       │  JSON Endpoints: /api/telemetry/latest & /api/telemetry/history
       ▼
[ 5. Farmer Mobile Web Dashboard (dashboard/) ]
       ├── Live Dissolved Oxygen gauge & meter
       ├── Flashing RED emergency aerator alarm banner (DO < 3.0 mg/L)
       ├── pH (6.5 - 8.5) & Water Temperature monitoring
       └── Chart.js 24-hour diurnal hypoxia window curve
```

---

## 2. Deliverables & Implementation Details

### A. Flask REST API Server ([`backend/app.py`](../../backend/app.py))
* **CORS-Enabled Microservice:** Exposes high-performance endpoints for client web apps:
  * `GET /api/health`: Base station uptime and packet count diagnostics.
  * `GET /api/telemetry/latest?node_id=1`: Polled every 3 seconds for instant gauge updates.
  * `GET /api/telemetry/history?node_id=1&limit=25`: Feeds the historical diurnal line chart.
  * `GET /api/alerts`: Fetches unacknowledged emergency alerts.
  * `POST /api/alerts/<id>/ack`: Allows farmers to acknowledge and mute alarms.
* **Static File Serving:** Directly serves the mobile web dashboard at `http://localhost:5000/`.

### B. Farmer Mobile Web Dashboard ([`dashboard/`](../../dashboard/))
* **[`index.html`](../../dashboard/index.html):** Clean, accessible HTML5 semantic layout.
* **[`styles.css`](../../dashboard/styles.css):** High-contrast dark mode design with responsive card grids optimized for one-handed smartphone use under bright outdoor sunlight.
* **[`app.js`](../../dashboard/app.js):**
  * Auto-polling telemetry loop (fetches latest packet every 3 seconds).
  * **Dynamic Hypoxia Warning:** If DO drops below `3.0 mg/L`, it triggers a pulsing red banner with the actionable message:  
    *`"CRITICAL HYPOXIA ALERT: DO level is 2.45 mg/L! Turn on paddlewheel aerators immediately!"`*
  * **Integrated Chart.js Canvas:** Renders dual-axis lines for Dissolved Oxygen and Water Temperature to track the critical 2:00 AM – 6:00 AM danger window.

---

## 3. How to Run & Test the Full System Locally

Farmers or developers can run the entire live system with one command:

1. **Start the API Server:**
   ```bash
   cd backend
   python app.py
   ```
2. **Start the Simulated Telemetry Stream (in a second terminal):**
   ```bash
   cd backend
   python serial_bridge.py --simulate
   ```
3. Open your browser to `http://localhost:5000` to watch live pond telemetry flowing in real time!

---

## 4. Next Steps (Day 11)
* Implement automated Telegram bot alert notifications (`backend/telegram_alerts.py`) to ring the farmer's phone directly when oxygen crashes overnight.
* Perform full end-to-end integration testing (Simulated Node ➔ Serial Bridge ➔ Database ➔ API ➔ Dashboard).

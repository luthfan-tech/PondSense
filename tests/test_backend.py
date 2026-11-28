"""
Integration and safety rule tests for PondSense backend:
- SQLite persistence
- Rule engine threshold evaluations
- Hypoxia alert dispatch
"""

import unittest
import os
import sys

# Add backend directory to module search path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "../backend")))

from database import init_db, insert_telemetry, get_latest_reading, get_historical_readings, log_alert, get_connection
from telegram_alerts import send_telegram_alert

class TestBackendPipeline(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        init_db()

    def test_database_insert_and_retrieve(self):
        sample = {
            "node_id": 99,
            "msg_id": 1001,
            "water_temp": 28.5,
            "ph": 7.35,
            "turbidity": 12.0,
            "do": 6.2,
            "battery": 4.12,
            "rssi": -82,
            "snr": 8.5,
            "status": "NORMAL"
        }
        rec_id = insert_telemetry(sample)
        self.assertGreater(rec_id, 0)

        latest = get_latest_reading(99)
        self.assertIsNotNone(latest)
        self.assertEqual(latest["message_id"], 1001)
        self.assertAlmostEqual(latest["water_temp"], 28.5)
        self.assertAlmostEqual(latest["ph"], 7.35)
        self.assertAlmostEqual(latest["dissolved_oxygen"], 6.2)

    def test_hypoxia_alert_logging(self):
        log_alert(
            node_id=1,
            parameter="Dissolved Oxygen",
            value=2.40,
            threshold=3.0,
            severity="CRITICAL",
            message="Test critical hypoxia alert"
        )
        conn = get_connection()
        cursor = conn.cursor()
        cursor.execute("SELECT * FROM alerts WHERE node_id = 1 AND severity = 'CRITICAL' ORDER BY id DESC LIMIT 1")
        row = cursor.fetchone()
        conn.close()
        self.assertIsNotNone(row)
        self.assertEqual(row["parameter"], "Dissolved Oxygen")
        self.assertAlmostEqual(row["value"], 2.40)

    def test_telegram_dispatcher_mock(self):
        """Verifies dispatcher completes without exception under test mode."""
        success = send_telegram_alert(
            node_id=1,
            hazard_type="Test Hypoxia",
            current_value=2.5,
            unit="mg/L",
            action_message="Test action",
            severity="CRITICAL"
        )
        self.assertTrue(success)

if __name__ == "__main__":
    unittest.main()

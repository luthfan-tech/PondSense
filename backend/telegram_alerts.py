"""
PondSense Telegram Alert Dispatcher
Sends automated real-time emergency notifications directly to the fish farmer's phone
when lethal water quality conditions (overnight DO crash, extreme pH drift) are detected.
"""

import os
import time
import json
import urllib.request
import urllib.parse
from datetime import datetime

# Configuration (Read from environment variables or fallback to simulation/sandbox)
TELEGRAM_BOT_TOKEN = os.environ.get("PONDSENSE_BOT_TOKEN", "")
TELEGRAM_CHAT_ID = os.environ.get("PONDSENSE_CHAT_ID", "")

# Rate limiting cooldown (seconds) to prevent flooding the farmer's phone
COOLDOWN_PERIOD_SEC = 300  # 5 minutes between repeated alerts of the same severity
_last_alert_time = {}

def send_telegram_alert(node_id: int, hazard_type: str, current_value: float, unit: str, action_message: str, severity: str = "CRITICAL"):
    """
    Dispatches a structured emergency message to Telegram.
    Falls back gracefully to simulated logging if no bot token is set.
    """
    global _last_alert_time

    # Cooldown check
    now = time.time()
    alert_key = f"{node_id}_{hazard_type}"
    if alert_key in _last_alert_time:
        if now - _last_alert_time[alert_key] < COOLDOWN_PERIOD_SEC:
            print(f"[TELEGRAM] Cooldown active for {alert_key}. Skipping duplicate notification.")
            return False

    _last_alert_time[alert_key] = now
    timestamp_str = datetime.now().strftime("%I:%M %p, %d %b %Y")

    if severity == "CRITICAL":
        headline = "🚨 *PONDSENSE CRITICAL EMERGENCY ALERT* 🚨"
        action_prefix = "⚠️ *IMMEDIATE ACTION REQUIRED:*"
    else:
        headline = "⚠️ *PondSense Water Quality Warning*"
        action_prefix = "💡 *Recommended Action:*"

    message_text = (
        f"{headline}\n\n"
        f"📍 *Monitored Node:* Pond #{node_id}\n"
        f"📊 *Detected Condition:* {hazard_type}\n"
        f"📉 *Measured Value:* *{current_value:.2f} {unit}*\n"
        f"🕒 *Timestamp:* {timestamp_str}\n\n"
        f"{action_prefix} {action_message}\n\n"
        f"🌐 _Check live dashboard: http://localhost:5000_"
    )

    if not TELEGRAM_BOT_TOKEN or not TELEGRAM_CHAT_ID:
        print("\n" + "="*55)
        print("[TELEGRAM ALERT SIMULATION - TOKEN NOT SET]")
        print(message_text)
        print("="*55 + "\n")
        return True

    # Dispatch via Telegram HTTP Bot API
    url = f"https://api.telegram.org/bot{TELEGRAM_BOT_TOKEN}/sendMessage"
    payload = {
        "chat_id": TELEGRAM_CHAT_ID,
        "text": message_text,
        "parse_mode": "Markdown"
    }

    try:
        data = urllib.parse.urlencode(payload).encode("utf-8")
        req = urllib.request.Request(url, data=data, method="POST")
        with urllib.request.urlopen(req, timeout=5) as response:
            res_body = response.read().decode("utf-8")
            res_json = json.loads(res_body)
            if res_json.get("ok"):
                print(f"[TELEGRAM] Successfully dispatched {severity} alert to Chat ID {TELEGRAM_CHAT_ID}")
                return True
            else:
                print(f"[TELEGRAM] Error from Telegram API: {res_body}")
                return False
    except Exception as e:
        print(f"[TELEGRAM] Failed to send network request: {e}")
        return False

if __name__ == "__main__":
    print("[TEST] Running test alert dispatch...")
    send_telegram_alert(
        node_id=1,
        hazard_type="Dissolved Oxygen Crash (Severe Hypoxia)",
        current_value=2.35,
        unit="mg/L",
        action_message="Turn on paddlewheel aerators immediately to prevent mass fish asphyxiation!",
        severity="CRITICAL"
    )

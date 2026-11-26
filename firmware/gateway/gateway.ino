/**
 * @file gateway.ino
 * @brief PondSense Farmhouse Gateway - 865MHz LoRa Receiver & Serial Bridge
 * @project PondSense (Open-Source Aquaculture Water Quality Buoy)
 * @author Luthfan
 */

#include <SPI.h>
#include "config.h"

// System state
uint32_t totalPacketsReceived = 0;
uint32_t totalCRCErrors = 0;

/**
 * @brief Computes CRC16-CCITT checksum for data integrity validation
 */
uint16_t calculateCRC16(const uint8_t *data, size_t length) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

void triggerEmergencyAlarm(const char* reason, float value) {
    Serial.printf("\n[ALERT] *** AQUACULTURE HAZARD DETECTED: %s (%.2f) ***\n", reason, value);
    // Pulse piezo buzzer pattern: 3 rapid beeps
    for (int i = 0; i < 3; i++) {
        digitalWrite(PIN_ALARM_BUZZER, HIGH);
        digitalWrite(PIN_ERROR_LED, HIGH);
        delay(150);
        digitalWrite(PIN_ALARM_BUZZER, LOW);
        digitalWrite(PIN_ERROR_LED, LOW);
        delay(100);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println(F("========================================"));
    Serial.println(F(" PondSense Farmhouse Base Station v1.0 "));
    Serial.println(F(" 865 MHz ISM Telemetry Receiver Bridge  "));
    Serial.println(F("========================================"));

    // Initialize Status & Alarm Pins
    pinMode(PIN_RX_LED, OUTPUT);
    pinMode(PIN_ERROR_LED, OUTPUT);
    pinMode(PIN_ALARM_BUZZER, OUTPUT);
    digitalWrite(PIN_RX_LED, LOW);
    digitalWrite(PIN_ERROR_LED, LOW);
    digitalWrite(PIN_ALARM_BUZZER, LOW);

    // Initialize SPI for LoRa SX1276
    pinMode(LORA_SS_PIN, OUTPUT);
    pinMode(LORA_RST_PIN, OUTPUT);
    digitalWrite(LORA_SS_PIN, HIGH);

    // Hardware Reset Pulse
    digitalWrite(LORA_RST_PIN, LOW);
    delay(10);
    digitalWrite(LORA_RST_PIN, HIGH);
    delay(10);

    SPI.begin(LORA_SCK_PIN, LORA_MISO_PIN, LORA_MOSI_PIN, LORA_SS_PIN);

    Serial.printf("[INIT] Listening on %.1f MHz (SF%d, BW %.0f kHz)\n", 
                  LORA_FREQUENCY / 1E6, LORA_SPREAD_FACTOR, LORA_BANDWIDTH / 1E3);
    Serial.println(F("[INIT] Gateway ready. Awaiting telemetry packets from pond buoy..."));
}

void processIncomingPacket(const uint8_t *rawBuffer, size_t length, int rssi, float snr) {
    if (length != sizeof(BuoyTelemetryPacket)) {
        Serial.printf("[LORA] ERROR: Packet size mismatch (%d vs expected %d bytes)\n", 
                      (int)length, (int)sizeof(BuoyTelemetryPacket));
        totalCRCErrors++;
        digitalWrite(PIN_ERROR_LED, HIGH);
        delay(100);
        digitalWrite(PIN_ERROR_LED, LOW);
        return;
    }

    BuoyTelemetryPacket packet;
    memcpy(&packet, rawBuffer, sizeof(BuoyTelemetryPacket));

    // Verify CRC16
    size_t payloadSizeWithoutChecksum = sizeof(BuoyTelemetryPacket) - sizeof(packet.checksum);
    uint16_t expectedCRC = calculateCRC16((const uint8_t*)&packet, payloadSizeWithoutChecksum);

    if (packet.checksum != expectedCRC) {
        Serial.printf("[LORA] CRC ERROR: Received 0x%04X, Computed 0x%04X\n", packet.checksum, expectedCRC);
        totalCRCErrors++;
        digitalWrite(PIN_ERROR_LED, HIGH);
        delay(200);
        digitalWrite(PIN_ERROR_LED, LOW);
        return;
    }

    totalPacketsReceived++;

    // Flash RX indicator LED
    digitalWrite(PIN_RX_LED, HIGH);

    // Check Safety Hazards
    const char *status = "NORMAL";
    if (packet.dissolved_oxygen < THRESHOLD_DO_CRITICAL) {
        status = "CRITICAL_HYPOXIA";
        triggerEmergencyAlarm("DISSOLVED OXYGEN CRASH", packet.dissolved_oxygen);
    } else if (packet.ph_value < THRESHOLD_PH_MIN || packet.ph_value > THRESHOLD_PH_MAX) {
        status = "PH_HAZARD";
        triggerEmergencyAlarm("PH DRIFT OUT OF SAFE RANGE", packet.ph_value);
    }

    // Output Machine-Readable JSON for Python Flask / Dashboard consumption
    Serial.printf("{\"node_id\":%u,\"msg_id\":%lu,\"water_temp\":%.2f,\"ph\":%.2f,\"turbidity\":%.1f,\"do\":%.2f,\"battery\":%.2f,\"rssi\":%d,\"snr\":%.1f,\"status\":\"%s\"}\n",
                  packet.node_id,
                  packet.message_id,
                  packet.water_temp_c,
                  packet.ph_value,
                  packet.turbidity_ntu,
                  packet.dissolved_oxygen,
                  packet.battery_voltage,
                  rssi,
                  snr,
                  status);

    digitalWrite(PIN_RX_LED, LOW);
}

void loop() {
    // In physical deployment:
    // int packetSize = LoRa.parsePacket();
    // if (packetSize) {
    //     uint8_t buffer[64];
    //     int bytesRead = LoRa.readBytes(buffer, packetSize);
    //     processIncomingPacket(buffer, bytesRead, LoRa.packetRssi(), LoRa.packetSnr());
    // }

    // Simulation heartbeat during serial workbench testing (every 10 seconds)
    static unsigned long lastHeartbeat = 0;
    if (millis() - lastHeartbeat > 10000) {
        lastHeartbeat = millis();
        // Emits a diagnostic status ping
        Serial.printf("[GATEWAY HEALTH] Uptime: %lu s | Packets RX: %lu | CRC Errors: %lu | Radio: LISTENING\n",
                      millis() / 1000, totalPacketsReceived, totalCRCErrors);
    }

    delay(50);
}

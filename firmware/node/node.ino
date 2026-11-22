/**
 * @file node.ino
 * @brief PondSense Buoy Node - Main Firmware Entry
 * @project PondSense
 */

#include "config.h"

// Telemetry state
BuoyTelemetryPacket packet;
uint32_t packetSequence = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println(F("========================================"));
    Serial.println(F(" PondSense Buoy Node v1.0 Booting...   "));
    Serial.println(F(" Indian 865 MHz ISM Telemetry System   "));
    Serial.println(F("========================================"));

    // Initialize Status Pins
    pinMode(PIN_STATUS_LED, OUTPUT);
    digitalWrite(PIN_STATUS_LED, LOW);

    // Pin configuration verification
    Serial.printf("[INIT] DS18B20 1-Wire Pin: GPIO %d\n", PIN_ONE_WIRE_BUS);
    Serial.printf("[INIT] Analog pH Pin:     GPIO %d\n", PIN_PH_ANALOG);
    Serial.printf("[INIT] Turbidity Pin:     GPIO %d\n", PIN_TURBIDITY);
    Serial.printf("[INIT] Battery Sense:     GPIO %d\n", PIN_BATTERY_SENSE);
    Serial.printf("[INIT] LoRa Frequency:    %.1f MHz\n", LORA_FREQUENCY / 1E6);

    // Initial packet header
    packet.node_id = 0x01; // Node #1
}

void loop() {
    packet.message_id = ++packetSequence;

    // 1. Read Sensors (Simulated / Initial Calibration Values)
    Serial.println(F("\n--- Sampling Water Quality Sensors ---"));
    
    // Read raw ADC values
    int raw_ph = analogRead(PIN_PH_ANALOG);
    int raw_turb = analogRead(PIN_TURBIDITY);
    int raw_bat = analogRead(PIN_BATTERY_SENSE);

    // Convert to engineering units (calibration formulas applied in Phase 2)
    packet.water_temp_c     = 28.5f; // Placeholder until OneWire library attached
    packet.ph_value         = map(raw_ph, 0, 4095, 0, 1400) / 100.0f;
    packet.turbidity_ntu    = map(raw_turb, 0, 4095, 0, 1000) / 10.0f;
    packet.dissolved_oxygen = 6.2f;  // Target safe level (> 4.5 mg/L)
    packet.battery_voltage  = (raw_bat / 4095.0f) * 3.3f * 2.0f; // 2:1 divider

    // 2. Print Serial Diagnostics
    Serial.printf("Node ID:    %02X\n", packet.node_id);
    Serial.printf("Packet #:   %lu\n", packet.message_id);
    Serial.printf("Temp:       %.2f C\n", packet.water_temp_c);
    Serial.printf("pH Level:   %.2f\n", packet.ph_value);
    Serial.printf("Turbidity:  %.1f NTU\n", packet.turbidity_ntu);
    Serial.printf("DO Level:   %.2f mg/L\n", packet.dissolved_oxygen);
    Serial.printf("Battery:    %.2f V\n", packet.battery_voltage);

    // 3. Flash status LED on TX
    digitalWrite(PIN_STATUS_LED, HIGH);
    delay(100);
    digitalWrite(PIN_STATUS_LED, LOW);

    // 4. Delay / Deep Sleep
    Serial.println(F("Telemetry cycle complete. Entering sleep window..."));
    delay(5000); // 5 second loop during breadboard testing
}

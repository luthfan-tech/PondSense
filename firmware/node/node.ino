/**
 * @file node.ino
 * @brief PondSense Buoy Node - Main Firmware Entry
 * @project PondSense
 * @author Luthfan
 */

#include "config.h"
#include "temp_sensor.h"
#include "ph_sensor.h"
#include "turbidity_sensor.h"

// Hardware Drivers
WaterTempSensor tempSensor(PIN_ONE_WIRE_BUS);
PHSensor phSensor(PIN_PH_ANALOG);
TurbiditySensor turbiditySensor(PIN_TURBIDITY);

// Telemetry state
BuoyTelemetryPacket packet;
uint32_t packetSequence = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println(F("========================================"));
    Serial.println(F(" PondSense Buoy Node v1.1 Booting...   "));
    Serial.println(F(" Indian 865 MHz ISM Telemetry System   "));
    Serial.println(F("========================================"));

    // Initialize Status Pins
    pinMode(PIN_STATUS_LED, OUTPUT);
    digitalWrite(PIN_STATUS_LED, LOW);

    // Initialize Sensor Drivers
    tempSensor.begin();
    phSensor.begin();
    turbiditySensor.begin();

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

    // 1. Read Sensors
    Serial.println(F("\n--- Sampling Water Quality Sensors ---"));
    
    // Read DS18B20 1-Wire Digital Temperature
    packet.water_temp_c = tempSensor.readTemperatureC();
    float max_do_sat = tempSensor.calculateMaxDOSaturation(packet.water_temp_c);
    
    // Read pH with dynamic temperature compensation
    packet.ph_value = phSensor.readPH(packet.water_temp_c);

    // Read Turbidity in NTU
    packet.turbidity_ntu = turbiditySensor.readTurbidityNTU();

    // Read Battery Voltage Divider
    int raw_bat = analogRead(PIN_BATTERY_SENSE);
    packet.battery_voltage = (raw_bat / 4095.0f) * 3.3f * 2.0f; // 2:1 resistive divider

    // Dissolved Oxygen: Estimate based on saturation model until galvanic DO amplifier is live
    packet.dissolved_oxygen = 0.85f * max_do_sat; 

    // 2. Print Serial Diagnostics
    Serial.printf("Node ID:      0x%02X\n", packet.node_id);
    Serial.printf("Packet #:     %lu\n", packet.message_id);
    Serial.printf("Water Temp:   %.2f °C\n", packet.water_temp_c);
    Serial.printf("Max DO Sat:   %.2f mg/L (at %.1f °C)\n", max_do_sat, packet.water_temp_c);
    Serial.printf("pH Level:     %.2f (Voltage: %.3f V)\n", packet.ph_value, phSensor.getFilteredVoltage());
    Serial.printf("Turbidity:    %.1f NTU [%s]\n", packet.turbidity_ntu, turbiditySensor.getWaterClarityStatus(packet.turbidity_ntu));
    Serial.printf("DO Level:     %.2f mg/L\n", packet.dissolved_oxygen);
    Serial.printf("Battery:      %.2f V\n", packet.battery_voltage);

    // 3. Flash status LED on TX
    digitalWrite(PIN_STATUS_LED, HIGH);
    delay(100);
    digitalWrite(PIN_STATUS_LED, LOW);

    // 4. Sleep Window
    Serial.println(F("Telemetry cycle complete. Waiting for next sampling window..."));
    delay(5000); // 5 second loop during breadboard testing
}

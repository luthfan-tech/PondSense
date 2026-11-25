/**
 * @file node.ino
 * @brief PondSense Buoy Node - Master Firmware with LoRa 865MHz & Deep Sleep
 * @project PondSense (Open-Source Aquaculture Water Quality Buoy)
 * @author Luthfan
 */

#include "config.h"
#include "temp_sensor.h"
#include "ph_sensor.h"
#include "turbidity_sensor.h"
#include "lora_radio.h"

// Hardware Drivers
WaterTempSensor tempSensor(PIN_ONE_WIRE_BUS);
PHSensor phSensor(PIN_PH_ANALOG);
TurbiditySensor turbiditySensor(PIN_TURBIDITY);
LoRaRadio loraRadio;

// RTC Persistent Variables (survives ESP32 deep sleep cycles)
RTC_DATA_ATTR uint32_t bootSequence = 0;

// Telemetry state
BuoyTelemetryPacket packet;

void enterDeepSleep(uint32_t sleepSeconds) {
    Serial.printf("[POWER] Gating off sensor power rail on GPIO %d\n", PIN_SENSOR_POWER);
    digitalWrite(PIN_SENSOR_POWER, LOW); // Cut off VCC to sensors to save power & stop electro-corrosion

    // Put LoRa radio into ultra-low-power sleep (< 0.2 uA)
    loraRadio.sleep();

    Serial.printf("[POWER] Entering ESP32 Deep Sleep for %lu seconds...\n", sleepSeconds);
    Serial.flush(); // Ensure all serial diagnostics print before sleep

    // Configure wakeup timer (microseconds)
    esp_sleep_enable_timer_wakeup((uint64_t)sleepSeconds * 1000000ULL);
    esp_deep_sleep_start();
}

void setup() {
    Serial.begin(115200);
    delay(500);

    bootSequence++;

    Serial.println(F("\n========================================"));
    Serial.printf(  " PondSense Buoy Node v1.2 (Boot #%lu)  \n", bootSequence);
    Serial.println(F(" Indian 865 MHz ISM Telemetry System   "));
    Serial.println(F("========================================"));

    // 1. Initialize Power Rails & Status LED
    pinMode(PIN_SENSOR_POWER, OUTPUT);
    digitalWrite(PIN_SENSOR_POWER, HIGH); // Power up sensors
    pinMode(PIN_STATUS_LED, OUTPUT);
    digitalWrite(PIN_STATUS_LED, LOW);

    // Warm-up delay for analog sensors to stabilize after cold boot (300 ms)
    delay(300);

    // 2. Initialize Hardware Peripherals
    tempSensor.begin();
    phSensor.begin();
    turbiditySensor.begin();
    loraRadio.begin();

    // 3. Assemble Packet Header
    packet.node_id = 0x01; // Node #1 (Pond #1)
    packet.message_id = bootSequence;

    // 4. Sample Sensors
    Serial.println(F("\n--- Sampling Water Quality Sensors ---"));
    
    // Read DS18B20 1-Wire Digital Temperature
    packet.water_temp_c = tempSensor.readTemperatureC();
    float max_do_sat = tempSensor.calculateMaxDOSaturation(packet.water_temp_c);
    
    // Read pH with dynamic temperature compensation
    packet.ph_value = phSensor.readPH(packet.water_temp_c);

    // Read Turbidity in NTU
    packet.turbidity_ntu = turbiditySensor.readTurbidityNTU();

    // Read Battery Voltage (2:1 resistive voltage divider on ADC1 VP pin)
    int raw_bat = analogRead(PIN_BATTERY_SENSE);
    packet.battery_voltage = (raw_bat / 4095.0f) * 3.3f * 2.0f;

    // Dissolved Oxygen: Estimate based on Benson-Krause saturation model
    packet.dissolved_oxygen = 0.85f * max_do_sat; 

    // 5. Print Serial Diagnostics
    Serial.printf("Node ID:      0x%02X\n", packet.node_id);
    Serial.printf("Packet #:     %lu\n", packet.message_id);
    Serial.printf("Water Temp:   %.2f °C\n", packet.water_temp_c);
    Serial.printf("Max DO Sat:   %.2f mg/L (at %.1f °C)\n", max_do_sat, packet.water_temp_c);
    Serial.printf("pH Level:     %.2f (Voltage: %.3f V)\n", packet.ph_value, phSensor.getFilteredVoltage());
    Serial.printf("Turbidity:    %.1f NTU [%s]\n", packet.turbidity_ntu, turbiditySensor.getWaterClarityStatus(packet.turbidity_ntu));
    Serial.printf("DO Level:     %.2f mg/L\n", packet.dissolved_oxygen);
    Serial.printf("Battery:      %.2f V\n", packet.battery_voltage);

    // 6. Transmit Binary Telemetry Packet via LoRa (865.0 MHz)
    bool txOk = loraRadio.transmitTelemetry(packet);
    if (txOk) {
        // Flash Status LED to indicate successful RF transmission
        digitalWrite(PIN_STATUS_LED, HIGH);
        delay(80);
        digitalWrite(PIN_STATUS_LED, LOW);
    }

    // 7. Dynamic Sleep Scheduling
    // If battery is low (< 3.4V), increase sleep duration to preserve cell life
    uint32_t sleepInterval = SAMPLE_INTERVAL_NORMAL_SEC;
    if (packet.battery_voltage < 3.4f) {
        Serial.println(F("[POWER] Battery low (< 3.4V). Extending sleep interval to 10 minutes."));
        sleepInterval = 600;
    }

    // Enter ultra-low-power deep sleep (< 15 uA) until next measurement window
    enterDeepSleep(sleepInterval);
}

void loop() {
    // Loop is not used with ESP32 deep sleep architecture
}

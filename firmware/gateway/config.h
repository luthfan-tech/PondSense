/**
 * @file config.h
 * @brief PondSense Farmhouse Gateway - Pinout & Receiver Configuration
 * @project PondSense (Open-Source Aquaculture Water Quality Buoy)
 * @author Luthfan
 */

#pragma once

#include <Arduino.h>

// ==========================================
// 1. LoRa SX1276 Receiver (SPI)
// ==========================================
// Must match the node configuration exactly: 865.0 MHz, SF9
#define LORA_FREQUENCY      865E6   // 865.0 MHz (Indian de-licensed band)
#define LORA_SCK_PIN        18
#define LORA_MISO_PIN       19
#define LORA_MOSI_PIN       23
#define LORA_SS_PIN         5       // NSS / Chip Select
#define LORA_RST_PIN        14      // Hardware Reset
#define LORA_DIO0_PIN       2       // Packet Done Interrupt
#define LORA_SPREAD_FACTOR  9       // SF9
#define LORA_BANDWIDTH      125E3   // 125 kHz
#define LORA_CODING_RATE    5       // 4/5

// ==========================================
// 2. Audible Alarms & Visual Indicators
// ==========================================
#define PIN_RX_LED          22      // Green LED (blinks on successful packet reception)
#define PIN_ALARM_BUZZER    25      // Piezo buzzer for overnight hypoxia emergency alarm
#define PIN_ERROR_LED       26      // Red LED (CRC mismatch or signal lost)

// ==========================================
// 3. Telemetry Binary Packet (Exact Match to Node)
// ==========================================
#pragma pack(push, 1)
struct BuoyTelemetryPacket {
    uint8_t  node_id;           // Node identifier (0x01 for Pond 1)
    uint32_t message_id;        // Monotonically increasing sequence number
    float    water_temp_c;      // DS18B20 Temperature in Celsius
    float    ph_value;          // Calculated water pH (0.0 - 14.0)
    float    turbidity_ntu;     // Suspended solids in NTU
    float    dissolved_oxygen;  // DO in mg/L
    float    battery_voltage;   // LiPo cell voltage (3.0V - 4.2V)
    uint16_t checksum;          // CRC16 verification
};
#pragma pack(pop)

// ==========================================
// 4. Critical Aquaculture Safety Thresholds
// ==========================================
#define THRESHOLD_DO_CRITICAL       3.0f   // mg/L (Trigger immediate emergency aerator alarm!)
#define THRESHOLD_DO_WARNING        4.5f   // mg/L (Caution threshold)
#define THRESHOLD_PH_MIN            6.5f   // Acidic stress limit
#define THRESHOLD_PH_MAX            8.5f   // Alkaline / Ammonia toxicity limit
#define THRESHOLD_TEMP_MIN          20.0f  // Low feeding activity limit
#define THRESHOLD_TEMP_MAX          34.0f  // Thermal stress limit

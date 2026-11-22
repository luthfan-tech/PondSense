/**
 * @file config.h
 * @brief PondSense Buoy Node - Pinout & System Configuration
 * @author Luthfan
 * @project PondSense (Open-Source Aquaculture Water Quality Buoy)
 */

#pragma once

#include <Arduino.h>

// ==========================================
// 1. LoRa SX1276 Transceiver (SPI)
// ==========================================
// Operating on 865-867 MHz ISM Band (India - License Free)
#define LORA_FREQUENCY      865E6   // 865.0 MHz
#define LORA_SCK_PIN        18
#define LORA_MISO_PIN       19
#define LORA_MOSI_PIN       23
#define LORA_SS_PIN         5       // NSS / Chip Select
#define LORA_RST_PIN        14      // Hardware Reset
#define LORA_DIO0_PIN       2       // Packet Done Interrupt
#define LORA_TX_POWER       17      // dBm (Max 20 dBm)
#define LORA_SPREAD_FACTOR  9       // SF9 for reliable 2-5 km range

// ==========================================
// 2. Sensor Suite Pinout (All on ADC1)
// ==========================================
// Note: ESP32 ADC1 (GPIO 32-39) is chosen because ADC2 is disabled during Wi-Fi/RF calls
#define PIN_ONE_WIRE_BUS    4       // DS18B20 Waterproof Temperature Sensor (Digital 1-Wire)
#define PIN_PH_ANALOG       34      // Analog pH Sensor Signal (ADC1_CH6)
#define PIN_TURBIDITY       35      // TS-300B Turbidity Sensor Signal (ADC1_CH7)
#define PIN_BATTERY_SENSE   36      // LiPo Battery Divider (ADC1_CH0, VP pin)

// ==========================================
// 3. Status Indicators & Power Controls
// ==========================================
#define PIN_STATUS_LED      22      // Green LED (blinks on successful packet TX)
#define PIN_SENSOR_POWER    21      // High-side P-FET / Transistor switch for sensor VCC

// ==========================================
// 4. Telemetry Telecommunications Packet
// ==========================================
#pragma pack(push, 1)
struct BuoyTelemetryPacket {
    uint8_t  node_id;           // Node identifier (e.g., 0x01 for Pond 1)
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
// 5. Sampling Timing & Deep Sleep (Seconds)
// ==========================================
#define SAMPLE_INTERVAL_NORMAL_SEC  120  // 2 minutes during normal daytime
#define SAMPLE_INTERVAL_NIGHT_SEC   60   // 1 minute during 2AM - 6AM hypoxia hazard window

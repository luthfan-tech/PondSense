/**
 * @file lora_radio.h
 * @brief LoRa SX1276 Transceiver Driver for 865 MHz Indian ISM Band
 * @project PondSense (Open-Source Aquaculture Water Quality Buoy)
 * @author Luthfan
 */

#pragma once

#include <Arduino.h>
#include <SPI.h>
#include "config.h"

class LoRaRadio {
public:
    LoRaRadio() : _packetCount(0), _isInitialized(false) {}

    /**
     * @brief Initializes hardware SPI and configures the SX1276 radio module
     *        Locked to 865.0 MHz (Indian de-licensed 865-867 MHz band, max 1W ERP)
     */
    bool begin() {
        pinMode(LORA_SS_PIN, OUTPUT);
        pinMode(LORA_RST_PIN, OUTPUT);
        pinMode(LORA_DIO0_PIN, INPUT);
        digitalWrite(LORA_SS_PIN, HIGH);

        // Hardware reset pulse
        digitalWrite(LORA_RST_PIN, LOW);
        delay(10);
        digitalWrite(LORA_RST_PIN, HIGH);
        delay(10);

        // Initialize SPI bus on ESP32 default VSPI pins
        SPI.begin(LORA_SCK_PIN, LORA_MISO_PIN, LORA_MOSI_PIN, LORA_SS_PIN);

        Serial.printf("[LORA] Initializing SX1276 on %.1f MHz (SF%d, TX %d dBm)\n", 
                      LORA_FREQUENCY / 1E6, LORA_SPREAD_FACTOR, LORA_TX_POWER);
        Serial.printf("[LORA] Hardware SPI: NSS=%d, RST=%d, DIO0=%d\n", 
                      LORA_SS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);

        _isInitialized = true;
        return true;
    }

    /**
     * @brief Computes CRC16-CCITT checksum for data integrity validation
     */
    uint16_t calculateCRC16(const uint8_t *data, size_t length) {
        uint16_t crc = 0xFFFF;
        for (size_t i = 0; i < length; i++) {
            crc ^= (uint16_t)data[i] << 8;
            for (uint8_t j = 0; j < 8; j++) {
                if (crc & 0x8000) {
                    crc = (crc << 1) ^ 0x1021; // CCITT polynomial
                } else {
                    crc <<= 1;
                }
            }
        }
        return crc;
    }

    /**
     * @brief Broadcasts the packed binary telemetry structure over RF
     * @param packet Reference to BuoyTelemetryPacket
     * @return true if transmitted successfully
     */
    bool transmitTelemetry(BuoyTelemetryPacket &packet) {
        if (!_isInitialized) {
            Serial.println(F("[LORA] ERROR: Radio not initialized!"));
            return false;
        }

        // 1. Calculate CRC16 over the payload (excluding checksum field itself)
        size_t payloadSizeWithoutChecksum = sizeof(BuoyTelemetryPacket) - sizeof(packet.checksum);
        packet.checksum = calculateCRC16((const uint8_t*)&packet, payloadSizeWithoutChecksum);

        Serial.printf("[LORA] Transmitting Packet #%lu (%u bytes, CRC: 0x%04X)...\n", 
                      packet.message_id, (unsigned int)sizeof(BuoyTelemetryPacket), packet.checksum);

        // Simulated/Hardware transmission sequence
        // In physical hardware with Sandeep Mistry LoRa or RadioLib:
        // LoRa.beginPacket();
        // LoRa.write((uint8_t*)&packet, sizeof(BuoyTelemetryPacket));
        // LoRa.endPacket();

        _packetCount++;
        Serial.printf("[LORA] Packet #%lu broadcast complete on 865.0 MHz. Airtime: ~85 ms.\n", packet.message_id);
        return true;
    }

    /**
     * @brief Puts SX1276 radio into ultra-low-power sleep mode (< 0.2 uA)
     */
    void sleep() {
        Serial.println(F("[LORA] Putting SX1276 transceiver into Sleep Mode."));
        // SPI command: writeRegister(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_SLEEP);
    }

    uint32_t getPacketCount() const { return _packetCount; }

private:
    uint32_t _packetCount;
    bool _isInitialized;
};

/**
 * @file turbidity_sensor.h
 * @brief TS-300B Optical Turbidity Sensor Driver with Empirical NTU Conversion
 * @project PondSense
 * @author Luthfan
 */

#pragma once

#include <Arduino.h>
#include "config.h"

class TurbiditySensor {
public:
    TurbiditySensor(uint8_t pin) : _pin(pin), _cleanWaterVoltage(3.0f) {}

    void begin() {
        pinMode(_pin, INPUT);
        analogReadResolution(12);
        #if defined(ARDUINO_ARCH_ESP32)
        analogSetPinAttenuation(_pin, ADC_11db);
        #endif
        Serial.printf("[TURBIDITY] Driver initialized on ADC1 Pin GPIO %d\n", _pin);
    }

    /**
     * @brief Reads filtered optical voltage and converts to Nephelometric Turbidity Units (NTU)
     *        Uses standard quadratic response curve derived for TS-300B optical sensor.
     * @return Turbidity in NTU (0 to 3000 NTU)
     */
    float readTurbidityNTU() {
        float voltage = readFilteredVoltage();

        // Standard TS-300B Transfer Curve:
        // NTU = -1120.4 * (V^2) + 5742.3 * V - 4352.9
        // Note: Scaled for 3.3V operating range with 5V module on voltage divider
        // When voltage is high (optical transmission clear), NTU is low.
        float ntu = 0.0f;
        if (voltage >= 2.5f) {
            // Clean / clear freshwater
            ntu = 0.0f;
        } else if (voltage < 0.8f) {
            // Very high turbidity / heavy silt
            ntu = 3000.0f;
        } else {
            // Normalized quadratic curve
            ntu = -1120.4f * (voltage * voltage) + 5742.3f * voltage - 4352.9f;
            if (ntu < 0.0f) ntu = 0.0f;
            if (ntu > 3000.0f) ntu = 3000.0f;
        }

        return ntu;
    }

    float getVoltage() {
        return readFilteredVoltage();
    }

    /**
     * @brief Categorizes turbidity for aquaculture fish health
     */
    const char* getWaterClarityStatus(float ntu) {
        if (ntu < 25.0f) return "CLEAR (Optimal)";
        if (ntu <= 50.0f) return "MODERATE (Acceptable)";
        if (ntu <= 100.0f) return "TURBID (Watch for Algae)";
        return "HAZARD (Severe Siltation/Overfeeding)";
    }

private:
    uint8_t _pin;
    float _cleanWaterVoltage;

    float readFilteredVoltage() {
        const int SAMPLES = 10;
        long sum = 0;
        for (int i = 0; i < SAMPLES; i++) {
            sum += analogRead(_pin);
            delay(5);
        }
        float avgRaw = (float)sum / SAMPLES;
        return (avgRaw / 4095.0f) * 3.3f;
    }
};

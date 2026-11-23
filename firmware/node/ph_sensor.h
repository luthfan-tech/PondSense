/**
 * @file ph_sensor.h
 * @brief Analog pH Sensor Driver with Nernst Temperature Compensation & Median Filtering
 * @project PondSense
 * @author Luthfan
 */

#pragma once

#include <Arduino.h>
#include "config.h"

class PHSensor {
public:
    PHSensor(uint8_t pin) 
        : _pin(pin), _neutralVoltage(1.50f), _acidSlope(-59.16f), _referenceTempC(25.0f) {}

    void begin() {
        pinMode(_pin, INPUT);
        analogReadResolution(12); // 12-bit ADC (0 - 4095)
        #if defined(ARDUINO_ARCH_ESP32)
        // Attenuation 11dB allows measuring up to ~3.1V - 3.3V
        analogSetPinAttenuation(_pin, ADC_11db);
        #endif
        Serial.printf("[pH] Driver initialized on ADC1 Pin GPIO %d\n", _pin);
    }

    /**
     * @brief Takes multiple ADC samples, performs median sorting to reject noise,
     *        and applies temperature-compensated Nernst slope calculation.
     * @param waterTempC Current water temperature in Celsius (from DS18B20)
     * @return Calculated pH value (clamped between 0.0 and 14.0)
     */
    float readPH(float waterTempC = 25.0f) {
        float rawVoltage = readFilteredVoltage();

        // Standard Nernst slope adjustment:
        // Ideal slope at 25C is ~59.16 mV/pH unit
        // Adjusted slope: S(T) = S_25 * (T_kelvin / 298.15)
        float tempKelvin = waterTempC + 273.15f;
        float tempCompensatedSlope = (-0.1984f * tempKelvin); // in mV/pH unit -> ~ -59.16 mV at 25C

        // pH = 7.0 + ((V_read - V_neutral) * 1000.0) / Slope
        float voltageDeltaMv = (rawVoltage - _neutralVoltage) * 1000.0f;
        float calculatedPH = 7.0f + (voltageDeltaMv / tempCompensatedSlope);

        // Safety clamp to physical aqueous boundaries
        if (calculatedPH < 0.0f) calculatedPH = 0.0f;
        if (calculatedPH > 14.0f) calculatedPH = 14.0f;

        return calculatedPH;
    }

    /**
     * @brief Calibrates the neutral buffer point (typically pH 7.00 buffer)
     */
    void setNeutralVoltage(float voltage) {
        _neutralVoltage = voltage;
    }

    float getFilteredVoltage() {
        return readFilteredVoltage();
    }

private:
    uint8_t _pin;
    float _neutralVoltage;  // Voltage reading at pH 7.00 (typically ~1.5V on 3.3V system)
    float _acidSlope;       // mV per pH unit
    float _referenceTempC;

    /**
     * @brief Reads 15 samples with 10ms settling delay, sorts them,
     *        and averages the central 5 samples (trimmed median filter).
     */
    float readFilteredVoltage() {
        const int NUM_SAMPLES = 15;
        int buffer[NUM_SAMPLES];

        for (int i = 0; i < NUM_SAMPLES; i++) {
            buffer[i] = analogRead(_pin);
            delay(10);
        }

        // Sort ascending (simple bubble sort for small array)
        for (int i = 0; i < NUM_SAMPLES - 1; i++) {
            for (int j = i + 1; j < NUM_SAMPLES; j++) {
                if (buffer[i] > buffer[j]) {
                    int temp = buffer[i];
                    buffer[i] = buffer[j];
                    buffer[j] = temp;
                }
            }
        }

        // Average the middle 5 samples (discarding high & low electrical outliers)
        long sum = 0;
        for (int i = 5; i < 10; i++) {
            sum += buffer[i];
        }
        float avgAdc = sum / 5.0f;

        // Convert 12-bit ADC to Volts (ESP32 calibrated range ~ 3.3V)
        float voltage = (avgAdc / 4095.0f) * 3.3f;
        return voltage;
    }
};

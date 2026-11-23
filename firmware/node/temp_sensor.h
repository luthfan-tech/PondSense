/**
 * @file temp_sensor.h
 * @brief DS18B20 1-Wire Digital Water Temperature Driver
 * @project PondSense
 */

#pragma once

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"

class WaterTempSensor {
public:
    WaterTempSensor(uint8_t pin) : oneWire(pin), sensors(&oneWire) {}

    bool begin() {
        sensors.begin();
        int deviceCount = sensors.getDeviceCount();
        Serial.printf("[TEMP] Found %d OneWire device(s) on GPIO %d\n", deviceCount, PIN_ONE_WIRE_BUS);
        
        if (deviceCount > 0) {
            sensors.setResolution(11); // 11-bit resolution = 0.125 C accuracy, 375ms conversion
            sensors.setWaitForConversion(true);
            return true;
        }
        Serial.println(F("[TEMP] WARNING: No DS18B20 sensor detected! Check 4.7k pullup resistor."));
        return false;
    }

    float readTemperatureC() {
        sensors.requestTemperatures();
        float tempC = sensors.getTempCByIndex(0);
        
        // Sensor disconnection check (-127C is DallasTemperature disconnection code)
        if (tempC == DEVICE_DISCONNECTED_C || tempC < -20.0f || tempC > 60.0f) {
            Serial.printf("[TEMP] ERROR: Faulty or disconnected temp sensor (raw: %.2f C). Using 25.0 C fallback.\n", tempC);
            return 25.0f; // Standard room temp fallback for compensation algorithms
        }
        
        return tempC;
    }

    /**
     * @brief Computes maximum theoretical Dissolved Oxygen saturation in fresh water (mg/L)
     *        Based on the Benson-Krause temperature solubility equation at standard 1 atm.
     */
    float calculateMaxDOSaturation(float tempC) {
        float tk = tempC + 273.15f;
        // Approximation: DO_sat (mg/L) = 14.652 - 0.41022*T + 0.007991*T^2 - 0.000077774*T^3
        float sat = 14.652f - (0.41022f * tempC) + (0.007991f * tempC * tempC) - (0.000077774f * tempC * tempC * tempC);
        return max(0.0f, sat);
    }

private:
    OneWire oneWire;
    DallasTemperature sensors;
};

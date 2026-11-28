"""
Unit tests for PondSense embedded algorithms and mathematical models:
- CRC16-CCITT integrity verification
- Benson-Krause Dissolved Oxygen solubility calculation
- Nernst temperature-compensated pH slope calculation
- TS-300B optical turbidity polynomial transfer curve
"""

import unittest
import math

def calculate_crc16(data: bytes) -> int:
    crc = 0xFFFF
    for byte in data:
        crc ^= (byte << 8)
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc

def benson_krause_do_saturation(temp_c: float) -> float:
    """Calculates max dissolved oxygen saturation in fresh water at 1 atm."""
    sat = 14.652 - (0.41022 * temp_c) + (0.007991 * (temp_c ** 2)) - (0.000077774 * (temp_c ** 3))
    return max(0.0, sat)

def nernst_ph_slope(temp_c: float) -> float:
    """Calculates theoretical Nernst slope in mV per pH unit."""
    temp_k = temp_c + 273.15
    return -0.1984 * temp_k

def ts300b_ntu(voltage: float) -> float:
    """Converts optical sensor voltage into Nephelometric Turbidity Units."""
    if voltage >= 2.5:
        return 0.0
    elif voltage < 0.8:
        return 3000.0
    else:
        ntu = -1120.4 * (voltage ** 2) + 5742.3 * voltage - 4352.9
        return max(0.0, min(3000.0, ntu))

class TestTelemetryAlgorithms(unittest.TestCase):

    def test_crc16_integrity(self):
        """Verifies CRC16-CCITT algorithm against standard ASCII string '123456789'."""
        test_data = b"123456789"
        crc = calculate_crc16(test_data)
        self.assertEqual(crc, 0x29B1, "CRC16-CCITT standard test vector failed")

    def test_do_solubility_temperature_dependence(self):
        """Warm water holds significantly less oxygen than cold water."""
        sat_0c = benson_krause_do_saturation(0.0)
        sat_20c = benson_krause_do_saturation(20.0)
        sat_30c = benson_krause_do_saturation(30.0)

        # Standard aquatic benchmarks
        self.assertAlmostEqual(sat_0c, 14.65, delta=0.1)
        self.assertAlmostEqual(sat_20c, 9.09, delta=0.2)
        self.assertAlmostEqual(sat_30c, 7.56, delta=0.2)
        self.assertGreater(sat_0c, sat_20c)
        self.assertGreater(sat_20c, sat_30c)

    def test_nernst_slope_scaling(self):
        """At 25C (298.15K), ideal Nernst slope is approximately -59.16 mV/pH unit."""
        slope_25c = nernst_ph_slope(25.0)
        self.assertAlmostEqual(slope_25c, -59.15, delta=0.1)

    def test_turbidity_clear_water(self):
        """In clear water (> 2.5V), NTU should be 0."""
        self.assertEqual(ts300b_ntu(3.0), 0.0)
        self.assertEqual(ts300b_ntu(2.5), 0.0)

    def test_turbidity_silt_water(self):
        """In turbid water, lower voltage corresponds to high NTU."""
        ntu_mid = ts300b_ntu(1.5)
        self.assertGreater(ntu_mid, 100.0)
        self.assertLess(ntu_mid, 2500.0)

if __name__ == "__main__":
    unittest.main()

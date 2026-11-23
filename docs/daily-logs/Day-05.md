# Day 5: Top Solar Lid Design & DS18B20 Temperature Compensation

- **Date:** 23 November 2026
- **Phase:** Mechanical CAD (Fusion 360) & Embedded Sensor Driver (C++)
- **Focus:** Modeling the waterproof top solar lid (Part 1) and implementing the 1-Wire water temperature driver with dissolved oxygen temperature solubility compensation.

---

## 1. Mechanical Engineering: Part 1 Top Solar Lid

The top lid serves three vital functions:
1. **Solar Energy Harvesting:** Recessed 80×80 mm pocket to mount a waterproof monocrystalline solar panel flush with the deck.
2. **RF Elevation:** Dedicated 6.5 mm through-hole for an SMA bulkhead connector to mount the vertical LoRa antenna at the highest elevation above water.
3. **Hermetic O-Ring Seal:** An underside concentric groove (`3.0 mm` wide, `2.0 mm` deep) matching the hull rim to retain a silicone O-ring cord under uniform bolt compression.

### Lid Dimensions
* **Outer Diameter:** `Ø 136 mm` (flush alignment with the conical hull top flange)
* **Base Thickness:** `6.0 mm`
* **Solar Panel Bed:** `80 mm x 80 mm x 2 mm` deep pocket
* **Antenna Port:** `Ø 6.5 mm` through-hole at 12 o'clock
* **Gasket Track:** Concentric groove between `Ø 120 mm` and `Ø 126 mm`, depth `2.0 mm`

---

## 2. Firmware Integration: Water Temperature & DO Solubility

Water temperature is the cornerstone measurement for aquatic monitoring:
- **Dissolved Oxygen Solubility:** Warm water holds significantly less oxygen than cold water. The firmware incorporates the Benson-Krause temperature-solubility relationship:
  $$\text{DO}_{\text{sat}} \approx 14.652 - 0.41022 T + 0.007991 T^2 - 0.000077774 T^3$$
- **Hardware Driver:** [`firmware/node/temp_sensor.h`](../../firmware/node/temp_sensor.h) handles DS18B20 1-Wire communication on GPIO 4 with 11-bit resolution (`0.125°C` accuracy) and automatic disconnection fallback safeguards.

---

## 3. Next Steps (Day 6)
* Model **Part 3: The Submerged Protective Sensor Shroud / Cage** (slotted cylinder under the keel).
* Implement analog sensor sampling and multi-point calibration formulas for the **pH sensor** and **Turbidity sensor**.

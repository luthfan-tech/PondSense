# Day 6: Submerged Sensor Shroud Design & Analog pH / Turbidity Drivers

- **Date:** 24 November 2026
- **Phase:** Mechanical CAD (Fusion 360) & Embedded Firmware (C++)
- **Focus:** Modeling the slotted submerged sensor shroud / protective cage (Part 3) and implementing multi-sample median-filtered drivers for analog pH and optical turbidity.

---

## 1. Mechanical Engineering: Part 3 Submerged Sensor Shroud / Cage

In outdoor aquaculture fish ponds, raw sensor glass bulbs and optical lenses face severe physical hazards:
1. **Curious Fish:** Species such as Carp, Tilapia, and Rohu aggressively peck at foreign objects and exposed cables.
2. **Biofouling & Floating Debris:** Duckweed, loose pond netting, and filamentous algae can snag and shear sensor heads.
3. **Hydrodynamic Flow:** Stagnant water around a sensor causes artificial micro-climates; continuous ambient water circulation is required for accurate real-time readings.
4. **Anchor Mooring Point:** The bottom of the buoy provides the ideal low-center-of-gravity anchor attachment point to tether the buoy in place against wind drift.

### Shroud Specifications & Design Parameters
* **Outer Diameter:** `Ø 65 mm` (slips cleanly over the `Ø 55 mm` lower hull bulkhead plate)
* **Overall Height:** `70.0 mm` (protects standard 50–60 mm sensor probe bodies)
* **Wall Thickness:** `3.0 mm` uniform (high-durability PETG)
* **Water Circulation Slots:** 6× longitudinal slots (`8.0 mm` width × `40.0 mm` height) spaced symmetrically at 60° intervals via Circular Pattern.
* **Anchor Mooring Port:** Central `Ø 8.0 mm` through-hole on the bottom face to secure nylon anchor tether rope.

---

## 2. Firmware Engineering: Analog pH & Optical Turbidity Drivers

### A. Analog pH Driver with Nernst Temperature Compensation ([`firmware/node/ph_sensor.h`](../../firmware/node/ph_sensor.h))
* **Trimmed Median Filter:** Samples the ADC 15 times with 10 ms settling delay, bubble-sorts the array, and averages the central 5 samples. This completely eliminates spurious electrical noise spikes caused by pond water ground potentials.
* **Dynamic Temperature Compensation:** The theoretical Nernst slope shifts with ambient temperature:
  $$\text{Slope}(T) = -0.1984 \times (T + 273.15) \text{ mV / pH unit}$$
  Using the live DS18B20 water temperature reading from Day 5, the driver dynamically scales the conversion slope, eliminating seasonal drift.

### B. TS-300B Optical Turbidity Driver ([`firmware/node/turbidity_sensor.h`](../../firmware/node/turbidity_sensor.h))
* **Quadratic Transfer Function:** Maps the optical receiver voltage to Nephelometric Turbidity Units (NTU):
  $$\text{NTU} = -1120.4 \cdot V^2 + 5742.3 \cdot V - 4352.9$$
* **Clarity Classification:** Real-time categorizer flags water clarity:
  * `< 25 NTU`: Clear (Optimal for fish health)
  * `25 – 50 NTU`: Moderate (Acceptable feeding conditions)
  * `50 – 100 NTU`: Turbid (Watch for phytoplankton bloom)
  * `> 100 NTU`: Hazard (Severe siltation / run-off)

---

## 3. Next Steps (Day 7)
* Finalize the full 3D assembly in Fusion 360 (Top Lid + Conical Hull + Sensor Shroud).
* Check component clearances, fastener alignments, and O-ring compression margins.
* Implement SX1276 LoRa packet transmission and low-power deep sleep cycles.

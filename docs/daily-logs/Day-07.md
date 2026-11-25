# Day 7: Full Mechanical Buoy Assembly, Section Analysis & LoRa Deep Sleep

- **Date:** 25 November 2026
- **Phase:** Mechanical Assembly (Fusion 360) & Low-Power RF Telemetry (C++)
- **Focus:** Assembling Part 1 (Solar Lid), Part 2 (Conical Hull), and Part 3 (Sensor Shroud) into a master assembly; inspecting internal fitment via Section Analysis; and implementing SX1276 LoRa 865 MHz telemetry transmission with ESP32 deep sleep power gating.

---

## 1. Mechanical Engineering Milestone: Week 1 Assembly Complete

Today marks the culmination of **Week 1 of CAD Modeling**: all three discrete mechanical components were integrated into a unified parametric assembly in Autodesk Fusion 360 (`PondSense_Full_Assembly`).

### Assembly Hierarchy & Joint Architecture
1. **Part 2: Conical Main Hull (Base Reference - Grounded):**
   * Acts as the coordinate datum for the entire buoy.
   * Locked in 3D space (`Ground`) to prevent accidental translation.
2. **Part 1: Top Solar Lid (Upper Mating Interface):**
   * Mated to the hull top collar via a **Rigid Joint** (`J`).
   * Aligns the underside concentric O-ring groove (`Ø 120–126 mm`) squarely with the `120 mm` hull rim.
   * Recessed `80×80 mm` solar bed sits proud on the upper deck at maximum solar zenith angle.
   * LoRa SMA antenna bulkhead port (`Ø 6.5 mm`) sits at the maximum freeboard elevation above the water surface.
3. **Part 3: Submerged Sensor Shroud / Protective Cage (Lower Keel Interface):**
   * Mated beneath the lower `Ø 55 mm` bulkhead plate via a **Rigid Joint** (`J`).
   * Shields protruding sensors (pH electrode, DS18B20 digital probe, and TS-300B optical chamber) within an armored cage.
   * 6× longitudinal intake vents (`8×40 mm`) allow continuous hydrodynamic flow while blocking fish bites and duckweed fouling.
   * Central `Ø 8.0 mm` bottom port provides a low-CG tethering point for the pond anchor rope.

### Internal Section Analysis (Cutaway Inspection)
Using **Inspect → Section Analysis**, an axial cutaway was performed across the primary symmetry plane:
* **Watertight Flange Clearance:** Confirms continuous `3.0 mm` wall thickness without intersection errors or micro-voids.
* **Internal Sled Envelope:** Verifies that the internal electronics sled cavity (`Ø 114 mm` down to `Ø 55 mm`) comfortably accommodates the 18650 cell holder, ESP32 DevKit, and TP4056 charge board.
* **O-Ring Compression Zone:** Confirms a nominal `0.5 mm` compression clearance for a standard `2.5 mm` silicone cord gasket.

---

## 2. Embedded Firmware Engineering: 865 MHz LoRa Telemetry & Deep Sleep

### A. SX1276 LoRa Radio Driver ([`firmware/node/lora_radio.h`](../../firmware/node/lora_radio.h))
* **Indian ISM Band Conformance:** Frequency locked to `865.0 MHz`, operating within the WPC de-licensed `865–867 MHz` band (max 1W ERP).
* **Link Budget Optimization:** Spreading Factor 9 (`SF9`), Bandwidth 125 kHz, Coding Rate 4/5, TX Power `+17 dBm`. This configuration delivers an estimated link budget of **148 dB**, easily providing `2–5 km` range across open agricultural fish ponds.
* **Payload Integrity (CRC16-CCITT):** Dynamically computes a 16-bit cyclic redundancy check over the packed binary telemetry payload prior to RF transmission.

### B. Ultra-Low-Power Deep Sleep Architecture ([`firmware/node/node.ino`](../../firmware/node/node.ino))
Continuous operation in outdoor aquaculture is constrained by battery capacity during cloudy monsoon weeks. The firmware eliminates all active continuous looping in favor of **RTC Timer-Driven Deep Sleep**:

1. **Power Domain Gating:** High-side transistor switch (`PIN_SENSOR_POWER` on GPIO 21) cuts power to all analog sensor frontends between sample cycles. This achieves two critical objectives:
   * **Zero Standby Current:** Cuts sensor idle draw from ~40 mA down to 0 mA.
   * **Anti-Electrolysis Protection:** Prevents DC galvanic corrosion of submerged probe electrodes.
2. **Radio Sleep:** SX1276 transceiver is commanded into register sleep (`< 0.2 µA`).
3. **ESP32 RTC Deep Sleep:** Microcontroller enters deep sleep with only ULP/RTC timer active (`~10–15 µA` current draw).
4. **Power Budget Breakdown:**
   * Active Sampling & TX Window: `~120 mA` for 2.0 seconds.
   * Deep Sleep Window: `~15 µA` for 120 seconds.
   * Average Power Draw: `~2.01 mA`.
   * **Operating Life on single 2600 mAh 18650 cell without solar:** **~53 days continuous operation!** With the 1.5W solar panel, the buoy operates on an infinite energy harvesting loop.

---

## 3. Week 1 Milestones Summary (Days 1–7)

| Day | Focus Area | Deliverable Status |
| :---: | :--- | :--- |
| **Day 1** | Fusion 360 Basics & Navigation | UI, units (`mm`), 2D parametric constraints mastered. |
| **Day 2** | Indian Aquaculture Problem Definition | Research log, competitive analysis, BOM finalized. |
| **Day 3** | Buoy Mechanical Blueprint | Exploded-view mechanical architecture drawing complete. |
| **Day 4** | Part 2: Conical Main Hull (CAD) | Revolved 120mm cone, 3x PG7 ports, sled keyways, STEP export. |
| **Day 5** | Part 1: Top Solar Lid (CAD) | 136mm disc, 80x80mm solar recess, antenna hole, O-ring groove. |
| **Day 6** | Part 3: Submerged Sensor Shroud (CAD) | Slotted protective cage, pH & turbidity median-filtered drivers. |
| **Day 7** | Full Buoy Master Assembly (CAD) | Rigid joints, section analysis, SX1276 LoRa TX & deep sleep. |

---

## 4. Next Steps (Week 2 Kickoff - Day 8)
* Model **Part 4: Internal Electronics Sled / Chassis** (modular sliding rack for ESP32, 18650 holder, and TP4056 that locks into the hull's 6 internal keyway slots).
* Scaffold the **Base Station Gateway Firmware** (`firmware/gateway/`) to receive LoRa packets and output JSON strings over USB Serial to the farmhouse server.

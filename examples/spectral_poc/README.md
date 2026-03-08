# Spectral NILM Auditor

The **Spectral NILM Auditor** is a high-performance energy intelligence system that provides deep visibility into electrical infrastructure **without intrusive sub-metering**. Leveraging the edge-computing capabilities of the **Tuya T5AI-Core**, it transforms raw electrical signals into actionable insights about appliance health, grid quality, and consumption patterns.

---

## 🚀 Vision & Problem Statement

Traditional smart meters act as simple accumulators, recording total energy consumption (kWh) but remain "blind" to individual loads. This lack of granularity prevents:

- Identifying inefficient appliances  
- Detecting early-stage equipment failures  
- Understanding the composition of energy bills  

The **Spectral NILM Auditor** solves this by providing **Spectral Intelligence**. It analyzes microscopic distortions in current and voltage waveforms—known as **harmonic signatures**—to disaggregate the total load. This allows the identification of specific devices based on their unique electrical "fingerprints" at the point of entry.

---

## 🏗️ Technical Architecture

The system uses a **dual-channel high-speed sensing approach** to capture a complete picture of electrical transients.

### High-Speed Sampling

- **Controller:** Tuya T5AI-Core (BK7258)  
- **Sampling Rate:** 4,000 samples/second across voltage and current  
- Captures harmonics up to the **40th order**, essential for accurate appliance classification  

### Spectral Engine (Local DFT)

- Executes a **local Discrete Fourier Transform (DFT)** to extract amplitude and phase of harmonics  
- Focuses on **third ($h_3$) and fifth ($h_5$) harmonic ratios**:

```math
h_n = \frac{I_n}{I_1}

Where:  
- \(I_1\) = fundamental frequency (50/60Hz)  
- \(I_n\) = n-th harmonic current  

### Edge Inference

Classifies loads based on harmonic ratios:

| Load Type           | Examples                    | Harmonic Signature |
|--------------------|----------------------------|------------------|
| Resistive           | Heaters, incandescent bulbs | \(h_3 \approx 0\) |
| Non-linear / SMPS   | Laptop chargers, LED drivers | High \(h_3\) |
| Inductive           | Motors, compressors         | Specific phase shifts |

---

## 🛠️ Hardware List

- **Core Controller:** Tuya T5AI-Core (BK7258)  
- **Voltage Sensing:** ZMPT101B Active Voltage Transformer  
- **Current Sensing:** SCT-013 Non-invasive Split-core CT  
- **Connectivity:** Wi-Fi + Bluetooth LE  
- **Operating Environment:** TuyaOpen SDK on FreeRTOS  

---

## 💡 Applications

### 1. Predictive Maintenance for Industry

- Monitors \(h_3\) and \(h_5\) of industrial motors  
- Detects **harmonic drift**, indicating: winding degradation, insulation failure, or bearing wear  
- Enables scheduled maintenance and avoids costly downtime  

### 2. Advanced Load Disaggregation

- Identifies when high-draw appliances (HVAC, refrigerators) cycle on/off  
- Generates **itemized energy bills** without separate meters  

### 3. Grid Health & THD Monitoring

- Calculates **Total Harmonic Distortion (THD)** locally:

```math
THD = \frac{\sqrt{\sum_{n=2}^{\infty} I_n^2}}{I_1}


### 4. 🏗️ Technical Architecture

- **Sampling:** High-fidelity 4000Hz sampling of Voltage and Current.

- **Spectral Engine:** Implementation of local Discrete Fourier Transform (DFT) to isolate $h_3$ and $h_5$ harmonic ratios.

- **Edge Inference:** Categorizes loads (e.g., LED drivers, SMPS, Inductive Motors) based on spectral fingerprints.



### 5. 🛠️ Hardware List

- **Controller:** Tuya T5AI-Core (BK7258 Chipset)

- **Voltage Sensor:** ZMPT101B

- **Current Sensor:** SCT-013 Non-invasive CT

- **Framework:** TuyaOpen SDK / FreeRTOS



### 6. ⚖️ Ethics & Resilience

The project ensures data privacy via edge-processing. 

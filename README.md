# V-Rod-Custom-Fuel-Level-Sensor
A custom DIY microcontroller-based (Arduino NANO Super Mini Atmega328) fuel level sensor for Harley-Davidson V-Rod. Replaces the unreliable factory ultrasonic sensor with a magnetic reed-switch array.

## The Problem
The OEM ultrasonic sensor is prone to failure due to vibrations, temperature changes, and piezo-element degradation. When it fails, the fuel gauge typically shows "Full" or drops to zero randomly.

## The Solution
This project replaces the ultrasonic unit with a highly reliable **magnetic reed-switch array**, processed by an **Atmega328 microcontroller**. It provides a perfectly stable analog signal (current sink) to the instrument cluster, perfectly mimicking a working OEM sensor.

### Hardware Features
<img src="photos/IMG_20260620_183127.jpg" width="600">
- **Sensor Probe:** Aluminum tube housing an array of 40 reed switches and resistors.
<img src="photos/IMG_20260610_010121.jpg" width="600">

<img src="photos/IMG_20260610_010118.jpg" width="600">

<img src="photos/IMG_20260610_010407.jpg" width="600">
- **The Float (PVC-2-195):** The float is custom-machined from **ПХВ-2-195 (PVC-2-195)** — an aviation-grade, closed-cell polyvinyl chloride foam. Unlike regular plastics or foams, it is 100% resistant to high-octane gasoline and ethanol blends, never absorbs fuel, and securely holds the dual neodymium magnets.
<img src="photos/IMG_20260617_225749.jpg" width="600">

- **Microcontroller:** Arduino NANO Super Mini Atmega328.

- **The Active Feedback Loop (LM358 + AO3400A):** 
This is the heart of the hardware. Simply using a PWM-driven transistor to ground the signal wire causes gauge needle bounce due to alternator voltage spikes (ranging from 12.5V at idle to 14.4V while riding). To solve this, the project utilizes a combination of a hardware constant current sink and dynamic software compensation:

1. **Hardware Constant Current Sink:** The Arduino Nano outputs a high-frequency PWM signal, which is smoothed into a stable DC reference voltage using an RC filter. This reference voltage is fed into the non-inverting input of the LM358 operational amplifier. Operating in a closed-loop feedback configuration with the AO3400A N-channel MOSFET and a source sense resistor, the LM358 continuously regulates the MOSFET's gate voltage. This ensures a precise, steady current draw (ranging from 12 mA to 20 mA) from the instrument cluster, shielding the signal from rapid electrical noise on the line.

2. **Dynamic Voltage Compensation:** Although the constant current sink stabilizes the output, fluctuations in the motorcycle's overall system voltage (12.5V to 14.4V) can still slightly affect the cluster's internal current-sensing behavior. To address this, the Arduino Nano monitors the system voltage via a resistor voltage divider connected to an analog pin. The firmware measures this voltage in real-time and dynamically adjusts the PWM duty cycle to compensate for these system-wide variations, keeping the gauge needle stable and consistent under all riding conditions.

## Tank Calibration
The V-Rod fuel tank has a complex, irregular shape. To ensure absolute accuracy (including the "Miles to Empty" range calculator), the sensor was mapped by filling the tank liter by liter with actual fuel.

## Schematic & PCB
<img src="photos/schematic.png" width="600">

## Source Code
Developed for the Harley-Davidson V-Rod, which has a complex-shaped under-seat fuel tank causing standard sensors to read highly non-linearly.

This code converts the non-linear resistance signal from the fuel float into a perfectly smooth PWM signal for an analog gauge, distributing the fuel volume evenly across the gauge marks (Empty, 1/4, 1/2, 3/4, Full).

## 🚀 Features
* **Anti-Slosh Logic:** Dual signal filtering (Median filter + Smoothing/Low-pass filter). The needle won't jump during braking, acceleration, or cornering.
* **Custom Tank Calibration:** A 33-point lookup table allows perfect volume mapping for a fuel tank of any geometric shape.
* **System Voltage Compensation:** Sensor readings remain accurate even during voltage drops (e.g., turning on headlights or radiator fans), as the Arduino monitors the battery voltage and dynamically compensates the PWM output.
* **Gauge Calibration Mode:** Built-in `CALIBRATION_MODE` to quickly find the exact PWM values needed for your specific aftermarket gauge.

## 🛠 Hardware & Wiring
* **Microcontroller:** Arduino Nano / Pro Mini (or any compatible board).
* **Sensor Input:** `A1` *(A pull-up resistor is required depending on your specific sender's resistance).*
* **Battery Voltage Monitor:** `A4` ⚠️ **WARNING:** Must be connected via a voltage divider! Motorcycle system voltage can reach 14.5V+, which will fry the Arduino pin if connected directly.
* **Gauge Output:** `D10` *(PWM output to the gauge).*

## 📐 Calibration Guide

If you are adapting this project for a different motorcycle or fuel tank, you need to complete a 2-step calibration process:

### Step 1: Gauge Needle Calibration
1. Set `bool CALIBRATION_MODE = true;` at the beginning of the code.
2. Change the `TEST_PWM` value, upload the code, and observe where the needle stops on the gauge.
3. Find the exact PWM values for your gauge marks (Empty, 1/4, 1/2, 3/4, Full) and write them into the `gPWM[]` array.
4. In the `gLiters[]` array, define how many liters each mark represents (e.g., for a 20.5L tank, the step for each 1/8 increment would be 2.5L).

### Step 2: Fuel Tank Mapping
1. Set `CALIBRATION_MODE = false;`.
2. Completely drain the fuel tank. Open the Arduino IDE Serial Monitor at 115200 baud.
3. Record the `Raw_Sensor` value for the empty tank.
4. Pour fuel into the tank in small, measured portions (e.g., 1 liter at a time) and record the `Raw_Sensor` readings for each step.
5. Transfer these "ADC - Liters" pairs into the `rawTable[]` and `litersTable[]` arrays. 
*Important: The arrays must be strictly ascending, with no duplicate ADC values in a row!*



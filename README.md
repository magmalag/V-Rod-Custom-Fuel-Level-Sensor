# V-Rod-Custom-Fuel-Level-Sensor
A custom DIY microcontroller-based (Arduino NANO Super Mini Atmega328) fuel level sensor for Harley-Davidson V-Rod. Replaces the unreliable factory ultrasonic sensor with a magnetic reed-switch array.

# Harley-Davidson V-Rod Custom Fuel Level Sensor

This project is a complete DIY replacement for the notoriously unreliable factory ultrasonic fuel level sensor found on Harley-Davidson V-Rod motorcycles. 

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
- **Float:** Custom-shaped fuel-resistant float with embedded neodymium magnets.
<img src="photos/IMG_20260617_225749.jpg" width="600">
- **Microcontroller:** Arduino NANO Super Mini Atmega328.
- **Signal Output:** Custom transistor-based circuit (AO3400A) providing a stable current sink for the dashboard, regardless of alternator voltage spikes.

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



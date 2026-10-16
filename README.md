# ESP32 Water Tank Monitor

This is a university project built using an **ESP32** board and an **HC-SR04 / Ultrasonic Sensor** mounted on a custom prototype pipe to monitor water tank levels in real time. The system features an embedded web server dashboard, status LEDs, and a buzzer for alerts.

## Features
- **Live Web Dashboard:** Displays current water percentage, status, and alerts over local Wi-Fi.
- **Refill Tracking:** Tracks time elapsed since the last detected tank refill.
- **Hardware Indicators:**
  - 🟢 **Green LED:** High water level (>= 75%)
  - 🟡 **Yellow LED:** Medium water level (25% - 74%)
  - 🔴 **Red LED + Buzzer:** Low water alert (< 25%)
  - 🚨 **Fast Buzzer Alert:** Overflow warning (>= 98%)

## Components Used
- ESP32 Development Board
- Ultrasonic Distance Sensor
- LEDs (Red, Yellow, Green)
- Buzzer
- Breadboard & Resistors
- Prototype Plastic Pipe Structure

## Pin Connections
- Trig Pin -> GPIO 14
- Echo Pin -> GPIO 25
- Red LED -> GPIO 26
- Yellow LED -> GPIO 27
- Green LED -> GPIO 33
- Buzzer -> GPIO 32

---

## Hardware Setup & Gallery

### Ultrasonic Sensor Mount
![Sensor Top](schematics/sensor_top.jpg.jfif)
![Sensor Board](schematics/sensor_bottom.jpg.jfif)

### Circuit & Breadboard Wiring
![Wiring Setup](schematics/breadboard_connections.jpg.jfif)

### Full Prototype Setup
![Full System 1](schematics/full_setup.jpg.jfif)
![Full System 2](schematics/full_setup2.jpg.jfif)

---

## How to Run
1. Open `src/main.cpp` using Arduino IDE or VS Code.
2. Enter your Wi-Fi SSID and Password.
3. Upload the code to the ESP32 board.
4. Open the Serial Monitor to get the IP address, then type that IP into any web browser.

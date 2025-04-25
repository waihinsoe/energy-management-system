# Energy Management System

This project presents a **low-cost Energy Management System (EMS)** designed to optimize electricity usage and avoid energy wastage in household environments, particularly focused on controlling an air cooler. The system is based on the **ESP32 microcontroller** and integrates multiple sensors for real-time monitoring and control.


## 🎯 Aim & Objectives

### Aim
To implement a **low-cost energy management system** that promotes efficient electricity use and prevents energy wastage.

### Objectives
- Investigate hardware specifications and integration with ESP32.
- Detect current and voltage drawn from appliances.
- Control home appliances using ESP32 and solid-state relay.

## 🧰 Hardware Components

- **ESP32 Microcontroller**  
- **Current Sensor (ACS712)**  
- **Voltage Sensor (F031-06)**  
- **Temperature Sensor (DHT22)**  
- **Solid State Relay Module (SSR)**  
- **Air Cooler (Glacier GAC-830)**

## 💻 Software Tools

- **Arduino IDE** – Programming the ESP32.
- **Blynk App** – Remote monitoring and control.
- **Firebase Cloud** – Cloud storage and data sync.

## 🗂 Application Areas

- Residential Homes
- Offices & Factories
- Public Services
- Emergency Facilities

## 📐 System Overview

The system continuously collects data from current, voltage, and temperature sensors connected to the ESP32. Based on real-time data, it controls the air cooler using a solid-state relay. The system supports:

- Real-time monitoring on OLED and mobile app (Blynk).
- Remote control via smartphone.
- Data synchronization with Firebase.

## 🔁 System Flow

1. Collect sensor data (current, voltage, temperature).
2. Process and analyze data using ESP32.
3. Display data on OLED and send to Firebase.
4. Receive user commands from Blynk App.
5. Control the air cooler via SSR module accordingly.


## 📈 Future Work

- Extend simulation results to include all sensors.
- Implement real hardware testing.
- Enhance app integration with more automation features.

## 📚 References

Key references include IEEE conference papers and documentation from Arduino, Blynk, Firebase, and various energy system research articles. Full list available in the project documentation.

---

**Thank you for checking out this project!**

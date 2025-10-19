# Dual-Axis Solar Tracking System with Weather Sensor

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Project Status](https://img.shields.io/badge/Status-Complete-brightgreen)](https://github.com/Dancode-188/dual-axis-solar-tracker)

**Course:** FEE 361 - Sensors and Measurements  
**Institution:** University of Nairobi - Department of Electrical and Information Engineering  
**Academic Year:** 2025/2026  
**Project Number:** 91

---

## 📋 Project Overview

An intelligent dual-axis solar tracking system that automatically adjusts solar panel orientation to maximize energy capture throughout the day. The system integrates multiple sensors for sun position detection and weather-based protection, controlled by an Arduino Mega 2560 microcontroller.

### Key Features

- **Dual-Axis Tracking:** Independent azimuth (0-360°) and elevation (0-90°) control
- **Intelligent Sun Detection:** 4 LDR sensor array for accurate solar positioning
- **Weather Protection:** Automatic stowing during rain or extreme temperatures
- **Real-time Monitoring:** 16x2 LCD display with system status
- **Manual Override:** Button controls for manual positioning and system reset
- **State Machine Architecture:** Professional software design with multiple operating modes

---

## 🎯 Project Objectives

1. Design and implement a dual-axis solar tracking mechanism
2. Integrate multiple sensors (LDR, temperature, humidity, rain)
3. Develop intelligent control algorithms for sun tracking
4. Implement weather-based safety features
5. Create a user-friendly interface with LCD display
6. Document complete hardware and software design process

---

## 🔧 Hardware Components

### Core Controller
- **Arduino Mega 2560** - Main microcontroller (ATmega2560)

### Sensors
- **4x LDR Sensors** - Solar position detection (quadrant configuration)
- **DHT22** - Temperature and humidity monitoring
- **Rain Sensor** - Precipitation detection for weather protection
- **Push Buttons (2x)** - Manual control and system reset

### Actuators
- **Servo Motor (MG996R)** - Azimuth axis control (horizontal rotation)
- **Stepper Motor (NEMA 17)** - Elevation axis control (vertical tilt)
- **A4988 Driver** - Stepper motor control

### User Interface
- **16x2 LCD Display (I2C)** - System status and readings
- **LEDs** - Status indicators

### Power Supply
- **12V DC Power Supply** - Main system power
- **Voltage Regulators** - 5V for logic, sensors

---

## 📁 Repository Structure

```
dual-axis-solar-tracker/
│
├── README.md                          # Project overview and documentation
├── LICENSE                            # MIT License
├── .gitignore                         # Git ignore rules
├── WOKWI-SETUP-INSTRUCTIONS.md       # Wokwi simulation setup guide
│
├── docs/                              # Complete project documentation
│   ├── PROJECT-PROGRESS.md           # Project tracking and status
│   ├── Q6-Arduino-Code-Documentation.md
│   ├── Q7-Simulation-Testing-Results.md
│   ├── Q8-Results-Analysis.md
│   └── Q9-PCB-Layout-Design.md
│
├── src/                               # Production source code
│   └── solar_tracker/
│       └── solar_tracker.ino         # Main Arduino program (1,255 lines)
│
├── simulation/                        # Wokwi simulation files
│   ├── diagram.json                  # Circuit configuration
│   ├── solar_tracker_wokwi.ino      # Simulation code (527 lines)
│   └── README.md                     # Simulation documentation
│
├── hardware/                          # Hardware design files
│   ├── pcb/                          # PCB design outputs
│   │   ├── images/                   # Layer visualizations
│   │   ├── gerbers/                  # Manufacturing files
│   │   └── source/                   # EasyEDA source files
│   └── schematics/                   # Circuit diagrams
│
└── images/                            # Documentation images
    ├── q7-simulation/                # Simulation screenshots
    └── README.md                     # Image documentation
```

---

## 🚀 Project Status

**Phase:** ✅ PROJECT COMPLETE  
**Progress:** 100% (All contracted deliverables complete)

### Completed Deliverables
- ✅ Q1: Background Research & Literature Review (4,800+ words)
- ✅ Q2: System Block Diagram Design (Hand-drawn)
- ✅ Q3: Component Analysis & Selection (11,500+ words)
- ✅ Q4: Detailed Circuit Diagram (Hand-drawn)
- ✅ Q5: Software Flowcharts & Algorithms (5 flowcharts)
- ✅ Q6: Arduino Code Implementation (1,255 lines, validated)
- ✅ Q7: Wokwi Simulation & Testing (100% pass rate)
- ✅ Q8: Results Analysis (536 lines)
- ✅ Q9: PCB Layout Design (Manufacturing-ready)

### Project Achievements
- ✅ 100% test success rate (12 scenarios, 5 documented)
- ✅ Zero DRC errors in PCB design
- ✅ Production-quality code with comprehensive documentation
- ✅ 50,000+ words of technical documentation
- ✅ Fully functional Wokwi simulation: [View Simulation](https://wokwi.com/projects/445089071466408961)
- ✅ Manufacturing-ready Gerber files for PCB fabrication
- ✅ Complete GitHub repository with professional structure

**Note:** Q10 (System Enhancement) was not included in the original project scope as per client agreement.

---

## 💻 Software Architecture

### State Machine Design
The system operates in 6 distinct states:
1. **INIT** - System initialization
2. **SEARCHING** - Sun detection mode
3. **TRACKING** - Active solar tracking
4. **STOW** - Weather protection mode
5. **MANUAL** - User-controlled positioning
6. **ERROR** - Fault handling

### Control Algorithm
- **LDR-based tracking:** Quadrant comparison for sun position
- **Weather monitoring:** Continuous temperature, humidity, and rain checks
- **Motor control:** Smooth servo and stepper positioning
- **Safety limits:** Software-enforced mechanical boundaries

---

## 📊 Technical Specifications

| Parameter | Specification |
|-----------|--------------|
| Tracking Range (Azimuth) | 0° to 360° |
| Tracking Range (Elevation) | 0° to 90° |
| Tracking Accuracy | ±5° |
| Tracking Speed | ~5°/second |
| Update Rate | 1 Hz (once per second) |
| Operating Voltage | 12V DC |
| Controller | ATmega2560 (Arduino Mega) |
| Weather Protection | Rain & Temperature |

---

## 🎓 Academic Context

This project fulfills the requirements for FEE 361 (Sensors and Measurements) at the University of Nairobi. It demonstrates:

- Sensor selection and interfacing
- Signal conditioning and processing
- Microcontroller programming
- Control system design
- Hardware-software integration
- Documentation and presentation skills

---

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## 👤 Author

**Project Developer:** Hilary Audi  
**Institution:** University of Nairobi  
**Department:** Electrical and Information Engineering  
**Course:** FEE 361 - Sensors and Measurements

---

## 🔗 Links

- **GitHub Repository:** [https://github.com/Dancode-188/dual-axis-solar-tracker](https://github.com/Dancode-188/dual-axis-solar-tracker)
- **University of Nairobi:** [https://uonbi.ac.ke](https://uonbi.ac.ke)

---

## 📧 Contact

For questions or collaboration:
- GitHub: [@Dancode-188](https://github.com/Dancode-188)

---

**Last Updated:** October 19, 2025  
**Project Status:** ✅ Complete - All Deliverables Met  
**Ready for:** Client Handover, Professor Review, Physical Implementation

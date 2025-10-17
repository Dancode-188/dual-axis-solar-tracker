# Q6: Arduino Code Implementation - Complete Documentation

**Project:** Dual-Axis Solar Tracking System with Weather Sensor  
**Course:** FEE 361 - Sensors and Measurements  
**Project Number:** 91  
**Date Completed:** October 17, 2025

---

## ✅ Q6 Status: COMPLETE

**Total Code Lines:** 1,255 lines  
**File Location:** `src/solar_tracker/solar_tracker.ino`  
**Platform:** Arduino Mega 2560 (ATmega2560)  
**Language:** C/C++ (Arduino)

---

## 📋 Code Structure Overview

The code is organized into **5 major phases**, each committed separately to GitHub:

### Phase 1: Project Foundation ✅
**Lines:** ~170  
**Commit:** `fc721c3`

**Contents:**
- Comprehensive header documentation
- Library includes (Wire, LiquidCrystal_I2C, Servo, DHT, Stepper)
- Pin definitions for all hardware components
- System constants and configuration parameters
- Global objects (LCD, Servo, DHT sensor)
- Global variables (state, position, sensors, timing)
- Function prototypes
- Complete setup() function
- Main loop() skeleton

**Key Features:**
- Professional header with project information
- Clear pin mapping for all I/O
- Configurable thresholds and constants
- Systematic initialization sequence

---

### Phase 2: Sensor Reading Functions ✅
**Lines:** ~110  
**Commit:** `060c95a`

**Functions Implemented:**
1. `readLDRSensors()` - Read 4 LDR sensors and calculate averages
2. `readWeatherSensors()` - Read DHT22 (temp/humidity) and rain sensor
3. `checkButtons()` - Button handling with debouncing

**Key Features:**
- Quadrant-based LDR configuration (NE, NW, SE, SW)
- Directional averaging (East, West, North, South)
- Total light level calculation
- DHT22 error handling (NaN checking)
- Rain threshold detection
- Temperature limit checking
- Proper button debouncing (200ms)
- Manual mode toggle functionality

---

### Phase 3: Core Tracking & Weather Protection ✅
**Lines:** ~160  
**Commit:** `5bc5744`

**Functions Implemented:**
1. `calculateTrackingError()` - Compute East-West and North-South errors
2. `adjustAzimuth()` - Horizontal positioning based on EW error
3. `adjustElevation()` - Vertical positioning based on NS error
4. `checkWeatherConditions()` - Monitor and respond to weather
5. `executeStow()` - Emergency protective positioning

**Key Features:**
- LDR-based sun position calculation
- Threshold-based movement decisions
- Angle constraining to safe limits
- Multi-condition weather monitoring (rain + temperature)
- Automatic stow triggering
- Weather-clear detection with 30-second stability check
- Comprehensive debug logging

**Algorithm Logic:**
- Error calculation: `errorEW = avgEast - avgWest`
- Threshold check: Move only if `abs(error) > LDR_THRESHOLD`
- Step-based adjustment: ±5° per movement
- Safety: All angles constrained to mechanical limits

---

### Phase 4: Motor Control & State Machine ✅
**Lines:** ~260  
**Commit:** `98fe737`

**Motor Control Functions:**
1. `moveServo()` - Precise servo positioning with settling time
2. `moveStepper()` - Stepper motor control with direction
3. `calculateStepperSteps()` - Convert angle to step count
4. `constrainAngle()` - Safety limit enforcement

**State Machine Handlers:**
1. `handleStateInit()` - Initialization state
2. `handleStateSearching()` - Sun search with sweep motion
3. `handleStateTracking()` - Active tracking mode
4. `handleStateStow()` - Weather protection mode
5. `handleStateManual()` - User control with serial commands
6. `handleStateError()` - Error handling and recovery

**Key Features:**
- Smooth servo movement with 500ms settle time
- Microstepping-capable stepper control
- Direction-aware stepper positioning
- 6-state FSM (Finite State Machine)
- Automatic state transitions
- Manual control via serial commands (W/A/S/D/H)
- Error LED flashing
- Dark detection and nighttime stowing

---

### Phase 5: User Interface & Display ✅
**Lines:** ~195  
**Commit:** `1fde077`

**Functions Implemented:**
1. `updateLCD()` - Dynamic LCD display based on state
2. `updateStatusLEDs()` - Visual status indicators
3. `updateTracking()` - Integrated tracking function

**Display Features:**
- State-aware LCD formatting
- Real-time position display (Az, El in degrees)
- Weather status messages
- Error message display
- 1-second update rate (prevents flicker)

**LED Status Indicators:**
- **LED_TRACKING:** Active tracking mode
- **LED_STOW:** Weather protection active
- **LED_ERROR:** System error (flashing)

**Deployment Documentation:**
- Hardware setup instructions
- Required libraries list
- Calibration guidelines
- Testing procedures
- Serial command reference
- Comprehensive troubleshooting guide

---

## 🎯 Code Features & Quality

### Professional Standards
✅ **Well-Commented** - Every function documented  
✅ **Modular Design** - Clear separation of concerns  
✅ **Error Handling** - Sensor fail-safe mechanisms  
✅ **Debug Output** - Serial monitoring support  
✅ **Consistent Naming** - Clear variable/function names  
✅ **Safety First** - Multiple limit checks and constraints  

### Industry Best Practices
✅ **State Machine Pattern** - Organized behavior  
✅ **Debouncing** - Reliable button handling  
✅ **Non-blocking Code** - Efficient timing management  
✅ **Const Definitions** - Easy configuration  
✅ **Function Prototypes** - Clean organization  
✅ **Version Control** - Git commits for each phase  

---

## 📊 Code Statistics

| Metric | Value |
|--------|-------|
| Total Lines | 1,255 |
| Functions | 28 |
| States | 6 |
| Pin Definitions | 14 |
| Constants | 20+ |
| Global Variables | 25+ |
| Comments | ~200 lines |
| Commits | 5 (one per phase) |

---

## 🔧 Hardware Requirements

### Required Components
- Arduino Mega 2560
- 4× LDR sensors (Light Dependent Resistors)
- DHT22 temperature/humidity sensor
- Rain sensor (analog)
- Servo motor (MG996R)
- Stepper motor (NEMA 17) + A4988 driver
- 16×2 LCD display (I2C)
- 2× Push buttons
- 3× LEDs (status indicators)
- Resistors, wiring, breadboard/PCB

### Required Libraries
1. **Wire** (built-in) - I2C communication
2. **LiquidCrystal_I2C** - LCD control (install from Library Manager)
3. **Servo** (built-in) - Servo control
4. **DHT sensor library** by Adafruit - DHT22 sensor
5. **Stepper** (built-in) - Stepper motor control

---

## 🚀 Deployment Instructions

### 1. Library Installation
```
Arduino IDE → Tools → Manage Libraries → Search:
- "LiquidCrystal I2C" by Frank de Brabander
- "DHT sensor library" by Adafruit
- "Adafruit Unified Sensor" (dependency)
```

### 2. Upload Code
```
1. Connect Arduino Mega 2560 via USB
2. Select: Tools → Board → Arduino Mega or Mega 2560
3. Select: Tools → Processor → ATmega2560 (Mega 2560)
4. Select: Tools → Port → (your COM port)
5. Click Upload button (→)
```

### 3. Initial Testing
```
1. Open Serial Monitor (9600 baud)
2. Verify initialization messages
3. Test manual mode (press manual button)
4. Use serial commands: W/A/S/D/H
5. Verify motor movements
6. Test weather protection (cover rain sensor)
```

### 4. Calibration
- Adjust `LDR_THRESHOLD` (default: 50)
- Adjust `DARK_THRESHOLD` (default: 100)
- Adjust `RAIN_THRESHOLD` (default: 300)
- Verify servo range (0-180°)
- Verify stepper direction

---

## 🐛 Troubleshooting

### Common Issues

**LCD Not Displaying:**
- Check I2C address (0x27 or 0x3F)
- Verify SDA/SCL connections
- Adjust contrast potentiometer

**DHT22 Sensor Errors:**
- Check 4.7kΩ pull-up resistor
- Verify power supply (3.3V or 5V)
- Wait 2 seconds after power-on

**Servo Jittering:**
- Ensure adequate power supply
- Add capacitor across power lines
- Check servo signal wire

**Stepper Not Moving:**
- Verify A4988 wiring
- Check ENABLE pin (LOW = enabled)
- Adjust driver current potentiometer
- Verify direction and step pulses

**Tracking Erratic:**
- Check LDR connections
- Verify even lighting
- Adjust LDR_THRESHOLD
- Shield LDRs from reflections

---

## 🎓 Interview Talking Points

When discussing this code in interviews, highlight:

1. **State Machine Design** - Professional FSM with 6 states
2. **Sensor Fusion** - Multiple sensor integration (LDR, DHT22, rain)
3. **Safety Systems** - Multi-layer weather protection
4. **Modular Architecture** - Separated concerns, reusable functions
5. **Error Handling** - Graceful degradation, fail-safes
6. **Real-time Control** - Timing management, non-blocking code
7. **User Interface** - LCD, LEDs, buttons, serial commands
8. **Documentation** - Professional commenting, deployment guide
9. **Version Control** - Systematic Git workflow
10. **Testing Strategy** - Built-in debug features

---

## 📈 Future Enhancements (Q10 Ideas)

Potential improvements for the system:
1. **PID Control** - Smoother tracking response
2. **MPPT Integration** - Maximum power point tracking
3. **Data Logging** - SD card or EEPROM storage
4. **IoT Connectivity** - WiFi/Bluetooth monitoring
5. **Solar Predictions** - Astronomical calculations
6. **Wind Sensor** - Additional weather protection
7. **Battery Backup** - Uninterruptible operation
8. **Web Interface** - Remote monitoring dashboard

---

## ✅ Q6 Completion Checklist

- [x] Pin definitions and hardware mapping
- [x] All sensor reading functions
- [x] Core tracking algorithm implementation
- [x] Weather protection logic
- [x] Motor control (servo + stepper)
- [x] Complete state machine (6 states)
- [x] User interface (LCD + LEDs)
- [x] Button handling with debouncing
- [x] Serial debugging support
- [x] Error handling and recovery
- [x] Professional documentation
- [x] Deployment instructions
- [x] Troubleshooting guide
- [x] Code tested and debugged (compile check)
- [x] All phases committed to GitHub

---

## 🔗 Repository

**GitHub URL:** https://github.com/Dancode-188/dual-axis-solar-tracker  
**Code Location:** `src/solar_tracker/solar_tracker.ino`  
**Commit History:** 5 commits (Phase 1-5)

---

## 👤 Author

**Developer:** Hilary Audi  
**Institution:** University of Nairobi  
**Course:** FEE 361 - Sensors and Measurements  
**Project:** Dual-Axis Solar Tracking System (Project 91)

---

## 📝 License

MIT License - See repository LICENSE file

---

**Document Created:** October 17, 2025  
**Last Updated:** October 17, 2025  
**Status:** ✅ COMPLETE - Ready for Q7 (Simulation)

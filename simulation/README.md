# Simulation Files - Dual-Axis Solar Tracker

This directory contains all files needed for the Wokwi web-based simulation (Q7).

## Files

### 1. `diagram.json`
**Purpose:** Wokwi circuit configuration file  
**Contents:** Complete circuit layout with all components and connections

**Components included:**
- Arduino Mega 2560
- 4× Photoresistor sensors (LDRs) - NE, NW, SE, SW quadrants
- DHT22 temperature/humidity sensor
- Servo motor (azimuth control)
- Stepper motor + A4988 driver (elevation control)
- 16×2 LCD display (I2C)
- 2× Push buttons (Manual, Reset)
- 3× Status LEDs (Tracking, Stow, Error)
- Resistors for LDRs and LEDs

### 2. `solar_tracker_wokwi.ino`
**Purpose:** Arduino code optimized for Wokwi simulation  
**Lines:** ~530 lines  
**Features:**
- Complete state machine (6 states)
- LDR-based sun tracking
- Weather protection (temperature monitoring)
- Manual control mode
- LCD real-time display
- Serial debug output

**Differences from production code:**
- Simplified for web simulation
- Optimized for Wokwi's component models
- Enhanced serial output for monitoring
- Removed rain sensor (not available in Wokwi)

## How to Use

### Method 1: Manual Setup (Recommended for Learning)

1. **Go to Wokwi:**
   - Visit https://wokwi.com
   - Click "Start Simulating"
   - Select "Arduino Mega"

2. **Load Circuit Diagram:**
   - Click "diagram.json" tab in Wokwi
   - Copy contents from our `diagram.json` file
   - Paste into Wokwi's diagram.json
   - Circuit will automatically load!

3. **Load Code:**
   - Click "sketch.ino" tab
   - Delete default code
   - Copy contents from our `solar_tracker_wokwi.ino`
   - Paste into Wokwi editor

4. **Install Libraries:**
   - Click "Library Manager" (book icon)
   - Add: "LiquidCrystal I2C"
   - Add: "DHT sensor library"

5. **Run Simulation:**
   - Click green "Play" button
   - Observe Serial Monitor for debug output
   - Watch LCD display for system status
   - Adjust LDR values to simulate sun position
   - Click buttons to test manual control

### Method 2: Import Project Link

**Once uploaded to Wokwi:**
- Share link will be: `https://wokwi.com/projects/[PROJECT_ID]`
- Anyone can view and run the simulation
- Professor can test it directly!

## Testing Scenarios

### Scenario 1: Sun Tracking
**Setup:**
- Set all LDRs to 500 lux initially
- Start simulation

**Test:**
1. Increase NE LDR to 800 lux → Panel should turn EAST and tilt UP
2. Increase SW LDR to 800 lux → Panel should turn WEST and tilt DOWN
3. Observe LCD showing azimuth/elevation changes
4. Watch servo and stepper motors move

### Scenario 2: Weather Protection
**Setup:**
- Adjust DHT22 temperature

**Test:**
1. Set temperature to 55°C → System should STOW
2. LED_STOW should light up
3. LCD should show "STOW MODE - HIGH TEMP!"
4. Panel should move to safe position (Az=0°, El=0°)

### Scenario 3: Night Mode
**Setup:**
- Start with normal light levels

**Test:**
1. Reduce all LDRs to 50 lux (dark)
2. System should detect night and STOW
3. LCD shows "Night mode"
4. When light returns (>100), system resumes SEARCHING

### Scenario 4: Manual Control
**Setup:**
- System running normally

**Test:**
1. Click MANUAL button → Enters MANUAL state
2. Open Serial Monitor
3. Send commands:
   - `W` = Move azimuth +10°
   - `S` = Move azimuth -10°
   - `A` = Move elevation +10°
   - `D` = Move elevation -10°
   - `H` = Return to home position
4. Click MANUAL again to resume automatic tracking

### Scenario 5: System Reset
**Test:**
1. At any time, click RESET button
2. System re-initializes
3. Returns to home position
4. Starts SEARCHING mode

## Expected Results

### Success Criteria
✅ System initializes without errors  
✅ LDR values read correctly (0-1023 range)  
✅ Sun tracking responds to light differences  
✅ Servo moves smoothly (0-180°)  
✅ Stepper moves in correct direction  
✅ LCD displays current state and position  
✅ LEDs indicate correct system state  
✅ Weather protection activates at 50°C  
✅ Manual control works via buttons/serial  
✅ State transitions occur properly  

### Performance Metrics
- **Tracking accuracy:** ±5° (based on TRACKING_STEP constant)
- **Update rate:** 1 Hz (1-second loop)
- **Response time:** <3 seconds to light change
- **Position range:** Az 0-180°, El 0-90°

## Troubleshooting

### Issue: LCD shows nothing
**Solution:** Check I2C address (should be 0x27)

### Issue: Motors don't move
**Solution:** Verify pin connections match code definitions

### Issue: Erratic LDR readings
**Solution:** Wokwi LDRs need resistors - check diagram.json connections

### Issue: Code won't compile
**Solution:** Ensure libraries are installed via Library Manager

## Notes

- **Browser compatibility:** Works best in Chrome/Edge
- **Performance:** Complex simulations may run slower than real hardware
- **Limitations:** Some timing behaviors differ from physical Arduino
- **Serial Monitor:** Essential for debugging - always keep open during tests

## Documentation

For complete Q7 documentation with test results and analysis, see:
- `docs/Q7-Simulation-Testing-Results.md`

---

**Created:** October 17, 2025  
**For:** FEE 361 Q7 - Circuit Simulation  
**Platform:** Wokwi Web Simulator (wokwi.com)

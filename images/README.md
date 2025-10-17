# Project Images

This directory contains visual documentation for the Dual-Axis Solar Tracking System project.

## Q7 Simulation Screenshots

The `/q7-simulation/` subdirectory should contain 5 screenshots from the Wokwi simulation demonstrating different test scenarios:

### Required Screenshots:

1. **test1-tracking-balanced.png** - Tracking mode with balanced sensors
   - Shows normal operation with Az:90° El:45°
   - All LDRs reading ~250 (balanced)
   - State: TRACKING

2. **test2-west-tracking.png** - West direction tracking
   - Shows azimuth adjustment (Az:55°)
   - Error correction in action (EW=-66)
   - Servo moving to track sun

3. **test3-high-temp-protection.png** - High temperature protection
   - STOW MODE triggered at 55.4°C
   - Motors moved to safe position (Az:0° El:0°)
   - LCD shows "HIGH TEMP!"

4. **test4-manual-control.png** - Manual control mode
   - MANUAL CTRL active
   - Button control enabled
   - Manual override functional

5. **test5-night-mode.png** - Night mode detection
   - STOW MODE due to darkness
   - LDR readings ~50 (too dark)
   - System in safe position

## Adding Screenshots

To add the screenshots to this project:

1. Download/capture the 5 screenshots from Wokwi simulation
2. Save them with the filenames listed above
3. Place them in `/images/q7-simulation/` directory
4. Commit them to git:
   ```bash
   git add images/q7-simulation/*.png
   git commit -m "docs: Add Q7 simulation test screenshots"
   git push origin main
   ```

## Image Guidelines

- Format: PNG (preferred for screenshots)
- Resolution: As captured from Wokwi (typically 1920x1080 or similar)
- File size: Keep under 500KB per image if possible
- Naming: Use descriptive kebab-case names

---

**Note:** Screenshots are referenced in `/docs/Q7-Simulation-Testing-Results.md`

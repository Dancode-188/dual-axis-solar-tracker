/*
 * =============================================================================
 * DUAL-AXIS SOLAR TRACKING SYSTEM WITH WEATHER SENSOR
 * =============================================================================
 * 
 * Project:      FEE 361 - Sensors and Measurements (Project 91)
 * Institution:  University of Nairobi
 * Department:   Electrical and Information Engineering
 * Academic Year: 2025/2026
 * 
 * Description:
 * Intelligent dual-axis solar tracking system that automatically adjusts
 * solar panel orientation to maximize energy capture. Features include:
 * - Dual-axis tracking (azimuth 0-360°, elevation 0-90°)
 * - 4-LDR sensor array for sun position detection
 * - Weather protection (rain and temperature monitoring)
 * - Real-time LCD display with system status
 * - Manual override and control buttons
 * - State machine architecture for robust operation
 * 
 * Hardware:
 * - Arduino Mega 2560 (ATmega2560 microcontroller)
 * - 4x LDR sensors (light-dependent resistors)
 * - DHT22 temperature/humidity sensor
 * - Rain sensor (analog)
 * - Servo motor (MG996R) - azimuth control
 * - Stepper motor (NEMA 17 + A4988 driver) - elevation control
 * - 16x2 LCD display (I2C interface)
 * - Push buttons for manual control
 * 
 * Author:       Hilary Audi
 * Created:      October 17, 2025
 * Version:      1.0.0
 * 
 * License:      MIT License
 * Repository:   https://github.com/Dancode-188/dual-axis-solar-tracker
 * 
 * =============================================================================
 */

// =============================================================================
// LIBRARY INCLUDES
// =============================================================================

#include <Wire.h>                    // I2C communication for LCD
#include <LiquidCrystal_I2C.h>       // LCD display library
#include <Servo.h>                   // Servo motor control
#include <DHT.h>                     // DHT22 temperature/humidity sensor
#include <Stepper.h>                 // Stepper motor control (alternative)

// =============================================================================
// PIN DEFINITIONS
// =============================================================================

// LDR Sensors (Analog Inputs) - Light Detection
#define LDR_NE        A0    // North-East LDR
#define LDR_NW        A1    // North-West LDR
#define LDR_SE        A2    // South-East LDR
#define LDR_SW        A3    // South-West LDR

// Weather Sensors
#define RAIN_SENSOR   A4    // Rain sensor (analog)
#define DHT_PIN       7     // DHT22 data pin (digital)

// Motor Control Pins
#define SERVO_PIN     9     // Servo motor PWM pin (azimuth control)
#define STEP_PIN      3     // Stepper STEP pin
#define DIR_PIN       4     // Stepper DIR (direction) pin
#define ENABLE_PIN    5     // Stepper ENABLE pin (optional)

// Button Inputs (with internal pull-up resistors)
#define BTN_MANUAL    2     // Manual mode toggle button
#define BTN_RESET     8     // System reset button

// Status LEDs
#define LED_TRACKING  13    // Tracking mode indicator (built-in LED)
#define LED_STOW      12    // Stow mode indicator
#define LED_ERROR     11    // Error indicator

// =============================================================================
// CONSTANTS AND CONFIGURATION
// =============================================================================

// DHT Sensor Configuration
#define DHT_TYPE      DHT22         // DHT22 (AM2302) sensor type

// Tracking Thresholds
#define LDR_THRESHOLD      50       // Minimum LDR difference to trigger adjustment
#define DARK_THRESHOLD     100      // Total light below this = nighttime
#define TRACKING_STEP      5        // Degrees to move per adjustment

// Motor Limits
#define AZIMUTH_MIN        0        // Minimum azimuth angle (degrees)
#define AZIMUTH_MAX        180      // Maximum azimuth angle (servo limit)
#define ELEVATION_MIN      0        // Minimum elevation angle (degrees)
#define ELEVATION_MAX      90       // Maximum elevation angle (degrees)

// Home Position (Default/Stow)
#define HOME_AZIMUTH       90       // Home azimuth position (facing east)
#define HOME_ELEVATION     45       // Home elevation position (45° tilt)
#define STOW_AZIMUTH       0        // Stow azimuth (safe position)
#define STOW_ELEVATION     0        // Stow elevation (horizontal)

// Weather Protection Thresholds
#define RAIN_THRESHOLD     300      // Analog value below this = rain detected
#define TEMP_MAX           50.0     // Maximum safe temperature (°C)
#define TEMP_MIN           -10.0    // Minimum safe temperature (°C)

// Timing Constants
#define LOOP_DELAY         1000     // Main loop delay (milliseconds)
#define SERVO_SETTLE_TIME  500      // Time for servo to reach position (ms)
#define STEPPER_SPEED      200      // Stepper motor speed (RPM)
#define WEATHER_CHECK_INT  5000     // Weather check interval (ms)

// Stepper Motor Configuration
#define STEPS_PER_REV      200      // Steps per revolution (1.8° per step)
#define MICRO_STEPS        1        // Microstepping factor (1, 2, 4, 8, 16)
#define TOTAL_STEPS        (STEPS_PER_REV * MICRO_STEPS)

// LCD Configuration
#define LCD_ADDRESS        0x27     // I2C address for LCD
#define LCD_COLS           16       // LCD columns
#define LCD_ROWS           2        // LCD rows

// =============================================================================
// SYSTEM STATE DEFINITIONS
// =============================================================================

enum SystemState {
  STATE_INIT,           // Initialization state
  STATE_SEARCHING,      // Searching for sun
  STATE_TRACKING,       // Active sun tracking
  STATE_STOW,           // Weather protection stow mode
  STATE_MANUAL,         // Manual control mode
  STATE_ERROR           // Error state
};

// =============================================================================
// GLOBAL OBJECTS
// =============================================================================

// Initialize LCD (I2C address, columns, rows)
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);

// Initialize Servo for azimuth control
Servo azimuthServo;

// Initialize DHT sensor
DHT dht(DHT_PIN, DHT_TYPE);

// =============================================================================
// GLOBAL VARIABLES
// =============================================================================

// System State
SystemState currentState = STATE_INIT;
SystemState previousState = STATE_INIT;

// Current Position
int currentAzimuth = HOME_AZIMUTH;      // Current azimuth angle (0-180°)
int currentElevation = HOME_ELEVATION;   // Current elevation angle (0-90°)

// Target Position
int targetAzimuth = HOME_AZIMUTH;
int targetElevation = HOME_ELEVATION;

// LDR Sensor Readings (0-1023)
int ldrNE = 0;    // North-East
int ldrNW = 0;    // North-West
int ldrSE = 0;    // South-East
int ldrSW = 0;    // South-West

// LDR Averages for tracking decisions
int avgEast = 0;   // Average of NE + SE
int avgWest = 0;   // Average of NW + SW
int avgNorth = 0;  // Average of NE + NW
int avgSouth = 0;  // Average of SE + SW
int avgTotal = 0;  // Total average light level

// Weather Sensor Readings
float temperature = 0.0;    // Temperature in Celsius
float humidity = 0.0;       // Humidity in percentage
int rainValue = 1023;       // Rain sensor value (lower = more rain)
bool isRaining = false;     // Rain detected flag
bool overTemp = false;      // Over-temperature flag

// Button States (for debouncing)
bool manualButtonState = false;
bool manualButtonLastState = false;
bool resetButtonState = false;
bool resetButtonLastState = false;
unsigned long lastManualPress = 0;
unsigned long lastResetPress = 0;
#define DEBOUNCE_DELAY 200  // Button debounce time (ms)

// Timing Variables
unsigned long lastWeatherCheck = 0;
unsigned long lastLCDUpdate = 0;
unsigned long loopStartTime = 0;

// Error Handling
String errorMessage = "";
bool systemError = false;

// =============================================================================
// FUNCTION PROTOTYPES
// =============================================================================

// Setup and initialization
void initializeLCD();
void initializeMotors();
void initializeSensors();
void moveToHomePosition();

// Sensor reading functions
void readLDRSensors();
void readWeatherSensors();
void checkButtons();

// Control algorithms
void updateTracking();
void calculateTrackingError(int &errorEW, int &errorNS);
void adjustAzimuth(int error);
void adjustElevation(int error);

// Weather protection
void checkWeatherConditions();
void executeStow();

// Motor control
void moveServo(int angle);
void moveStepper(int targetAngle);
int calculateStepperSteps(int currentAngle, int targetAngle);

// State machine
void handleStateInit();
void handleStateSearching();
void handleStateTracking();
void handleStateStow();
void handleStateManual();
void handleStateError();

// User interface
void updateLCD();
void updateStatusLEDs();
void displayWelcomeMessage();
void displayError(String message);

// Utility functions
int constrainAngle(int angle, int minAngle, int maxAngle);
void serialDebug(String message);

// =============================================================================
// SETUP FUNCTION - RUNS ONCE AT STARTUP
// =============================================================================

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(9600);
  serialDebug("=== SOLAR TRACKER INITIALIZING ===");
  
  // Small delay for system stability
  delay(2000);
  
  // Initialize LCD display
  initializeLCD();
  displayWelcomeMessage();
  
  // Configure input pins
  serialDebug("Configuring input pins...");
  pinMode(LDR_NE, INPUT);
  pinMode(LDR_NW, INPUT);
  pinMode(LDR_SE, INPUT);
  pinMode(LDR_SW, INPUT);
  pinMode(RAIN_SENSOR, INPUT);
  pinMode(BTN_MANUAL, INPUT_PULLUP);  // Internal pull-up resistor
  pinMode(BTN_RESET, INPUT_PULLUP);   // Internal pull-up resistor
  
  // Configure output pins
  serialDebug("Configuring output pins...");
  pinMode(LED_TRACKING, OUTPUT);
  pinMode(LED_STOW, OUTPUT);
  pinMode(LED_ERROR, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);
  
  // Initialize all LEDs to OFF
  digitalWrite(LED_TRACKING, LOW);
  digitalWrite(LED_STOW, LOW);
  digitalWrite(LED_ERROR, LOW);
  digitalWrite(ENABLE_PIN, LOW);  // Enable stepper driver
  
  // Initialize motors
  initializeMotors();
  
  // Initialize sensors
  initializeSensors();
  
  // Move to home position
  serialDebug("Moving to home position...");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Moving to HOME");
  moveToHomePosition();
  
  // Initial sensor readings
  serialDebug("Reading initial sensor values...");
  readLDRSensors();
  readWeatherSensors();
  
  // Set initial state
  currentState = STATE_SEARCHING;
  serialDebug("Initialization complete!");
  serialDebug("State: SEARCHING");
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System READY");
  lcd.setCursor(0, 1);
  lcd.print("Searching...");
  delay(2000);
}


// =============================================================================
// MAIN LOOP FUNCTION - RUNS CONTINUOUSLY
// =============================================================================

void loop() {
  loopStartTime = millis();
  
  // Read all sensors
  readLDRSensors();
  checkButtons();
  
  // Periodic weather check (every 5 seconds)
  if (millis() - lastWeatherCheck >= WEATHER_CHECK_INT) {
    readWeatherSensors();
    checkWeatherConditions();
    lastWeatherCheck = millis();
  }
  
  // Execute current state behavior
  switch (currentState) {
    case STATE_INIT:
      handleStateInit();
      break;
      
    case STATE_SEARCHING:
      handleStateSearching();
      break;
      
    case STATE_TRACKING:
      handleStateTracking();
      break;
      
    case STATE_STOW:
      handleStateStow();
      break;
      
    case STATE_MANUAL:
      handleStateManual();
      break;
      
    case STATE_ERROR:
      handleStateError();
      break;
  }
  
  // Update user interface
  updateLCD();
  updateStatusLEDs();
  
  // Serial debugging output
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == 'd') {  // 'd' for debug info
      serialDebug("--- DEBUG INFO ---");
      Serial.print("State: ");
      Serial.println(currentState);
      Serial.print("Position - Az: ");
      Serial.print(currentAzimuth);
      Serial.print("° El: ");
      Serial.print(currentElevation);
      Serial.println("°");
      Serial.print("LDR - NE:");
      Serial.print(ldrNE);
      Serial.print(" NW:");
      Serial.print(ldrNW);
      Serial.print(" SE:");
      Serial.print(ldrSE);
      Serial.print(" SW:");
      Serial.println(ldrSW);
      Serial.print("Weather - Temp:");
      Serial.print(temperature);
      Serial.print("°C Humidity:");
      Serial.print(humidity);
      Serial.print("% Rain:");
      Serial.println(rainValue);
    }
  }
  
  // Maintain consistent loop timing
  unsigned long loopTime = millis() - loopStartTime;
  if (loopTime < LOOP_DELAY) {
    delay(LOOP_DELAY - loopTime);
  }
}

// =============================================================================
// INITIALIZATION FUNCTIONS
// =============================================================================

/**
 * Initialize LCD display
 */
void initializeLCD() {
  serialDebug("Initializing LCD...");
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  // Test if LCD is responding
  lcd.setCursor(0, 0);
  lcd.print("LCD Init...");
  delay(1000);
  
  serialDebug("LCD initialized successfully");
}

/**
 * Initialize motor systems
 */
void initializeMotors() {
  serialDebug("Initializing motors...");
  
  // Attach servo to pin
  azimuthServo.attach(SERVO_PIN);
  
  // Set initial servo position
  azimuthServo.write(HOME_AZIMUTH);
  delay(500);
  
  // Initialize stepper (already configured with pinMode)
  digitalWrite(ENABLE_PIN, LOW);  // Enable driver
  
  serialDebug("Motors initialized successfully");
}

/**
 * Initialize sensors
 */
void initializeSensors() {
  serialDebug("Initializing sensors...");
  
  // Initialize DHT22 sensor
  dht.begin();
  delay(2000);  // DHT22 needs time to stabilize
  
  // Test read
  float testTemp = dht.readTemperature();
  if (isnan(testTemp)) {
    serialDebug("WARNING: DHT22 sensor not responding!");
  } else {
    serialDebug("DHT22 initialized successfully");
  }
  
  serialDebug("Sensors initialized successfully");
}

/**
 * Move system to home position
 */
void moveToHomePosition() {
  serialDebug("Moving to home position...");
  
  // Move azimuth to home
  moveServo(HOME_AZIMUTH);
  currentAzimuth = HOME_AZIMUTH;
  
  // Move elevation to home
  moveStepper(HOME_ELEVATION);
  currentElevation = HOME_ELEVATION;
  
  delay(1000);  // Allow time for positioning
  serialDebug("Home position reached");
}


/**
 * Display welcome message on LCD
 */
void displayWelcomeMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Solar Tracker");
  lcd.setCursor(0, 1);
  lcd.print("v1.0 - Init...");
  delay(2000);
}

/**
 * Display error message on LCD
 */
void displayError(String message) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ERROR!");
  lcd.setCursor(0, 1);
  lcd.print(message.substring(0, 16));  // Limit to LCD width
}

/**
 * Serial debugging helper function
 */
void serialDebug(String message) {
  if (Serial) {
    Serial.print("[");
    Serial.print(millis());
    Serial.print("ms] ");
    Serial.println(message);
  }
}

// =============================================================================
// END OF PHASE 1: PROJECT FOUNDATION
// Next: Phase 2 - Sensor Reading Functions
// =============================================================================


// =============================================================================
// PHASE 2: SENSOR READING FUNCTIONS
// =============================================================================

/**
 * Read all four LDR sensors and calculate averages
 * LDR Configuration:
 *   NE (North-East) - Top-right quadrant
 *   NW (North-West) - Top-left quadrant  
 *   SE (South-East) - Bottom-right quadrant
 *   SW (South-West) - Bottom-left quadrant
 */
void readLDRSensors() {
  // Read raw analog values (0-1023)
  ldrNE = analogRead(LDR_NE);
  ldrNW = analogRead(LDR_NW);
  ldrSE = analogRead(LDR_SE);
  ldrSW = analogRead(LDR_SW);
  
  // Calculate directional averages for tracking decisions
  avgEast = (ldrNE + ldrSE) / 2;   // Average of east sensors
  avgWest = (ldrNW + ldrSW) / 2;   // Average of west sensors
  avgNorth = (ldrNE + ldrNW) / 2;  // Average of north sensors
  avgSouth = (ldrSE + ldrSW) / 2;  // Average of south sensors
  
  // Calculate total average light level
  avgTotal = (ldrNE + ldrNW + ldrSE + ldrSW) / 4;
}

/**
 * Read weather sensors (DHT22 and rain sensor)
 * Includes error handling for failed sensor reads
 */
void readWeatherSensors() {
  // Read DHT22 temperature and humidity
  float tempReading = dht.readTemperature();
  float humidityReading = dht.readHumidity();
  
  // Validate DHT22 readings
  if (!isnan(tempReading) && !isnan(humidityReading)) {
    temperature = tempReading;
    humidity = humidityReading;
  } else {
    serialDebug("WARNING: DHT22 read failed, using previous values");
    // Keep previous values, sensor might recover
  }
  
  // Read rain sensor (lower value = more moisture)
  rainValue = analogRead(RAIN_SENSOR);
  
  // Update rain detection flag
  if (rainValue < RAIN_THRESHOLD) {
    isRaining = true;
  } else {
    isRaining = false;
  }
  
  // Update temperature flag
  if (temperature > TEMP_MAX || temperature < TEMP_MIN) {
    overTemp = true;
  } else {
    overTemp = false;
  }
}

/**
 * Check button states with debouncing
 * Buttons use internal pull-up resistors (pressed = LOW)
 */
void checkButtons() {
  // Read current button states (inverted because of pull-up)
  manualButtonState = !digitalRead(BTN_MANUAL);
  resetButtonState = !digitalRead(BTN_RESET);
  
  // Manual mode toggle button
  if (manualButtonState && !manualButtonLastState) {
    // Button was just pressed
    if (millis() - lastManualPress > DEBOUNCE_DELAY) {
      // Toggle between MANUAL and previous state
      if (currentState == STATE_MANUAL) {
        serialDebug("Manual mode OFF - returning to previous state");
        currentState = previousState;
        digitalWrite(LED_TRACKING, HIGH);
      } else {
        serialDebug("Manual mode ON");
        previousState = currentState;
        currentState = STATE_MANUAL;
        digitalWrite(LED_TRACKING, LOW);
      }
      lastManualPress = millis();
    }
  }
  manualButtonLastState = manualButtonState;
  
  // Reset button
  if (resetButtonState && !resetButtonLastState) {
    // Button was just pressed
    if (millis() - lastResetPress > DEBOUNCE_DELAY) {
      serialDebug("RESET button pressed - returning to SEARCHING state");
      systemError = false;
      errorMessage = "";
      currentState = STATE_SEARCHING;
      digitalWrite(LED_ERROR, LOW);
      lastResetPress = millis();
    }
  }
  resetButtonLastState = resetButtonState;
}


// =============================================================================
// PHASE 3: CORE TRACKING ALGORITHM & WEATHER PROTECTION
// =============================================================================

/**
 * Calculate tracking errors from LDR readings
 * @param errorEW - Output: East-West error (positive = east brighter)
 * @param errorNS - Output: North-South error (positive = north brighter)
 */
void calculateTrackingError(int &errorEW, int &errorNS) {
  // East-West error: positive = turn east, negative = turn west
  errorEW = avgEast - avgWest;
  
  // North-South error: positive = tilt up (north), negative = tilt down (south)
  errorNS = avgNorth - avgSouth;
  
  // Debug output
  Serial.print("Tracking Error - EW: ");
  Serial.print(errorEW);
  Serial.print(" NS: ");
  Serial.println(errorNS);
}

/**
 * Adjust azimuth (horizontal) position based on East-West error
 * @param error - Tracking error (positive = move east, negative = move west)
 */
void adjustAzimuth(int error) {
  if (abs(error) < LDR_THRESHOLD) {
    // Error too small, no adjustment needed
    return;
  }
  
  // Determine direction and calculate new position
  if (error > 0) {
    // East is brighter, move east (increase azimuth)
    targetAzimuth = currentAzimuth + TRACKING_STEP;
    serialDebug("Adjusting azimuth EAST");
  } else {
    // West is brighter, move west (decrease azimuth)
    targetAzimuth = currentAzimuth - TRACKING_STEP;
    serialDebug("Adjusting azimuth WEST");
  }
  
  // Constrain to valid range
  targetAzimuth = constrainAngle(targetAzimuth, AZIMUTH_MIN, AZIMUTH_MAX);
  
  // Move servo to new position
  moveServo(targetAzimuth);
  currentAzimuth = targetAzimuth;
}

/**
 * Adjust elevation (vertical) position based on North-South error
 * @param error - Tracking error (positive = tilt up, negative = tilt down)
 */
void adjustElevation(int error) {
  if (abs(error) < LDR_THRESHOLD) {
    // Error too small, no adjustment needed
    return;
  }
  
  // Determine direction and calculate new position
  if (error > 0) {
    // North is brighter, tilt up (increase elevation)
    targetElevation = currentElevation + TRACKING_STEP;
    serialDebug("Adjusting elevation UP");
  } else {
    // South is brighter, tilt down (decrease elevation)
    targetElevation = currentElevation - TRACKING_STEP;
    serialDebug("Adjusting elevation DOWN");
  }
  
  // Constrain to valid range
  targetElevation = constrainAngle(targetElevation, ELEVATION_MIN, ELEVATION_MAX);
  
  // Move stepper to new position
  moveStepper(targetElevation);
  currentElevation = targetElevation;
}

/**
 * Check weather conditions and trigger stow if necessary
 * Weather protection is highest priority
 */
void checkWeatherConditions() {
  bool needsStow = false;
  String stowReason = "";
  
  // Check for rain
  if (isRaining) {
    needsStow = true;
    stowReason = "RAIN DETECTED";
    serialDebug("Weather alert: Rain detected!");
  }
  
  // Check temperature limits
  if (overTemp) {
    needsStow = true;
    if (temperature > TEMP_MAX) {
      stowReason = "HIGH TEMP";
      serialDebug("Weather alert: Temperature too high!");
    } else {
      stowReason = "LOW TEMP";
      serialDebug("Weather alert: Temperature too low!");
    }
  }
  
  // Execute stow if needed
  if (needsStow && currentState != STATE_STOW) {
    serialDebug("EMERGENCY STOW INITIATED: " + stowReason);
    errorMessage = stowReason;
    previousState = currentState;
    currentState = STATE_STOW;
    executeStow();
  }
  
  // Check if weather has cleared
  if (!needsStow && currentState == STATE_STOW) {
    // Weather is clear but we're still in stow
    // Wait a bit before resuming to ensure stability
    static unsigned long stowClearTime = 0;
    if (stowClearTime == 0) {
      stowClearTime = millis();
    }
    
    if (millis() - stowClearTime > 30000) {  // 30 seconds clear weather
      serialDebug("Weather cleared - resuming tracking");
      currentState = STATE_SEARCHING;
      stowClearTime = 0;
    }
  } else if (needsStow) {
    // Reset clear time if weather is still bad
    static unsigned long stowClearTime = 0;
    stowClearTime = 0;
  }
}

/**
 * Execute emergency stow procedure
 * Moves panel to safe horizontal position
 */
void executeStow() {
  serialDebug("Executing STOW procedure...");
  
  // Turn on stow LED
  digitalWrite(LED_STOW, HIGH);
  digitalWrite(LED_TRACKING, LOW);
  
  // Move to stow position (horizontal and facing forward)
  moveServo(STOW_AZIMUTH);
  currentAzimuth = STOW_AZIMUTH;
  
  moveStepper(STOW_ELEVATION);
  currentElevation = STOW_ELEVATION;
  
  serialDebug("STOW position reached");
}


// =============================================================================
// PHASE 4: MOTOR CONTROL & STATE MACHINE
// =============================================================================

/**
 * Move servo motor to specified angle
 * @param angle - Target angle in degrees (0-180)
 */
void moveServo(int angle) {
  // Constrain angle to servo limits
  angle = constrainAngle(angle, AZIMUTH_MIN, AZIMUTH_MAX);
  
  // Write angle to servo
  azimuthServo.write(angle);
  
  // Allow time for servo to reach position
  delay(SERVO_SETTLE_TIME);
  
  Serial.print("Servo moved to: ");
  Serial.print(angle);
  Serial.println("°");
}

/**
 * Move stepper motor to specified angle
 * @param targetAngle - Target elevation angle (0-90 degrees)
 */
void moveStepper(int targetAngle) {
  // Constrain angle to valid range
  targetAngle = constrainAngle(targetAngle, ELEVATION_MIN, ELEVATION_MAX);
  
  // Calculate steps needed
  int steps = calculateStepperSteps(currentElevation, targetAngle);
  
  if (steps == 0) {
    return;  // Already at target position
  }
  
  // Set direction
  if (steps > 0) {
    digitalWrite(DIR_PIN, HIGH);  // Move up
  } else {
    digitalWrite(DIR_PIN, LOW);   // Move down
    steps = -steps;  // Make steps positive for loop
  }
  
  // Step the motor
  for (int i = 0; i < steps; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(500);  // Pulse width
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(500);  // Step delay (controls speed)
  }
  
  Serial.print("Stepper moved to: ");
  Serial.print(targetAngle);
  Serial.println("°");
}

/**
 * Calculate number of steps needed for stepper motor
 * @param currentAngle - Current elevation angle
 * @param targetAngle - Desired elevation angle
 * @return Number of steps (positive = up, negative = down)
 */
int calculateStepperSteps(int currentAngle, int targetAngle) {
  int angleDifference = targetAngle - currentAngle;
  
  // Calculate steps: (angle_difference / 360°) * steps_per_revolution
  // For 200 steps/rev: 200/360 = 0.555 steps per degree
  int steps = (angleDifference * TOTAL_STEPS) / 360;
  
  return steps;
}

/**
 * Constrain angle to specified range
 */
int constrainAngle(int angle, int minAngle, int maxAngle) {
  if (angle < minAngle) return minAngle;
  if (angle > maxAngle) return maxAngle;
  return angle;
}

// =============================================================================
// STATE MACHINE HANDLERS
// =============================================================================

/**
 * Handle INIT state - should not normally be called
 */
void handleStateInit() {
  // This state is only used during setup()
  // If we're here in the main loop, something went wrong
  serialDebug("WARNING: In INIT state during main loop");
  currentState = STATE_SEARCHING;
}

/**
 * Handle SEARCHING state - scan for sun
 */
void handleStateSearching() {
  // Check if it's too dark (nighttime)
  if (avgTotal < DARK_THRESHOLD) {
    serialDebug("Too dark for tracking - entering STOW mode");
    currentState = STATE_STOW;
    executeStow();
    return;
  }
  
  // If we have sufficient light, start tracking
  if (avgTotal > DARK_THRESHOLD) {
    serialDebug("Sun detected - entering TRACKING mode");
    currentState = STATE_TRACKING;
    digitalWrite(LED_TRACKING, HIGH);
    return;
  }
  
  // Perform slow sweep to find sun
  static unsigned long lastSweepMove = 0;
  if (millis() - lastSweepMove > 2000) {  // Move every 2 seconds
    currentAzimuth += 10;
    if (currentAzimuth > AZIMUTH_MAX) {
      currentAzimuth = AZIMUTH_MIN;
    }
    moveServo(currentAzimuth);
    lastSweepMove = millis();
  }
}

/**
 * Handle TRACKING state - active sun tracking
 */
void handleStateTracking() {
  // Check if it's too dark
  if (avgTotal < DARK_THRESHOLD) {
    serialDebug("Lost sun - entering SEARCHING mode");
    currentState = STATE_SEARCHING;
    digitalWrite(LED_TRACKING, LOW);
    return;
  }
  
  // Calculate tracking errors
  int errorEW = 0;
  int errorNS = 0;
  calculateTrackingError(errorEW, errorNS);
  
  // Adjust azimuth based on east-west error
  if (abs(errorEW) > LDR_THRESHOLD) {
    adjustAzimuth(errorEW);
  }
  
  // Adjust elevation based on north-south error
  if (abs(errorNS) > LDR_THRESHOLD) {
    adjustElevation(errorNS);
  }
  
  // If errors are small, panel is well-aligned
  if (abs(errorEW) < LDR_THRESHOLD && abs(errorNS) < LDR_THRESHOLD) {
    // Panel is optimally positioned
    // Could add efficiency logging here
  }
}


/**
 * Handle STOW state - weather protection mode
 */
void handleStateStow() {
  // Keep panel in safe position
  // Weather check happens in main loop
  // State will transition back to SEARCHING when weather clears
  
  // Ensure we're at stow position
  if (currentAzimuth != STOW_AZIMUTH || currentElevation != STOW_ELEVATION) {
    executeStow();
  }
  
  // Keep stow LED on
  digitalWrite(LED_STOW, HIGH);
  digitalWrite(LED_TRACKING, LOW);
}

/**
 * Handle MANUAL state - user control mode
 */
void handleStateManual() {
  // In manual mode, user can control position via Serial commands
  // or additional buttons could be added for manual jog
  
  // For now, maintain current position
  // Future enhancement: Add serial commands for manual positioning
  
  // Check for serial commands
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    
    switch (cmd) {
      case 'w':  // Tilt up
        if (currentElevation < ELEVATION_MAX) {
          currentElevation += 5;
          moveStepper(currentElevation);
          serialDebug("Manual: Elevation increased");
        }
        break;
        
      case 's':  // Tilt down
        if (currentElevation > ELEVATION_MIN) {
          currentElevation -= 5;
          moveStepper(currentElevation);
          serialDebug("Manual: Elevation decreased");
        }
        break;
        
      case 'a':  // Turn left (west)
        if (currentAzimuth > AZIMUTH_MIN) {
          currentAzimuth -= 5;
          moveServo(currentAzimuth);
          serialDebug("Manual: Azimuth decreased");
        }
        break;
        
      case 'd':  // Turn right (east)
        if (currentAzimuth < AZIMUTH_MAX) {
          currentAzimuth += 5;
          moveServo(currentAzimuth);
          serialDebug("Manual: Azimuth increased");
        }
        break;
        
      case 'h':  // Return to home
        serialDebug("Manual: Returning to home position");
        moveToHomePosition();
        break;
    }
  }
}

/**
 * Handle ERROR state - system error
 */
void handleStateError() {
  // Flash error LED
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 500) {
    digitalWrite(LED_ERROR, !digitalRead(LED_ERROR));
    lastBlink = millis();
  }
  
  // Turn off other LEDs
  digitalWrite(LED_TRACKING, LOW);
  digitalWrite(LED_STOW, LOW);
  
  // Display error on LCD
  displayError(errorMessage);
  
  // Wait for reset button
  // State will be changed by button handler
}


// =============================================================================
// PHASE 5: USER INTERFACE & DISPLAY
// =============================================================================

/**
 * Update LCD display with current system status
 * Display format:
 * Line 1: State and position
 * Line 2: Weather or tracking info
 */
void updateLCD() {
  // Update LCD every second to avoid flicker
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate < 1000) {
    return;
  }
  lastUpdate = millis();
  
  lcd.clear();
  lcd.setCursor(0, 0);
  
  // Display current state and position
  switch (currentState) {
    case STATE_INIT:
      lcd.print("Initializing...");
      break;
      
    case STATE_SEARCHING:
      lcd.print("SEARCHING");
      lcd.setCursor(0, 1);
      lcd.print("Az:");
      lcd.print(currentAzimuth);
      lcd.print(" El:");
      lcd.print(currentElevation);
      break;
      
    case STATE_TRACKING:
      lcd.print("TRACKING");
      lcd.setCursor(0, 1);
      lcd.print("Az:");
      lcd.print(currentAzimuth);
      lcd.print(" El:");
      lcd.print(currentElevation);
      break;
      
    case STATE_STOW:
      lcd.print("STOW MODE");
      lcd.setCursor(0, 1);
      if (isRaining) {
        lcd.print("Rain detected");
      } else if (overTemp) {
        lcd.print("Temp:");
        lcd.print((int)temperature);
        lcd.print("C");
      } else {
        lcd.print("Weather safe");
      }
      break;
      
    case STATE_MANUAL:
      lcd.print("MANUAL MODE");
      lcd.setCursor(0, 1);
      lcd.print("Az:");
      lcd.print(currentAzimuth);
      lcd.print(" El:");
      lcd.print(currentElevation);
      break;
      
    case STATE_ERROR:
      // Error display is handled by displayError() function
      break;
  }
}

/**
 * Update status LED indicators
 */
void updateStatusLEDs() {
  switch (currentState) {
    case STATE_TRACKING:
      digitalWrite(LED_TRACKING, HIGH);
      digitalWrite(LED_STOW, LOW);
      digitalWrite(LED_ERROR, LOW);
      break;
      
    case STATE_STOW:
      digitalWrite(LED_TRACKING, LOW);
      digitalWrite(LED_STOW, HIGH);
      digitalWrite(LED_ERROR, LOW);
      break;
      
    case STATE_ERROR:
      digitalWrite(LED_TRACKING, LOW);
      digitalWrite(LED_STOW, LOW);
      // Error LED is flashed in handleStateError()
      break;
      
    case STATE_SEARCHING:
      digitalWrite(LED_TRACKING, LOW);
      digitalWrite(LED_STOW, LOW);
      digitalWrite(LED_ERROR, LOW);
      break;
      
    case STATE_MANUAL:
      digitalWrite(LED_TRACKING, LOW);
      digitalWrite(LED_STOW, LOW);
      digitalWrite(LED_ERROR, LOW);
      break;
      
    default:
      digitalWrite(LED_TRACKING, LOW);
      digitalWrite(LED_STOW, LOW);
      digitalWrite(LED_ERROR, LOW);
      break;
  }
}

/**
 * Main tracking function (called from TRACKING state)
 * Integrates all tracking logic
 */
void updateTracking() {
  // This function encapsulates the tracking logic
  // and is called from handleStateTracking()
  
  int errorEW, errorNS;
  calculateTrackingError(errorEW, errorNS);
  
  // Adjust azimuth if needed
  if (abs(errorEW) > LDR_THRESHOLD) {
    adjustAzimuth(errorEW);
  }
  
  // Adjust elevation if needed
  if (abs(errorNS) > LDR_THRESHOLD) {
    adjustElevation(errorNS);
  }
}

// =============================================================================
// END OF ARDUINO CODE
// =============================================================================

/*
 * DEPLOYMENT NOTES:
 * 
 * 1. Hardware Setup:
 *    - Connect all sensors according to pin definitions above
 *    - Ensure power supply is adequate (12V recommended)
 *    - Check all connections before powering on
 * 
 * 2. Library Requirements:
 *    - Wire (built-in)
 *    - LiquidCrystal_I2C (install via Library Manager)
 *    - Servo (built-in)
 *    - DHT sensor library by Adafruit (install via Library Manager)
 *    - Stepper (built-in)
 * 
 * 3. Calibration:
 *    - Adjust LDR_THRESHOLD if tracking is too sensitive/insensitive
 *    - Adjust DARK_THRESHOLD for nighttime detection
 *    - Adjust TRACKING_STEP for faster/slower tracking response
 *    - Adjust RAIN_THRESHOLD based on rain sensor characteristics
 *    - Verify servo and stepper movement directions
 * 
 * 4. Testing:
 *    - Test manual mode first (press manual button)
 *    - Use serial monitor for debug output (9600 baud)
 *    - Verify all LEDs are functioning
 *    - Test weather protection by covering rain sensor
 *    - Verify tracking by varying light on LDRs
 * 
 * 5. Serial Commands (in MANUAL mode):
 *    - 'w' = Tilt up (increase elevation)
 *    - 's' = Tilt down (decrease elevation)
 *    - 'a' = Turn left/west (decrease azimuth)
 *    - 'd' = Turn right/east (increase azimuth)
 *    - 'h' = Return to home position
 *    - 'd' (in any mode) = Print debug information
 * 
 * 6. Troubleshooting:
 *    - If DHT22 fails, check wiring and 4.7kΩ pull-up resistor
 *    - If servo jitters, ensure adequate power supply
 *    - If stepper doesn't move, check A4988 driver connections
 *    - If LCD doesn't display, verify I2C address (0x27 or 0x3F)
 *    - If tracking is erratic, check LDR connections and lighting
 * 
 * Version: 1.0.0
 * Last Updated: October 17, 2025
 * Author: Hilary Audi
 * License: MIT
 */

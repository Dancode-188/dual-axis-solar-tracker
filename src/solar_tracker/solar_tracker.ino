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

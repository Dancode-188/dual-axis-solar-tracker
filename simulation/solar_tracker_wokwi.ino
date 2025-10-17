/*
 * =============================================================================
 * DUAL-AXIS SOLAR TRACKING SYSTEM - WOKWI SIMULATION VERSION
 * =============================================================================
 * 
 * Project:      FEE 361 - Sensors and Measurements (Project 91)
 * Institution:  University of Nairobi
 * Department:   Electrical and Information Engineering
 * 
 * Description:
 * Wokwi-optimized version of the dual-axis solar tracking system.
 * This version is specifically adapted for Wokwi web simulator while
 * maintaining all core functionality for Q7 simulation demonstration.
 * 
 * Author:       Hilary Audi
 * Created:      October 17, 2025
 * Version:      1.0.0 (Wokwi)
 * 
 * =============================================================================
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <DHT.h>

// =============================================================================
// PIN DEFINITIONS
// =============================================================================

// LDR Sensors
#define LDR_NE        A0    // North-East
#define LDR_NW        A1    // North-West
#define LDR_SE        A2    // South-East
#define LDR_SW        A3    // South-West

// Weather Sensors
#define DHT_PIN       7     // DHT22 data pin
#define DHT_TYPE      DHT22

// Motor Control
#define SERVO_PIN     9     // Servo (azimuth)
#define STEP_PIN      3     // Stepper STEP
#define DIR_PIN       4     // Stepper DIR
#define ENABLE_PIN    5     // Stepper ENABLE

// Buttons
#define BTN_MANUAL    2
#define BTN_RESET     8

// LEDs
#define LED_TRACKING  13
#define LED_STOW      12
#define LED_ERROR     11

// =============================================================================
// CONSTANTS
// =============================================================================

#define LDR_THRESHOLD      50
#define DARK_THRESHOLD     100
#define TRACKING_STEP      5
#define AZIMUTH_MIN        0
#define AZIMUTH_MAX        180
#define ELEVATION_MIN      0
#define ELEVATION_MAX      90
#define HOME_AZIMUTH       90
#define HOME_ELEVATION     45
#define STOW_AZIMUTH       0
#define STOW_ELEVATION     0
#define TEMP_MAX           50.0
#define LOOP_DELAY         1000

// =============================================================================
// SYSTEM STATES
// =============================================================================

enum SystemState {
  STATE_INIT,
  STATE_SEARCHING,
  STATE_TRACKING,
  STATE_STOW,
  STATE_MANUAL,
  STATE_ERROR
};

// =============================================================================
// GLOBAL OBJECTS
// =============================================================================

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo azimuthServo;
DHT dht(DHT_PIN, DHT_TYPE);

// =============================================================================
// GLOBAL VARIABLES
// =============================================================================

SystemState currentState = STATE_INIT;
int currentAzimuth = HOME_AZIMUTH;
int currentElevation = HOME_ELEVATION;

// LDR readings
int ldrNE = 0, ldrNW = 0, ldrSE = 0, ldrSW = 0;
int avgEast = 0, avgWest = 0, avgNorth = 0, avgSouth = 0, avgTotal = 0;

// Weather
float temperature = 25.0;
float humidity = 50.0;
bool overTemp = false;

// Button states
bool manualMode = false;
unsigned long lastButtonPress = 0;

// Timing
unsigned long lastWeatherCheck = 0;
unsigned long loopStartTime = 0;

// =============================================================================
// SETUP
// =============================================================================

void setup() {
  Serial.begin(115200);
  Serial.println("=== SOLAR TRACKER INITIALIZING ===");
  
  // Configure pins
  pinMode(LDR_NE, INPUT);
  pinMode(LDR_NW, INPUT);
  pinMode(LDR_SE, INPUT);
  pinMode(LDR_SW, INPUT);
  pinMode(BTN_MANUAL, INPUT_PULLUP);
  pinMode(BTN_RESET, INPUT_PULLUP);
  pinMode(LED_TRACKING, OUTPUT);
  pinMode(LED_STOW, OUTPUT);
  pinMode(LED_ERROR, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);
  
  // Initialize
  digitalWrite(LED_TRACKING, LOW);
  digitalWrite(LED_STOW, LOW);
  digitalWrite(LED_ERROR, LOW);
  digitalWrite(ENABLE_PIN, LOW);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Solar Tracker");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  
  // Initialize motors
  azimuthServo.attach(SERVO_PIN);
  moveServo(HOME_AZIMUTH);
  
  // Initialize DHT
  dht.begin();
  
  delay(2000);
  
  // Read initial sensors
  readLDRSensors();
  readWeatherSensors();
  
  currentState = STATE_SEARCHING;
  Serial.println("Initialization complete - SEARCHING mode");
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System READY");
  lcd.setCursor(0, 1);
  lcd.print("Searching...");
  delay(1000);
}

// =============================================================================
// MAIN LOOP
// =============================================================================

void loop() {
  loopStartTime = millis();
  
  // Read sensors
  readLDRSensors();
  checkButtons();
  
  // Periodic weather check
  if (millis() - lastWeatherCheck >= 5000) {
    readWeatherSensors();
    checkWeatherConditions();
    lastWeatherCheck = millis();
  }
  
  // State machine
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
  
  // Update display
  updateLCD();
  updateStatusLEDs();
  
  delay(LOOP_DELAY);
}

// =============================================================================
// SENSOR READING FUNCTIONS
// =============================================================================

void readLDRSensors() {
  ldrNE = analogRead(LDR_NE);
  ldrNW = analogRead(LDR_NW);
  ldrSE = analogRead(LDR_SE);
  ldrSW = analogRead(LDR_SW);
  
  avgEast = (ldrNE + ldrSE) / 2;
  avgWest = (ldrNW + ldrSW) / 2;
  avgNorth = (ldrNE + ldrNW) / 2;
  avgSouth = (ldrSE + ldrSW) / 2;
  avgTotal = (ldrNE + ldrNW + ldrSE + ldrSW) / 4;
  
  Serial.print("LDR: NE="); Serial.print(ldrNE);
  Serial.print(" NW="); Serial.print(ldrNW);
  Serial.print(" SE="); Serial.print(ldrSE);
  Serial.print(" SW="); Serial.print(ldrSW);
  Serial.print(" | Avg="); Serial.println(avgTotal);
}

void readWeatherSensors() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  
  if (isnan(temperature)) temperature = 25.0;
  if (isnan(humidity)) humidity = 50.0;
  
  Serial.print("Weather: Temp="); Serial.print(temperature);
  Serial.print("C Humidity="); Serial.print(humidity); Serial.println("%");
}

void checkButtons() {
  if (digitalRead(BTN_MANUAL) == LOW && millis() - lastButtonPress > 200) {
    manualMode = !manualMode;
    currentState = manualMode ? STATE_MANUAL : STATE_TRACKING;
    lastButtonPress = millis();
    Serial.print("Manual mode: "); Serial.println(manualMode ? "ON" : "OFF");
  }
  
  if (digitalRead(BTN_RESET) == LOW && millis() - lastButtonPress > 200) {
    currentState = STATE_INIT;
    lastButtonPress = millis();
    Serial.println("System RESET");
  }
}

// =============================================================================
// WEATHER PROTECTION
// =============================================================================

void checkWeatherConditions() {
  overTemp = (temperature > TEMP_MAX);
  
  if (overTemp && currentState != STATE_STOW) {
    Serial.println("!!! HIGH TEMPERATURE - STOWING !!!");
    currentState = STATE_STOW;
    executeStow();
  }
}

void executeStow() {
  Serial.println("Executing STOW sequence...");
  moveServo(STOW_AZIMUTH);
  moveStepper(STOW_ELEVATION);
  currentAzimuth = STOW_AZIMUTH;
  currentElevation = STOW_ELEVATION;
}

// =============================================================================
// MOTOR CONTROL
// =============================================================================

void moveServo(int angle) {
  angle = constrain(angle, AZIMUTH_MIN, AZIMUTH_MAX);
  azimuthServo.write(angle);
  currentAzimuth = angle;
  Serial.print("Servo moved to: "); Serial.println(angle);
  delay(500);
}

void moveStepper(int targetAngle) {
  targetAngle = constrain(targetAngle, ELEVATION_MIN, ELEVATION_MAX);
  
  int delta = targetAngle - currentElevation;
  int steps = abs(delta) * 200 / 360;
  
  digitalWrite(DIR_PIN, delta > 0 ? HIGH : LOW);
  
  for (int i = 0; i < steps; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(1000);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(1000);
  }
  
  currentElevation = targetAngle;
  Serial.print("Stepper moved to: "); Serial.println(targetAngle);
}

// =============================================================================
// TRACKING ALGORITHMS
// =============================================================================

void calculateTrackingError(int &errorEW, int &errorNS) {
  errorEW = avgEast - avgWest;
  errorNS = avgNorth - avgSouth;
  
  Serial.print("Errors: EW="); Serial.print(errorEW);
  Serial.print(" NS="); Serial.println(errorNS);
}

void adjustAzimuth(int error) {
  int newAzimuth = currentAzimuth;
  
  if (error > LDR_THRESHOLD) {
    newAzimuth += TRACKING_STEP;
    Serial.println("Adjusting azimuth: Turn EAST");
  } else if (error < -LDR_THRESHOLD) {
    newAzimuth -= TRACKING_STEP;
    Serial.println("Adjusting azimuth: Turn WEST");
  }
  
  newAzimuth = constrain(newAzimuth, AZIMUTH_MIN, AZIMUTH_MAX);
  if (newAzimuth != currentAzimuth) {
    moveServo(newAzimuth);
  }
}

void adjustElevation(int error) {
  int newElevation = currentElevation;
  
  if (error > LDR_THRESHOLD) {
    newElevation += TRACKING_STEP;
    Serial.println("Adjusting elevation: Tilt UP");
  } else if (error < -LDR_THRESHOLD) {
    newElevation -= TRACKING_STEP;
    Serial.println("Adjusting elevation: Tilt DOWN");
  }
  
  newElevation = constrain(newElevation, ELEVATION_MIN, ELEVATION_MAX);
  if (newElevation != currentElevation) {
    moveStepper(newElevation);
  }
}

// =============================================================================
// STATE MACHINE HANDLERS
// =============================================================================

void handleStateInit() {
  Serial.println("State: INIT");
  moveServo(HOME_AZIMUTH);
  moveStepper(HOME_ELEVATION);
  currentState = STATE_SEARCHING;
}

void handleStateSearching() {
  Serial.println("State: SEARCHING");
  
  if (avgTotal < DARK_THRESHOLD) {
    Serial.println("Too dark - moving to STOW");
    currentState = STATE_STOW;
    executeStow();
    return;
  }
  
  int errorEW, errorNS;
  calculateTrackingError(errorEW, errorNS);
  
  if (abs(errorEW) > LDR_THRESHOLD || abs(errorNS) > LDR_THRESHOLD) {
    Serial.println("Sun found! Switching to TRACKING");
    currentState = STATE_TRACKING;
  }
}

void handleStateTracking() {
  Serial.println("State: TRACKING");
  
  if (avgTotal < DARK_THRESHOLD) {
    Serial.println("Too dark - moving to STOW");
    currentState = STATE_STOW;
    executeStow();
    return;
  }
  
  int errorEW, errorNS;
  calculateTrackingError(errorEW, errorNS);
  
  adjustAzimuth(errorEW);
  adjustElevation(errorNS);
}

void handleStateStow() {
  Serial.println("State: STOW");
  
  if (!overTemp && avgTotal > DARK_THRESHOLD) {
    static int clearCount = 0;
    clearCount++;
    if (clearCount > 3) {
      Serial.println("Conditions clear - resuming tracking");
      currentState = STATE_SEARCHING;
      clearCount = 0;
    }
  }
}

void handleStateManual() {
  Serial.println("State: MANUAL");
  
  if (Serial.available()) {
    char cmd = Serial.read();
    switch(cmd) {
      case 'w': case 'W':
        moveServo(currentAzimuth + 10);
        break;
      case 's': case 'S':
        moveServo(currentAzimuth - 10);
        break;
      case 'a': case 'A':
        moveStepper(currentElevation + 10);
        break;
      case 'd': case 'D':
        moveStepper(currentElevation - 10);
        break;
      case 'h': case 'H':
        moveServo(HOME_AZIMUTH);
        moveStepper(HOME_ELEVATION);
        break;
    }
  }
}

void handleStateError() {
  Serial.println("State: ERROR");
  digitalWrite(LED_ERROR, !digitalRead(LED_ERROR));
  delay(500);
}

// =============================================================================
// USER INTERFACE
// =============================================================================

void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  
  switch(currentState) {
    case STATE_INIT:
      lcd.print("INITIALIZING...");
      break;
    case STATE_SEARCHING:
      lcd.print("SEARCHING");
      lcd.setCursor(0, 1);
      lcd.print("Light:");
      lcd.print(avgTotal);
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
      if (overTemp) {
        lcd.print("HIGH TEMP!");
      } else {
        lcd.print("Night mode");
      }
      break;
    case STATE_MANUAL:
      lcd.print("MANUAL CTRL");
      lcd.setCursor(0, 1);
      lcd.print("Az:");
      lcd.print(currentAzimuth);
      lcd.print(" El:");
      lcd.print(currentElevation);
      break;
    case STATE_ERROR:
      lcd.print("ERROR!");
      lcd.setCursor(0, 1);
      lcd.print("Reset required");
      break;
  }
}

void updateStatusLEDs() {
  digitalWrite(LED_TRACKING, currentState == STATE_TRACKING ? HIGH : LOW);
  digitalWrite(LED_STOW, currentState == STATE_STOW ? HIGH : LOW);
  digitalWrite(LED_ERROR, currentState == STATE_ERROR ? HIGH : LOW);
}

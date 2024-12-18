#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

// Initialize LCD
hd44780_I2Cexp lcd;

// Pin Definitions
const int ultrasonicTriggerPin = 9; // Ultrasonic sensor trigger pin
const int ultrasonicEchoPin = 10;   // Ultrasonic sensor echo pin
const int soilMoisturePins[3] = {A0, A1, A2}; // Soil moisture sensor pins
const int pHSensorPin = A3; // pH sensor pin
const int buttonPins[5] = {5, 4, 3, 2, 6}; // Button pins (leftmost to rightmost + new button)

// Tank dimensions and thresholds
const int tankHeight = 11; // Height of the tank in cm (adjust as needed)
const int lowWaterLevelThreshold = 3; // Low water level threshold in cm

// Soil moisture thresholds
const int veryDryThreshold = 300; // Adjust for "Very Dry" reading
const int goodMoistureThreshold = 700; // Adjust for "Good Moisture" reading

// pH thresholds
const float lowPHThreshold = 5.5;  // Low pH threshold
const float highPHThreshold = 7.5; // High pH threshold

// State Variables
int currentScreen = 0; // 0: Water Level, 1-3: Soil Moisture Sensors, 4: pH Sensor
bool buttonPressed[5] = {false, false, false, false, false}; // Button press tracking

void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);

  // Initialize LCD
  lcd.begin(16, 2);
  lcd.print("Hydroponics");
  delay(2000);
  lcd.clear();

  // Initialize ultrasonic sensor pins
  pinMode(ultrasonicTriggerPin, OUTPUT);
  pinMode(ultrasonicEchoPin, INPUT);

  // Initialize button pins with internal pull-ups
  for (int i = 0; i < 5; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
}

void loop() {
  // Check button states to switch screens
  for (int i = 0; i < 5; i++) {
    if (digitalRead(buttonPins[i]) == LOW && !buttonPressed[i]) {
      buttonPressed[i] = true;
      currentScreen = i; // Switch to the screen corresponding to the button
      delay(200); // Debounce delay
    } else if (digitalRead(buttonPins[i]) == HIGH) {
      buttonPressed[i] = false;
    }
  }

  // Display the current screen
  switch (currentScreen) {
    case 0:
      displayWaterLevel();
      break;
    case 1:
    case 2:
    case 3:
      displaySoilMoisture(currentScreen - 1); // Soil moisture sensor 1-3
      break;
    case 4:
      displayPH();
      break;
  }

  delay(500); // Refresh rate
}

void displayWaterLevel() {
  // Measure water level using ultrasonic sensor
  long duration;
  int distance, waterLevel;

  digitalWrite(ultrasonicTriggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(ultrasonicTriggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(ultrasonicTriggerPin, LOW);
  duration = pulseIn(ultrasonicEchoPin, HIGH);

  distance = duration * 0.034 / 2; // Convert duration to cm
  waterLevel = tankHeight - distance; // Adjust based on your tank height

  // Print to Serial Monitor
  Serial.print("Water Level: ");
  Serial.print(waterLevel);
  Serial.println(" cm");

  // Display on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Water Level:");
  lcd.setCursor(0, 1);
  lcd.print(waterLevel);
  lcd.print(" cm");
}

void displaySoilMoisture(int sensorIndex) {
  // Read the moisture value from the corresponding sensor
  int soilMoistureValue = analogRead(soilMoisturePins[sensorIndex]);

  // Determine soil moisture status
  String status;
  if (soilMoistureValue < veryDryThreshold) {
    status = "Very Dry";
  } else if (soilMoistureValue < goodMoistureThreshold) {
    status = "Good Moisture";
  } else {
    status = "Too Wet";
  }

  // Print to Serial Monitor
  Serial.print("Soil ");
  Serial.print(sensorIndex + 1);
  Serial.print(" Moisture: ");
  Serial.print(soilMoistureValue);
  Serial.print(" - ");
  Serial.println(status);

  // Display on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Soil ");
  lcd.print(sensorIndex + 1);
  lcd.print(":");
  lcd.setCursor(0, 1);
  lcd.print(soilMoistureValue);
  lcd.print(" (");
  lcd.print(status);
  lcd.print(")");
}

void displayPH() {
  // Read pH value
  int rawValue = analogRead(pHSensorPin);
  float voltage = rawValue * (5.0 / 1023.0); // Convert raw value to voltage
  float pHValue = 3.5 * voltage; // Example calculation (adjust based on sensor calibration)

  // Determine pH status
  String status;
  if (pHValue < lowPHThreshold) {
    status = "Acidic";
  } else if (pHValue > highPHThreshold) {
    status = "Alkaline";
  } else {
    status = "Neutral";
  }

  // Print to Serial Monitor
  Serial.print("pH Level: ");
  Serial.print(pHValue);
  Serial.print(" - ");
  Serial.println(status);

  // Display on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("pH Level:");
  lcd.setCursor(0, 1);
  lcd.print(pHValue, 2); // Display with two decimal places
  lcd.print(" (");
  lcd.print(status);
  lcd.print(")");
}
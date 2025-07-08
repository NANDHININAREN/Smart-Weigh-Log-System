/* // WITH FOUR SEGMENT DISPLAY // */

#include "HX711.h"
#include <LiquidCrystal_I2C.h>

// HX711 Pins
const int dt_pin = A0;
const int sck_pin = A1;
HX711 scale;

// LCD Setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// 7-Segment Pins
const int digitPins[4] = {2, 3, 4, 5};  // Digit select pins (D1 to D4)
const int segmentPins[8] = {6, 7, 8, 9, 10, 11, 12, 13};  // Segment A to G and DP

// Segment byte patterns for 0-9 (Common Cathode)
const byte numberTable[10] = {
  B00111111, // 0
  B00000110, // 1
  B01011011, // 2
  B01001111, // 3
  B01100110, // 4
  B01101101, // 5
  B01111101, // 6
  B00000111, // 7
  B01111111, // 8
  B01101111  // 9
};

float total_amount = 0;
int currentWeight = 0;
unsigned long lastUpdate = 0;

void setup() {
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);

  // HX711 Setup
  scale.begin(dt_pin, sck_pin);
  scale.set_scale(405.0);
  scale.tare();
  Serial.println("Product,Weight(g),Price(₹),Total(₹)");

  // Segment pins as output
  for (int i = 0; i < 8; i++) pinMode(segmentPins[i], OUTPUT);
  for (int i = 0; i < 4; i++) pinMode(digitPins[i], OUTPUT);
}

void loop() {
  // Every 1.5 seconds: Update LCD + weight value
  if (millis() - lastUpdate > 1500) {
    if (scale.is_ready()) {
      float raw_weight = scale.get_units(15);
      if (raw_weight < 0) raw_weight = 0;
      currentWeight = (int)(raw_weight + 0.5);  // For display

      float item_price = currentWeight * 0.10;
      total_amount += item_price;
      int total_rounded = (int)(total_amount + 0.5);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("W:");
      lcd.print(currentWeight);
      lcd.print("g ");
      lcd.print(item_price, 1);

      lcd.setCursor(0, 1);
      lcd.print("Total: RS.");
      lcd.print(total_rounded);

      Serial.print("Item,");
      Serial.print(currentWeight);
      Serial.print(",");
      Serial.print(item_price, 1);
      Serial.print(",");
      Serial.println(total_rounded);

      lastUpdate = millis();
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Check HX711");
      Serial.println("Check HX711 wiring");
      lastUpdate = millis();
    }
  }

  // Always update 7-segment display in every loop
  displayNumber(currentWeight);
}

// Function to split and display number on 4-digit 7-segment
void displayNumber(float value) {
  int displayval=(int)(value*10);
  int digits[4] = {
    (displayval / 1000) % 10,
    (displayval / 100) % 10,
    (displayval / 10) % 10,
     displayval % 10
  };

  for (int i = 0; i < 4; i++) {
    byte pattern = numberTable[digits[i]];

    if (i == 1) pattern |= B10000000;  // Set DP bit

    setSegments(pattern);
    digitalWrite(digitPins[i], LOW);
    delay(2);
    digitalWrite(digitPins[i], HIGH);
  }
}

// Send segment pattern to display
void setSegments(byte pattern) {
  for (int i = 0; i < 8; i++) {
    digitalWrite(segmentPins[i], (pattern >> i) & 1);
  }
}

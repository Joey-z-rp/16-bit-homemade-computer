#ifndef LCD_CONTROLLER_H
#define LCD_CONTROLLER_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

class LCDController
{
public:
  // Constructor
  LCDController(uint8_t address = 0x27, uint8_t columns = 16, uint8_t rows = 2);

  // Setup method
  void setup();

  // Display update methods
  void updateDisplay(float frequency, unsigned long period, bool manualMode, bool clockState);
  void clearDisplay();
  void setBacklight(bool on);

  // Utility methods
  bool isConnected() const;
  void testDisplay();

private:
  LiquidCrystal_I2C *lcd;
  uint8_t lcdAddress;
  uint8_t lcdColumns;
  uint8_t lcdRows;
  bool connected;

  // Track last displayed values to avoid unnecessary updates
  float lastFrequency;
  unsigned long lastPeriod;
  bool lastManualMode;
  bool lastClockState;
  bool valuesChanged;

  // Display formatting methods
  String formatFrequency(float frequency);
  String formatPeriod(unsigned long period);
  String formatMode(bool manualMode, bool clockState);

  // Helper method to check if values have changed
  bool hasValuesChanged(float frequency, unsigned long period, bool manualMode, bool clockState);
};

#endif // LCD_CONTROLLER_H
#include <Arduino.h>
#include "ClockController.h"
#include "Debouncer.h"
#include "FrequencyCalculator.h"
#include "LCDController.h"

// Global objects
ClockController clockController;
Debouncer manualModeDebouncer(50);
Debouncer manualTriggerDebouncer(50);
FrequencyCalculator frequencyCalculator(FrequencyCalculator::DEFAULT_POT_PIN, 0.1, 4000000.0);
LCDController lcdController(0x27, 16, 2); // I2C address 0x27, 16x2 display

void setup()
{
  // Initialize serial for debugging
  Serial.begin(9600);
  Serial.println("16-bit Computer System Clock Starting...");

  // Setup clock controller
  clockController.setupPins();

  // Setup LCD display
  lcdController.setup();
  lcdController.testDisplay();

  Serial.println("System Clock Ready!");
}

void loop()
{
  // Update frequency calculator
  frequencyCalculator.updateFrequency();

  // Update clock controller with new frequency
  clockController.setFrequency(frequencyCalculator.getCurrentFrequency());

  // Update clock controller
  clockController.update();

  // Handle manual mode with debouncer (polling)
  bool manualModeReading = digitalRead(2); // D2
  if (manualModeDebouncer.update(manualModeReading))
  {
    // With pull-up resistor: LOW = pressed (manual mode ON), HIGH = not pressed (manual mode OFF)
    bool manualModeRequested = !manualModeDebouncer.getState(); // Invert because pull-up makes pressed = LOW
    if (manualModeRequested != clockController.isManualMode())
    {
      clockController.setManualMode(manualModeRequested);
    }
  }

  // Handle manual trigger with debouncer (polling)
  bool manualTriggerReading = digitalRead(3); // D3
  static bool lastTriggerState = HIGH;

  if (manualTriggerDebouncer.update(manualTriggerReading))
  {
    bool currentTriggerState = manualTriggerDebouncer.getState();

    // Detect press (falling edge)
    if (!currentTriggerState && lastTriggerState)
    {
      clockController.handleManualTriggerPress();
    }
    // Detect release (rising edge)
    else if (currentTriggerState && !lastTriggerState)
    {
      clockController.handleManualTriggerRelease();
    }

    lastTriggerState = currentTriggerState;
  }

  // Update LCD display
  lcdController.updateDisplay(
      clockController.getCurrentFrequency(),
      clockController.getCurrentPeriod(),
      clockController.isManualMode(),
      clockController.getClockState());

  // Debug output every second
  static unsigned long lastDebugTime = 0;
  if (millis() - lastDebugTime > 2000)
  {
    Serial.print("Pot Value: ");
    Serial.print(frequencyCalculator.getPotValue());
    Serial.print(" (");
    Serial.print(frequencyCalculator.getPotValue() * 100.0 / 1023.0);
    Serial.print("%), Requested: ");
    Serial.print(frequencyCalculator.getCurrentFrequency());
    Serial.print(" Hz, Actual: ");
    Serial.print(clockController.getCurrentFrequency());
    Serial.print(" Hz, Period: ");
    Serial.print(clockController.getCurrentPeriod());
    Serial.print(" us, Manual Mode: ");
    Serial.println(clockController.isManualMode() ? "ON" : "OFF");
    lastDebugTime = millis();
  }

  // Small delay to prevent overwhelming the system
  delayMicroseconds(100);
}
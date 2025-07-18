#include <Arduino.h>
#include "ClockController.h"
#include "Debouncer.h"
#include "FrequencyCalculator.h"

// Global objects
ClockController clockController;
Debouncer manualModeDebouncer(50);
Debouncer manualTriggerDebouncer(50);
FrequencyCalculator frequencyCalculator(A0, 1.0, 10000000.0);

void setup()
{
  // Initialize serial for debugging
  Serial.begin(9600);
  Serial.println("16-bit Computer System Clock Starting...");

  // Setup clock controller
  clockController.setupPins();

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
    if (manualModeDebouncer.getState() != clockController.isManualMode())
    {
      clockController.setManualMode(!manualModeDebouncer.getState()); // Inverted due to pull-up
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

  // Debug output every second
  static unsigned long lastDebugTime = 0;
  if (millis() - lastDebugTime > 1000)
  {
    Serial.print("Pot Value: ");
    Serial.print(frequencyCalculator.getPotValue());
    Serial.print(", Frequency: ");
    Serial.print(frequencyCalculator.getCurrentFrequency());
    Serial.print(" Hz, Period: ");
    Serial.print(frequencyCalculator.getCurrentPeriod());
    Serial.print(" us, Manual Mode: ");
    Serial.println(clockController.isManualMode() ? "ON" : "OFF");
    lastDebugTime = millis();
  }

  // Small delay to prevent overwhelming the system
  delayMicroseconds(100);
}
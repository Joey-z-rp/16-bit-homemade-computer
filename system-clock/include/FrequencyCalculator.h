#ifndef FREQUENCY_CALCULATOR_H
#define FREQUENCY_CALCULATOR_H

#include <Arduino.h>

class FrequencyCalculator
{
public:
  // Pin definitions
  static const int DEFAULT_POT_PIN = A0; // Default potentiometer pin

  // Constructor
  FrequencyCalculator(int potPin, float minFreq = 0.1, float maxFreq = 4000000.0);

  // Update frequency based on potentiometer
  float updateFrequency();

  // Get current values
  float getCurrentFrequency() const;
  int getPotValue() const;

  // Calculate frequency from potentiometer value
  float calculateFrequency(int potValue) const;

private:
  int potPin;
  float minFrequency;
  float maxFrequency;
  float currentFrequency;
  int currentPotValue;
  int lastPotValue;
  int debounceThreshold;

  // Logarithmic scaling parameters
  float logMinFreq;
  float logMaxFreq;
  float logRange;
};

#endif // FREQUENCY_CALCULATOR_H
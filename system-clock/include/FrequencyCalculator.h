#ifndef FREQUENCY_CALCULATOR_H
#define FREQUENCY_CALCULATOR_H

#include <Arduino.h>

class FrequencyCalculator
{
public:
  // Constructor
  FrequencyCalculator(int potPin, float minFreq = 1.0, float maxFreq = 10000000.0);

  // Update frequency based on potentiometer
  float updateFrequency();

  // Get current values
  float getCurrentFrequency() const;
  unsigned long getCurrentPeriod() const;
  int getPotValue() const;

  // Configuration
  void setFrequencyRange(float minFreq, float maxFreq);
  void setPotPin(int pin);

  // Calculate period from frequency
  static unsigned long calculatePeriod(float frequency);

  // Calculate frequency from potentiometer value
  float calculateFrequency(int potValue) const;

private:
  int potPin;
  float minFrequency;
  float maxFrequency;
  float currentFrequency;
  unsigned long currentPeriod;
  int currentPotValue;

  // Logarithmic scaling parameters
  float logMinFreq;
  float logMaxFreq;
  float logRange;
};

#endif // FREQUENCY_CALCULATOR_H
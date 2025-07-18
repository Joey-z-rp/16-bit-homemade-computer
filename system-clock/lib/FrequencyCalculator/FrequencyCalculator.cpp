#include "FrequencyCalculator.h"

FrequencyCalculator::FrequencyCalculator(int potPin, float minFreq, float maxFreq)
{
  this->potPin = potPin;
  this->minFrequency = minFreq;
  this->maxFrequency = maxFreq;
  this->currentFrequency = minFreq;
  this->currentPeriod = calculatePeriod(minFreq);
  this->currentPotValue = 0;

  // Initialize logarithmic scaling parameters
  logMinFreq = log10(minFreq);
  logMaxFreq = log10(maxFreq);
  logRange = logMaxFreq - logMinFreq;
}

float FrequencyCalculator::updateFrequency()
{
  // Read potentiometer value (0-1023)
  currentPotValue = analogRead(potPin);

  // Calculate frequency using logarithmic scale
  currentFrequency = calculateFrequency(currentPotValue);

  // Calculate period in microseconds
  currentPeriod = calculatePeriod(currentFrequency);

  // Ensure minimum period for stability
  if (currentPeriod < 1)
    currentPeriod = 1;

  return currentFrequency;
}

float FrequencyCalculator::getCurrentFrequency() const
{
  return currentFrequency;
}

unsigned long FrequencyCalculator::getCurrentPeriod() const
{
  return currentPeriod;
}

int FrequencyCalculator::getPotValue() const
{
  return currentPotValue;
}

void FrequencyCalculator::setFrequencyRange(float minFreq, float maxFreq)
{
  minFrequency = minFreq;
  maxFrequency = maxFreq;

  // Update logarithmic scaling parameters
  logMinFreq = log10(minFreq);
  logMaxFreq = log10(maxFreq);
  logRange = logMaxFreq - logMinFreq;
}

void FrequencyCalculator::setPotPin(int pin)
{
  potPin = pin;
}

unsigned long FrequencyCalculator::calculatePeriod(float frequency)
{
  return (unsigned long)(1000000.0 / frequency);
}

float FrequencyCalculator::calculateFrequency(int potValue) const
{
  // Convert to frequency range using logarithmic scale for better control
  // Map 0-1023 to log(MIN_FREQ) to log(MAX_FREQ)
  float logFreq = logMinFreq + (potValue / 1023.0) * logRange;

  // Convert back to frequency
  return pow(10, logFreq);
}
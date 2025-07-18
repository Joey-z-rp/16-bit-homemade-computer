#include "FrequencyCalculator.h"

FrequencyCalculator::FrequencyCalculator(int potPin, float minFreq, float maxFreq)
{
  this->potPin = potPin;
  this->minFrequency = minFreq;
  this->maxFrequency = maxFreq;
  this->currentFrequency = minFreq;
  this->currentPotValue = 0;
  this->lastPotValue = 0;
  this->debounceThreshold = 5;

  // Initialize logarithmic scaling parameters
  logMinFreq = log10(minFreq);
  logMaxFreq = log10(maxFreq);
  logRange = logMaxFreq - logMinFreq;
}

float FrequencyCalculator::updateFrequency()
{
  // Read potentiometer value (0-1023)
  int newPotValue = analogRead(potPin);

  // Check if the change is significant enough to update frequency
  if (abs(newPotValue - lastPotValue) >= debounceThreshold)
  {
    currentPotValue = newPotValue;
    lastPotValue = newPotValue;

    // Calculate frequency using logarithmic scale
    currentFrequency = calculateFrequency(currentPotValue);
  }

  return currentFrequency;
}

float FrequencyCalculator::getCurrentFrequency() const
{
  return currentFrequency;
}

int FrequencyCalculator::getPotValue() const
{
  return currentPotValue;
}

float FrequencyCalculator::calculateFrequency(int potValue) const
{
  // Custom mapping for better control at low and high frequencies
  // Use a sigmoid-like function to provide more precision at the ends

  // Normalize pot value to 0-1
  float normalizedValue = potValue / 1023.0;

  // Apply custom mapping function
  // This creates more precision at the ends and less in the middle
  float mappedValue;

  if (normalizedValue <= 0.5)
  {
    // First half: more precision at low frequencies
    // Use a curve that starts slow and accelerates
    float x = normalizedValue * 2.0; // 0 to 1
    mappedValue = x * x * x;         // Cubic curve for more precision at start
  }
  else
  {
    // Second half: more precision at high frequencies
    // Use a curve that accelerates and then slows down
    float x = (normalizedValue - 0.5) * 2.0;                       // 0 to 1
    mappedValue = 0.5 + (1.0 - (1.0 - x) * (1.0 - x) * (1.0 - x)); // Inverse cubic for more precision at end
  }

  // Map to logarithmic frequency range
  float logFreq = logMinFreq + mappedValue * logRange;

  // Convert back to frequency
  return pow(10, logFreq);
}
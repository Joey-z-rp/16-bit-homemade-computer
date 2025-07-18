#ifndef DEBOUNCER_H
#define DEBOUNCER_H

#include <Arduino.h>

class Debouncer
{
public:
  // Constructor
  Debouncer(unsigned long debounceDelay = 50);

  // Update method - call this in loop() for each switch
  bool update(bool currentState);

  // Check if state has changed
  bool hasChanged() const;

  // Get current debounced state
  bool getState() const;

  // Reset the debouncer
  void reset();

  // Set debounce delay
  void setDebounceDelay(unsigned long delay);

private:
  unsigned long debounceDelay;
  bool lastState;
  bool currentState;
  bool stateChanged;
  unsigned long lastChangeTime;
};

#endif // DEBOUNCER_H
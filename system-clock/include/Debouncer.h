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

  // Get current debounced state
  bool getState() const;

private:
  unsigned long debounceDelay;
  bool lastState;
  bool currentState;
  unsigned long lastChangeTime;
};

#endif // DEBOUNCER_H
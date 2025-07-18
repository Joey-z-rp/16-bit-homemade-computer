#include "Debouncer.h"

Debouncer::Debouncer(unsigned long debounceDelay)
{
  this->debounceDelay = debounceDelay;
  lastState = false;
  currentState = false;
  lastChangeTime = 0;
}

bool Debouncer::update(bool currentState)
{
  bool stateChangedThisUpdate = false;

  if (currentState != lastState)
  {
    lastChangeTime = millis();
  }

  if ((millis() - lastChangeTime) > debounceDelay)
  {
    if (currentState != this->currentState)
    {
      this->currentState = currentState;
      stateChangedThisUpdate = true;
    }
  }

  lastState = currentState;
  return stateChangedThisUpdate;
}

bool Debouncer::getState() const
{
  return currentState;
}
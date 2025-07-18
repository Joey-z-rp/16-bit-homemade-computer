#include "Debouncer.h"

Debouncer::Debouncer(unsigned long debounceDelay)
{
  this->debounceDelay = debounceDelay;
  lastState = false;
  currentState = false;
  stateChanged = false;
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
      stateChanged = true;
      stateChangedThisUpdate = true;
    }
  }

  lastState = currentState;
  return stateChangedThisUpdate;
}

bool Debouncer::hasChanged() const
{
  return stateChanged;
}

bool Debouncer::getState() const
{
  return currentState;
}

void Debouncer::reset()
{
  lastState = false;
  currentState = false;
  stateChanged = false;
  lastChangeTime = 0;
}

void Debouncer::setDebounceDelay(unsigned long delay)
{
  debounceDelay = delay;
}
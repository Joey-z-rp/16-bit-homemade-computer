#ifndef CLOCK_CONTROLLER_H
#define CLOCK_CONTROLLER_H

#include <Arduino.h>

class ClockController
{
public:
  // Constructor
  ClockController();

  // Setup methods
  void setupPins();

  // Main loop methods
  void update();

  // Clock control methods
  void setClockHigh();
  void setClockLow();
  void startClock();
  void stopClock();

  // Mode control methods
  void setManualMode(bool manual);
  bool isManualMode() const;

  // Manual trigger methods
  void handleManualTriggerPress();
  void handleManualTriggerRelease();

  // Getters
  float getCurrentFrequency() const;
  unsigned long getCurrentPeriod() const;
  bool getClockState() const;

  // Setters for modular design
  void setFrequency(float frequency);
  void setPeriod(unsigned long period);

  // Configuration
  static const unsigned long MIN_FREQ = 1;        // 1 Hz
  static const unsigned long MAX_FREQ = 10000000; // 10 MHz

private:
  // Pin definitions
  static const int MANUAL_MODE_PIN = 2;
  static const int MANUAL_TRIGGER_PIN = 3;
  static const int CLOCK_OUT_PIN = 9; // Timer1 OC1A pin

  // Clock state
  volatile bool clockState;
  volatile bool manualMode;
  volatile bool manualTriggerPressed;

  // Frequency calculation
  float currentFrequency;
  unsigned long currentPeriod;

  // Private methods
  unsigned long calculatePeriod(float frequency);
  void setupPWM();
  void stopPWM();
  void updatePWM();
};

#endif // CLOCK_CONTROLLER_H
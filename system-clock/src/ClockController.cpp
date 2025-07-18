#include "ClockController.h"

ClockController::ClockController()
{
  clockState = false;
  manualMode = false;
  currentFrequency = 1.0;
  requestedFrequency = 1.0;
  lastConfiguredFrequency = 0.0; // Initialize to 0 to force first configuration
  manualTriggerPressed = false;
  currentPeriod = 1000000000; // Default 1Hz period in nanoseconds
}

void ClockController::setupPins()
{
  // Setup input pins with internal pull-up resistors
  pinMode(MANUAL_MODE_PIN, INPUT_PULLUP);
  pinMode(MANUAL_TRIGGER_PIN, INPUT_PULLUP);

  // Setup clock output pin (Timer1 OC1A)
  pinMode(CLOCK_OUT_PIN, OUTPUT);
  digitalWrite(CLOCK_OUT_PIN, HIGH); // Start with HIGH output
  clockState = true;                 // Initialize clock state to HIGH
}

void ClockController::update()
{
  // Hardware PWM handles the square wave generation
  // No need for manual toggling in the main loop
}

void ClockController::setClockHigh()
{
  clockState = true;
  digitalWrite(CLOCK_OUT_PIN, HIGH);
}

void ClockController::setClockLow()
{
  clockState = false;
  digitalWrite(CLOCK_OUT_PIN, LOW);
}

void ClockController::startClock()
{
  if (!manualMode)
  {
    // Start hardware PWM for square wave generation
    setupPWM();
  }
}

void ClockController::stopClock()
{
  stopPWM();
  setClockHigh();
}

void ClockController::setManualMode(bool manual)
{
  manualMode = manual;

  if (manualMode)
  {
    Serial.println("Manual Mode: ON");
    stopClock();
  }
  else
  {
    Serial.println("Manual Mode: OFF");
    startClock();
  }
}

bool ClockController::isManualMode() const
{
  return manualMode;
}

void ClockController::handleManualTriggerPress()
{
  if (manualMode && !manualTriggerPressed)
  {
    manualTriggerPressed = true;
    setClockLow(); // Set output to LOW when trigger is pressed
    Serial.println("Manual Trigger: LOW");
  }
}

void ClockController::handleManualTriggerRelease()
{
  if (manualMode && manualTriggerPressed)
  {
    manualTriggerPressed = false;
    setClockHigh(); // Set output to HIGH when trigger is released
    Serial.println("Manual Trigger: HIGH");
  }
}

float ClockController::getCurrentFrequency() const
{
  return currentFrequency;
}

unsigned long ClockController::getCurrentPeriod() const
{
  return currentPeriod; // Returns period in nanoseconds
}

bool ClockController::getClockState() const
{
  return clockState;
}

void ClockController::setFrequency(float frequency)
{
  // Only update if requested frequency has actually changed
  if (abs(frequency - requestedFrequency) > 0.01) // Small tolerance for floating point comparison
  {
    requestedFrequency = frequency;

    // Update PWM with new frequency only if not in manual mode
    if (!manualMode)
    {
      updatePWM();
    }
  }
}

void ClockController::setPeriod(unsigned long period)
{
  // Only update if period has actually changed
  if (period != currentPeriod)
  {
    currentPeriod = period;                   // period is in nanoseconds
    currentFrequency = 1000000000.0 / period; // Convert nanoseconds to frequency

    if (!manualMode)
    {
      updatePWM();
    }
  }
}

unsigned long ClockController::calculatePeriod(float frequency)
{
  return (unsigned long)(1000000000.0 / frequency); // Returns period in nanoseconds
}

void ClockController::setupPWM()
{
  // Stop any existing PWM
  stopPWM();

  // Configure Timer1 for PWM generation on OC1A (pin 9)
  // Set pin as output
  DDRB |= (1 << PB1); // Set PB1 (pin 9) as output

  // Clear Timer1 registers
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;

  // Calculate PWM values for desired frequency
  unsigned long pwmTop;
  unsigned char prescaler;
  float actualFrequency;

  // Calculate the best prescaler and top value for the frequency
  if (requestedFrequency >= 1000000)
  {
    // High frequency: no prescaler
    prescaler = (1 << CS10);                      // No prescaler
    pwmTop = 16000000 / (2 * requestedFrequency); // 16MHz clock
    actualFrequency = 16000000.0 / (2.0 * pwmTop);
  }
  else if (requestedFrequency >= 100000)
  {
    // Medium frequency: prescaler 8
    prescaler = (1 << CS11);                     // Prescaler 8
    pwmTop = 2000000 / (2 * requestedFrequency); // 2MHz with prescaler 8
    actualFrequency = 2000000.0 / (2.0 * pwmTop);
  }
  else if (requestedFrequency >= 1.0)
  {
    // Low frequency: prescaler 64
    prescaler = (1 << CS11) | (1 << CS10);      // Prescaler 64
    pwmTop = 250000 / (2 * requestedFrequency); // 250kHz with prescaler 64
    actualFrequency = 250000.0 / (2.0 * pwmTop);
  }
  else
  {
    // Very low frequency: use prescaler 1024 for frequencies below 1 Hz
    prescaler = (1 << CS12) | (1 << CS10);     // Prescaler 1024
    pwmTop = 15625 / (2 * requestedFrequency); // 15.625kHz with prescaler 1024
    actualFrequency = 15625.0 / (2.0 * pwmTop);
  }

  // Ensure pwmTop is within valid range (minimum 2 for proper PWM operation)
  if (pwmTop < 2)
  {
    pwmTop = 2;
    // Recalculate actual frequency with minimum pwmTop
    if (requestedFrequency >= 1000000)
    {
      actualFrequency = 16000000.0 / (2.0 * pwmTop);
    }
    else if (requestedFrequency >= 100000)
    {
      actualFrequency = 2000000.0 / (2.0 * pwmTop);
    }
    else if (requestedFrequency >= 1.0)
    {
      actualFrequency = 250000.0 / (2.0 * pwmTop);
    }
    else
    {
      actualFrequency = 15625.0 / (2.0 * pwmTop);
    }
  }
  else if (pwmTop > 65535)
  {
    pwmTop = 65535;
    // Recalculate actual frequency with clamped pwmTop
    if (requestedFrequency >= 1000000)
    {
      actualFrequency = 16000000.0 / (2.0 * pwmTop);
    }
    else if (requestedFrequency >= 100000)
    {
      actualFrequency = 2000000.0 / (2.0 * pwmTop);
    }
    else if (requestedFrequency >= 1.0)
    {
      actualFrequency = 250000.0 / (2.0 * pwmTop);
    }
    else
    {
      actualFrequency = 15625.0 / (2.0 * pwmTop);
    }
  }

  // Set ICR1 as top value for Phase Correct PWM
  ICR1 = pwmTop;

  // Set OCR1A for 50% duty cycle (square wave)
  OCR1A = pwmTop / 2;

  // Configure Timer1 for Phase Correct PWM with ICR1 as top
  // COM1A1:0 = 10 for non-inverting PWM on OC1A
  // WGM13:0 = 1010 for Phase Correct PWM with ICR1 as top
  TCCR1A = (1 << COM1A1) | (1 << WGM11);
  TCCR1B = (1 << WGM13) | prescaler;

  // Update the actual frequency and period being generated
  currentFrequency = actualFrequency;
  currentPeriod = calculatePeriod(actualFrequency); // Now returns nanoseconds
  lastConfiguredFrequency = actualFrequency;

  Serial.print("PWM started - Requested: ");
  Serial.print(requestedFrequency);
  Serial.print(" Hz, Actual: ");
  Serial.print(actualFrequency);
  Serial.print(" Hz, Top: ");
  Serial.print(pwmTop);
  Serial.println();
}

void ClockController::stopPWM()
{
  // Stop Timer1
  TCCR1A = 0;
  TCCR1B = 0;

  Serial.println("PWM stopped");
}

void ClockController::updatePWM()
{
  // Only reconfigure PWM if requested frequency has changed since last configuration
  if (abs(requestedFrequency - lastConfiguredFrequency) > 0.01) // Small tolerance for floating point comparison
  {
    setupPWM();
    // Note: setupPWM() now updates lastConfiguredFrequency internally
  }
}
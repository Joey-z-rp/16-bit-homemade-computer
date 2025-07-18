#include "ClockController.h"

ClockController::ClockController()
{
  clockState = false;
  manualMode = false;
  currentFrequency = 1.0;
  manualTriggerPressed = false;
  currentPeriod = 1000000; // Default 1Hz period
}

void ClockController::setupPins()
{
  // Setup input pins with internal pull-up resistors
  pinMode(MANUAL_MODE_PIN, INPUT_PULLUP);
  pinMode(MANUAL_TRIGGER_PIN, INPUT_PULLUP);

  // Setup clock output pin (Timer1 OC1A)
  pinMode(CLOCK_OUT_PIN, OUTPUT);
  digitalWrite(CLOCK_OUT_PIN, LOW);
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
  setClockLow();
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
    setClockHigh();
    Serial.println("Manual Trigger: HIGH");
  }
}

void ClockController::handleManualTriggerRelease()
{
  if (manualMode && manualTriggerPressed)
  {
    manualTriggerPressed = false;
    setClockLow();
    Serial.println("Manual Trigger: LOW");
  }
}

float ClockController::getCurrentFrequency() const
{
  return currentFrequency;
}

unsigned long ClockController::getCurrentPeriod() const
{
  return currentPeriod;
}

bool ClockController::getClockState() const
{
  return clockState;
}

void ClockController::setFrequency(float frequency)
{
  currentFrequency = frequency;
  currentPeriod = calculatePeriod(frequency);

  // Ensure minimum period for stability
  if (currentPeriod < 1)
    currentPeriod = 1;

  // Update PWM with new frequency
  if (!manualMode)
  {
    updatePWM();
  }
}

void ClockController::setPeriod(unsigned long period)
{
  currentPeriod = period;
  currentFrequency = 1000000.0 / period;

  if (!manualMode)
  {
    updatePWM();
  }
}

unsigned long ClockController::calculatePeriod(float frequency)
{
  return (unsigned long)(1000000.0 / frequency);
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

  // Calculate the best prescaler and top value for the frequency
  if (currentFrequency > 1000000)
  {
    // High frequency: no prescaler
    prescaler = (1 << CS10);                    // No prescaler
    pwmTop = 16000000 / (2 * currentFrequency); // 16MHz clock
  }
  else if (currentFrequency > 100000)
  {
    // Medium frequency: prescaler 8
    prescaler = (1 << CS11);                   // Prescaler 8
    pwmTop = 2000000 / (2 * currentFrequency); // 2MHz with prescaler 8
  }
  else
  {
    // Low frequency: prescaler 64
    prescaler = (1 << CS11) | (1 << CS10);    // Prescaler 64
    pwmTop = 250000 / (2 * currentFrequency); // 250kHz with prescaler 64
  }

  // Ensure pwmTop is within 16-bit range
  if (pwmTop > 65535)
  {
    pwmTop = 65535;
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

  Serial.print("PWM started - Frequency: ");
  Serial.print(currentFrequency);
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
  // Reconfigure PWM with new frequency
  if (!manualMode)
  {
    setupPWM();
  }
}
#include "LCDController.h"

LCDController::LCDController(uint8_t address, uint8_t columns, uint8_t rows)
    : lcdAddress(address), lcdColumns(columns), lcdRows(rows), connected(false),
      lastFrequency(0), lastPeriod(0), lastManualMode(false), lastClockState(false), valuesChanged(false)
{
  lcd = new LiquidCrystal_I2C(lcdAddress, lcdColumns, lcdRows);
}

void LCDController::setup()
{
  Wire.begin();

  // Initialize the LCD
  lcd->init();
  lcd->backlight();

  // Clear and show initial message
  lcd->clear();
  lcd->setCursor(0, 0);
  lcd->print("Initializing...");

  connected = true;
}

void LCDController::updateDisplay(float frequency, unsigned long period, bool manualMode, bool clockState)
{
  if (!connected)
    return;

  // Check if any values have changed
  if (!hasValuesChanged(frequency, period, manualMode, clockState))
  {
    return; // No changes, skip update
  }

  // Update the display
  lcd->clear();

  // First line: Frequency
  lcd->setCursor(0, 0);
  String freqStr = formatFrequency(frequency);
  lcd->print(freqStr);

  // Second line: Period and Mode
  lcd->setCursor(0, 1);
  String periodStr = formatPeriod(period);
  lcd->print(periodStr);

  // Add mode indicator on the right side of second line
  String modeStr = formatMode(manualMode, clockState);
  int modeStart = lcdColumns - modeStr.length();
  if (modeStart > 0)
  {
    lcd->setCursor(modeStart, 1);
    lcd->print(modeStr);
  }

  // Store current values for next comparison
  lastFrequency = frequency;
  lastPeriod = period;
  lastManualMode = manualMode;
  lastClockState = clockState;
}

void LCDController::clearDisplay()
{
  if (connected)
  {
    lcd->clear();
  }
}

void LCDController::setBacklight(bool on)
{
  if (connected)
  {
    if (on)
    {
      lcd->backlight();
    }
    else
    {
      lcd->noBacklight();
    }
  }
}

bool LCDController::hasValuesChanged(float frequency, unsigned long period, bool manualMode, bool clockState)
{
  // Use small tolerance for floating point comparison
  const float freqTolerance = 0.1;

  return (abs(frequency - lastFrequency) > freqTolerance) ||
         (period != lastPeriod) ||
         (manualMode != lastManualMode) ||
         (clockState != lastClockState);
}

String LCDController::formatFrequency(float frequency)
{
  String result = "F:";

  if (frequency >= 1000000)
  {
    // Display in MHz
    result += String(frequency / 1000000.0, 2);
    result += "MHz";
  }
  else if (frequency >= 1000)
  {
    // Display in kHz
    result += String(frequency / 1000.0, 1);
    result += "kHz";
  }
  else if (frequency >= 1)
  {
    // Display in Hz
    result += String(frequency, 0);
    result += "Hz";
  }
  else if (frequency >= 0.001)
  {
    // Display in mHz (millihertz)
    result += String(frequency * 1000.0, 0);
    result += "mHz";
  }
  else
  {
    // Display in Hz with decimal for very low frequencies
    result += String(frequency, 2);
    result += "Hz";
  }

  return result;
}

String LCDController::formatPeriod(unsigned long period)
{
  String result = "P:";

  // period is in nanoseconds from ClockController
  if (period >= 1000000000)
  {
    // Display in seconds
    result += String(period / 1000000000.0, 3);
    result += "s";
  }
  else if (period >= 1000000)
  {
    // Display in milliseconds
    result += String(period / 1000000.0, 3);
    result += "ms";
  }
  else if (period >= 1000)
  {
    // Display in microseconds
    result += String(period / 1000.0, 1);
    result += "us";
  }
  else
  {
    // Display in nanoseconds
    result += String(period);
    result += "ns";
  }

  return result;
}

String LCDController::formatMode(bool manualMode, bool clockState)
{
  if (manualMode)
  {
    return clockState ? "M:Low" : "M:High";
  }
  else
  {
    return "AUTO";
  }
}
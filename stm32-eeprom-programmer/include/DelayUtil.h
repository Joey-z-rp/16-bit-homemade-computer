#ifndef DELAY_UTIL_H
#define DELAY_UTIL_H

#include "stm32f1xx_hal.h"

class DelayUtil
{
public:
  // Basic delay functions
  static void delay(uint32_t cycles);
  static void delayMicroseconds(uint32_t microseconds);
  static void delayMilliseconds(uint32_t milliseconds);

private:
  static const uint32_t CYCLES_PER_MICROSECOND = 72; // Assuming 72MHz clock
  static const uint32_t CYCLES_PER_MILLISECOND = 72000;
};

#endif // DELAY_UTIL_H
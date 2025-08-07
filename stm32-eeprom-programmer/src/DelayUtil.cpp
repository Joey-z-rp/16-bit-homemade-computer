#include "DelayUtil.h"

void DelayUtil::delay(uint32_t cycles)
{
  volatile uint32_t delay_count;
  for (delay_count = 0; delay_count < cycles; delay_count++)
  {
    // Busy wait loop
  }
}

void DelayUtil::delayMicroseconds(uint32_t microseconds)
{
  uint32_t cycles = microseconds * CYCLES_PER_MICROSECOND;
  delay(cycles);
}

void DelayUtil::delayMilliseconds(uint32_t milliseconds)
{
  uint32_t cycles = milliseconds * CYCLES_PER_MILLISECOND;
  delay(cycles);
}
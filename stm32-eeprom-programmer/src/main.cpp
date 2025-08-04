#include "stm32f1xx_hal.h"
#include "EEPROMProgrammer.h"

// Global EEPROM programmer instance
EEPROMProgrammer eeprom;

int main(void)
{
  // Initialize EEPROM programmer
  eeprom.begin();

  // Simple delay function
  volatile uint32_t delay_count;

  const uint8_t programData[] = {
      0x00,
      0xEA,
      0x00,
      0xEF,
      0x00,
      0xFC,
      0x00,
      0xE4,
      0x00,
      0xE3,
      0x00,
      0xFC,
      0x00,
      0xF0,
      0x00,
      0xFD,
      0x00,
      0xEA,
      0x00,
      0xEA,
  };

  // volatile bool writeSuccess = eeprom.writeDataBlock(0x0000, programData, sizeof(programData));

  volatile uint8_t readData = eeprom.readByte(0x0001);
  while (1)
  {
    for (delay_count = 0; delay_count < 2000000; delay_count++)
    {
    }
    eeprom.blinkLED(1);
  }
}
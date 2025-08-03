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

  // eeprom.setPinHigh(eeprom.addressPortA, eeprom.ADDRESS_PIN_A0);
  // eeprom.setPinHigh(eeprom.addressPortA, eeprom.ADDRESS_PIN_A1);
  // eeprom.setPinHigh(eeprom.addressPortA, eeprom.ADDRESS_PIN_A2);
  // eeprom.setPinHigh(eeprom.addressPortA, eeprom.ADDRESS_PIN_A3);
  // eeprom.setPinHigh(eeprom.addressPortA, eeprom.ADDRESS_PIN_A4);
  // eeprom.setPinHigh(eeprom.addressPortA, eeprom.ADDRESS_PIN_A5);
  // eeprom.setPinHigh(eeprom.addressPortA, eeprom.ADDRESS_PIN_A6);
  // eeprom.setPinHigh(eeprom.addressPortA, eeprom.ADDRESS_PIN_A7);
  // eeprom.setPinHigh(eeprom.addressPortA, eeprom.ADDRESS_PIN_A8);
  // eeprom.setPinHigh(eeprom.addressPortA, eeprom.ADDRESS_PIN_A9);
  // eeprom.setPinHigh(eeprom.addressPortB, eeprom.ADDRESS_PIN_A10);
  // eeprom.setPinHigh(eeprom.addressPortB, eeprom.ADDRESS_PIN_A11);
  // eeprom.setPinHigh(eeprom.addressPortB, eeprom.ADDRESS_PIN_A12);
  // eeprom.setPinHigh(eeprom.addressPortB, eeprom.ADDRESS_PIN_A13);
  // eeprom.setPinHigh(eeprom.addressPortB, eeprom.ADDRESS_PIN_A14);

  eeprom.setPinLow(eeprom.addressPortA, eeprom.ADDRESS_PIN_A0);
  eeprom.setPinLow(eeprom.addressPortA, eeprom.ADDRESS_PIN_A1);
  eeprom.setPinLow(eeprom.addressPortA, eeprom.ADDRESS_PIN_A2);
  eeprom.setPinLow(eeprom.addressPortA, eeprom.ADDRESS_PIN_A3);
  eeprom.setPinLow(eeprom.addressPortA, eeprom.ADDRESS_PIN_A4);
  eeprom.setPinLow(eeprom.addressPortA, eeprom.ADDRESS_PIN_A5);
  eeprom.setPinLow(eeprom.addressPortA, eeprom.ADDRESS_PIN_A6);
  eeprom.setPinLow(eeprom.addressPortA, eeprom.ADDRESS_PIN_A7);
  eeprom.setPinLow(eeprom.addressPortA, eeprom.ADDRESS_PIN_A8);
  eeprom.setPinLow(eeprom.addressPortA, eeprom.ADDRESS_PIN_A9);
  eeprom.setPinLow(eeprom.addressPortB, eeprom.ADDRESS_PIN_A10);
  eeprom.setPinLow(eeprom.addressPortB, eeprom.ADDRESS_PIN_A11);
  eeprom.setPinLow(eeprom.addressPortB, eeprom.ADDRESS_PIN_A12);
  eeprom.setPinLow(eeprom.addressPortB, eeprom.ADDRESS_PIN_A13);
  eeprom.setPinLow(eeprom.addressPortB, eeprom.ADDRESS_PIN_A14);

  // -----------------------------

  // eeprom.setPinLow(eeprom.controlPort, eeprom.EEPROM_CE_PIN);
  // eeprom.setPinHigh(eeprom.controlPort, eeprom.EEPROM_OE_PIN);
  // eeprom.setDataBusOutput();
  // eeprom.setPinHigh(eeprom.dataPort, eeprom.DATA_PIN_D0);
  // eeprom.setPinLow(eeprom.dataPort, eeprom.DATA_PIN_D1);
  // eeprom.setPinHigh(eeprom.dataPort, eeprom.DATA_PIN_D2);
  // eeprom.setPinLow(eeprom.dataPort, eeprom.DATA_PIN_D3);
  // eeprom.setPinHigh(eeprom.dataPort, eeprom.DATA_PIN_D4);
  // eeprom.setPinLow(eeprom.dataPort, eeprom.DATA_PIN_D5);
  // eeprom.setPinHigh(eeprom.dataPort, eeprom.DATA_PIN_D6);
  // eeprom.setPinLow(eeprom.dataPort, eeprom.DATA_PIN_D7);
  // eeprom.setPinLow(eeprom.controlPort, eeprom.EEPROM_WE_PIN);
  // eeprom.setPinHigh(eeprom.controlPort, eeprom.EEPROM_WE_PIN);

  // -----------------------------

  eeprom.setPinLow(eeprom.controlPort, eeprom.EEPROM_CE_PIN);
  eeprom.setPinLow(eeprom.controlPort, eeprom.EEPROM_OE_PIN);
  volatile uint8_t readData = eeprom.readData();
  uint8_t verifyData = eeprom.readData();
  if (verifyData == readData)
  {
    eeprom.blinkLED(3);
  }

  // ------------------
  while (1)
  {
    for (delay_count = 0; delay_count < 2000000; delay_count++)
    {
    }
    eeprom.blinkLED(1);
  }
}
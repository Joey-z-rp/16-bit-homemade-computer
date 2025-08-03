#ifndef EEPROM_PROGRAMMER_H
#define EEPROM_PROGRAMMER_H

#include "stm32f1xx_hal.h"

class EEPROMProgrammer
{
public:
  // Pin definitions for STM32 Blue Pill
  // Address pins A0-A14 (using GPIOA, GPIOB, and GPIOC pins)
  static const uint16_t ADDRESS_PIN_A0 = GPIO_PIN_0;  // PA0
  static const uint16_t ADDRESS_PIN_A1 = GPIO_PIN_1;  // PA1
  static const uint16_t ADDRESS_PIN_A2 = GPIO_PIN_2;  // PA2
  static const uint16_t ADDRESS_PIN_A3 = GPIO_PIN_3;  // PA3
  static const uint16_t ADDRESS_PIN_A4 = GPIO_PIN_4;  // PA4
  static const uint16_t ADDRESS_PIN_A5 = GPIO_PIN_8;  // PA8
  static const uint16_t ADDRESS_PIN_A6 = GPIO_PIN_9;  // PA9
  static const uint16_t ADDRESS_PIN_A7 = GPIO_PIN_10; // PA10
  static const uint16_t ADDRESS_PIN_A8 = GPIO_PIN_11; // PA11
  static const uint16_t ADDRESS_PIN_A9 = GPIO_PIN_12; // PA12
  static const uint16_t ADDRESS_PIN_A10 = GPIO_PIN_3; // PB3
  static const uint16_t ADDRESS_PIN_A11 = GPIO_PIN_4; // PB4
  static const uint16_t ADDRESS_PIN_A12 = GPIO_PIN_5; // PB5
  static const uint16_t ADDRESS_PIN_A13 = GPIO_PIN_6; // PB6
  static const uint16_t ADDRESS_PIN_A14 = GPIO_PIN_7; // PB7

  // Data pins D0-D7 (using GPIOB pins)
  static const uint16_t DATA_PIN_D0 = GPIO_PIN_0;  // PB0
  static const uint16_t DATA_PIN_D1 = GPIO_PIN_1;  // PB1
  static const uint16_t DATA_PIN_D2 = GPIO_PIN_10; // PB10
  static const uint16_t DATA_PIN_D3 = GPIO_PIN_11; // PB11
  static const uint16_t DATA_PIN_D4 = GPIO_PIN_12; // PB12
  static const uint16_t DATA_PIN_D5 = GPIO_PIN_13; // PB13
  static const uint16_t DATA_PIN_D6 = GPIO_PIN_14; // PB14
  static const uint16_t DATA_PIN_D7 = GPIO_PIN_15; // PB15

  // Control pins
  static const uint16_t EEPROM_WE_PIN = GPIO_PIN_5;   // PA5 - Write Enable (active low)
  static const uint16_t EEPROM_OE_PIN = GPIO_PIN_6;   // PA6 - Output Enable (active low)
  static const uint16_t EEPROM_CE_PIN = GPIO_PIN_7;   // PA7 - Chip Enable (active low)
  static const uint16_t STATUS_LED_PIN = GPIO_PIN_13; // PC13 - Built-in LED

  // EEPROM specifications
  static const uint16_t EEPROM_SIZE = 32768;       // 32KB = 32768 bytes
  static const int ADDRESS_BITS = 15;              // 15 address lines (A0-A14)
  static const int PROGRAMMABLE_ADDRESS_BITS = 15; // All 15 address bits are programmable

  // Constructor
  EEPROMProgrammer();

  // Initialization
  void begin();

  // Data bus direction control
  void setDataBusInput();
  void setDataBusOutput();

  // Address and data handling
  void setAddress(uint16_t address);
  uint8_t readData();
  void writeData(uint8_t data);

  // Write completion detection
  bool waitForWriteComplete(uint8_t expectedData);

  // Software data protection
  void disableSoftwareDataProtection();

  // Basic operations
  uint8_t readByte(uint16_t address);
  bool writeByte(uint16_t address, uint8_t data);

  // Bulk operations
  bool eraseChip();
  bool writeDataBlock(uint16_t startAddress, const uint8_t *data, uint16_t length);
  bool verifyData(uint16_t startAddress, const uint8_t *data, uint16_t length);

  // Utility functions
  void dumpMemory(uint16_t startAddress, uint16_t length);
  void blinkLED(int times = 1);

  // GPIO port pointers
  GPIO_TypeDef *addressPortA;
  GPIO_TypeDef *addressPortB;
  GPIO_TypeDef *dataPort;
  GPIO_TypeDef *controlPort;
  GPIO_TypeDef *ledPort;

  // Data bus pin array for easy iteration
  uint16_t dataPins[8];

  // Helper functions
  void configureGPIO();
  void setPinHigh(GPIO_TypeDef *port, uint16_t pin);
  void setPinLow(GPIO_TypeDef *port, uint16_t pin);
  uint8_t readPin(GPIO_TypeDef *port, uint16_t pin);
};

#endif // EEPROM_PROGRAMMER_H
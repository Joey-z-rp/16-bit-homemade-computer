#ifndef EEPROM_PROGRAMMER_H
#define EEPROM_PROGRAMMER_H

#include <Arduino.h>

class EEPROMProgrammer
{
private:
  // Pin definitions
  static const int SHIFT_DATA_PIN = 2;  // DS (Data Serial)
  static const int SHIFT_CLOCK_PIN = 3; // SHCP (Shift Register Clock)
  static const int SHIFT_LATCH_PIN = 4; // STCP (Storage Register Clock)
  static const int DATA_PINS_START = 5; // D0 starts at pin 5 (D5-D12)
  static const int EEPROM_WE_PIN = A3;  // Write Enable (active low)
  static const int EEPROM_OE_PIN = A4;  // Output Enable (active low)
  static const int EEPROM_CE_PIN = A5;  // Chip Enable (active low)
  static const int STATUS_LED_PIN = 13; // Built-in LED on pin 13

  // EEPROM specifications
  static const uint16_t EEPROM_SIZE = 32768; // 32KB = 32768 bytes
  static const int ADDRESS_BITS = 15;        // 15 address lines (A0-A14)

  // Data bus direction control
  void setDataBusInput();
  void setDataBusOutput();

  // Address and data handling
  void setAddress(uint16_t address);
  uint8_t readData();
  void writeData(uint8_t data);

  // Write completion detection
  bool waitForWriteComplete(uint8_t expectedData);

public:
  void begin();

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
};

#endif // EEPROM_PROGRAMMER_H
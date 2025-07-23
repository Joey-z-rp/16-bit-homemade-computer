#include "EEPROMProgrammer.h"

void EEPROMProgrammer::setDataBusInput()
{
  for (int i = 0; i < 8; i++)
  {
    pinMode(DATA_PINS_START + i, INPUT);
  }
}

void EEPROMProgrammer::setDataBusOutput()
{
  for (int i = 0; i < 8; i++)
  {
    pinMode(DATA_PINS_START + i, OUTPUT);
  }
}

void EEPROMProgrammer::setAddress(uint16_t address)
{
  digitalWrite(SHIFT_LATCH_PIN, LOW);

  // Send 15 address bits (A0-A14)
  for (int i = 0; i < ADDRESS_BITS; i++)
  {
    digitalWrite(SHIFT_DATA_PIN, (address >> i) & 0x01);
    digitalWrite(SHIFT_CLOCK_PIN, HIGH);
    digitalWrite(SHIFT_CLOCK_PIN, LOW);
  }

  digitalWrite(SHIFT_LATCH_PIN, HIGH);
}

uint8_t EEPROMProgrammer::readData()
{
  uint8_t data = 0;
  for (int i = 0; i < 8; i++)
  {
    data |= (digitalRead(DATA_PINS_START + i) << i);
  }
  return data;
}

void EEPROMProgrammer::writeData(uint8_t data)
{
  for (int i = 0; i < 8; i++)
  {
    digitalWrite(DATA_PINS_START + i, (data >> i) & 0x01);
  }
}

bool EEPROMProgrammer::waitForWriteComplete(uint8_t expectedData)
{
  setDataBusInput();
  digitalWrite(EEPROM_OE_PIN, LOW);

  // Poll the data bus for write completion
  // During write, reading the same address should return the complement of written data
  // When write is complete, it returns the actual written data
  uint8_t currentData;
  unsigned long startTime = millis();
  int stableCount = 0;
  const int requiredStableReads = 3; // Need 3 consecutive same reads to confirm completion

  while (millis() - startTime < 1000)
  { // 1 second timeout
    currentData = readData();

    // Check if we're reading the expected data (write is complete)
    if (currentData == expectedData)
    {
      stableCount++;
      if (stableCount >= requiredStableReads)
      {
        digitalWrite(EEPROM_OE_PIN, HIGH);
        return true;
      }
    }
    else
    {
      stableCount = 0; // Reset counter if data doesn't match expected
    }

    delayMicroseconds(10);
  }

  digitalWrite(EEPROM_OE_PIN, HIGH);
  return false;
}

void EEPROMProgrammer::begin()
{
  // Initialize shift register pins
  pinMode(SHIFT_DATA_PIN, OUTPUT);
  pinMode(SHIFT_CLOCK_PIN, OUTPUT);
  pinMode(SHIFT_LATCH_PIN, OUTPUT);

  // Initialize EEPROM control pins
  pinMode(EEPROM_WE_PIN, OUTPUT);
  pinMode(EEPROM_OE_PIN, OUTPUT);
  pinMode(EEPROM_CE_PIN, OUTPUT);

  // Initialize status LED
  pinMode(STATUS_LED_PIN, OUTPUT);

  // Set initial states
  digitalWrite(SHIFT_DATA_PIN, LOW);
  digitalWrite(SHIFT_CLOCK_PIN, LOW);
  digitalWrite(SHIFT_LATCH_PIN, HIGH);
  digitalWrite(EEPROM_WE_PIN, HIGH); // Write disabled
  digitalWrite(EEPROM_OE_PIN, HIGH); // Output disabled
  digitalWrite(EEPROM_CE_PIN, HIGH); // Chip disabled
  digitalWrite(STATUS_LED_PIN, LOW);
}

uint8_t EEPROMProgrammer::readByte(uint16_t address)
{
  if (address >= EEPROM_SIZE)
    return 0xFF;

  setAddress(address);
  setDataBusInput();

  digitalWrite(EEPROM_CE_PIN, LOW);
  digitalWrite(EEPROM_OE_PIN, LOW);

  uint8_t data = readData();

  digitalWrite(EEPROM_OE_PIN, HIGH);
  digitalWrite(EEPROM_CE_PIN, HIGH);

  return data;
}

bool EEPROMProgrammer::writeByte(uint16_t address, uint8_t data)
{
  if (address >= EEPROM_SIZE)
    return false;

  setAddress(address);
  setDataBusOutput();
  writeData(data);

  digitalWrite(EEPROM_CE_PIN, LOW);
  digitalWrite(EEPROM_WE_PIN, LOW);
  delayMicroseconds(1);
  digitalWrite(EEPROM_WE_PIN, HIGH);

  return waitForWriteComplete(data);
}

bool EEPROMProgrammer::eraseChip()
{
  Serial.println("Erasing EEPROM...");

  for (uint16_t address = 0; address < EEPROM_SIZE; address++)
  {
    if (!writeByte(address, 0xFF))
    {
      Serial.print("Erase failed at address 0x");
      Serial.println(address, HEX);
      return false;
    }

    if (address % 1024 == 0)
    {
      Serial.print("Erased ");
      Serial.print(address);
      Serial.println(" bytes");
    }
  }

  Serial.println("Erase complete");
  return true;
}

bool EEPROMProgrammer::writeDataBlock(uint16_t startAddress, const uint8_t *data, uint16_t length)
{
  Serial.println("Writing data block...");

  for (uint16_t i = 0; i < length; i++)
  {
    if (!writeByte(startAddress + i, data[i]))
    {
      Serial.print("Write failed at address 0x");
      Serial.println(startAddress + i, HEX);
      return false;
    }

    if (i % 256 == 0)
    {
      Serial.print("Written ");
      Serial.print(i);
      Serial.println(" bytes");
    }
  }

  Serial.println("Write complete");
  return true;
}

bool EEPROMProgrammer::verifyData(uint16_t startAddress, const uint8_t *data, uint16_t length)
{
  Serial.println("Verifying data...");

  for (uint16_t i = 0; i < length; i++)
  {
    uint8_t readData = readByte(startAddress + i);
    if (readData != data[i])
    {
      Serial.print("Verify failed at address 0x");
      Serial.print(startAddress + i, HEX);
      Serial.print(": expected 0x");
      Serial.print(data[i], HEX);
      Serial.print(", got 0x");
      Serial.println(readData, HEX);
      return false;
    }
  }

  Serial.println("Verify complete");
  return true;
}

void EEPROMProgrammer::dumpMemory(uint16_t startAddress, uint16_t length)
{
  Serial.println("Memory dump:");

  for (uint16_t addr = startAddress; addr < startAddress + length; addr += 16)
  {
    Serial.print("0x");
    Serial.print(addr, HEX);
    Serial.print(": ");

    for (int i = 0; i < 16; i++)
    {
      if (addr + i < startAddress + length)
      {
        uint8_t data = readByte(addr + i);
        if (data < 0x10)
          Serial.print("0");
        Serial.print(data, HEX);
        Serial.print(" ");
      }
    }

    Serial.println();
  }
}

void EEPROMProgrammer::blinkLED(int times)
{
  for (int i = 0; i < times; i++)
  {
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(200);
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(200);
  }
}
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
  // Send 15 address bits (A0-A14)
  for (int i = ADDRESS_BITS - 1; i >= 0; i--)
  {
    digitalWrite(SHIFT_DATA_PIN, (address >> i) & 0x01);
    digitalWrite(SHIFT_CLOCK_PIN, HIGH);
    digitalWrite(SHIFT_CLOCK_PIN, LOW);
  }

  digitalWrite(SHIFT_LATCH_PIN, HIGH);
  digitalWrite(SHIFT_LATCH_PIN, LOW);
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
  // Wait for the minimum write time before checking
  delayMicroseconds(100);
  setDataBusInput();
  digitalWrite(EEPROM_OE_PIN, LOW);

  // Poll the data bus for write completion
  // During write, reading the same address should return the complement of written data
  // When write is complete, it returns the actual written data
  uint8_t currentData;
  unsigned long startTime = millis();
  int stableCount = 0;
  const int requiredStableReads = 3;

  while (millis() - startTime < 1000)
  {
    currentData = readData();

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

  // Debug output for timeout
  Serial.print("Write timeout - expected 0x");
  Serial.print(expectedData, HEX);
  Serial.print(", last read 0x");
  Serial.println(currentData, HEX);

  digitalWrite(EEPROM_OE_PIN, HIGH);
  return false;
}

void EEPROMProgrammer::begin()
{
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
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
  digitalWrite(SHIFT_LATCH_PIN, LOW);
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

  delayMicroseconds(1);
  uint8_t data = readData();

  digitalWrite(EEPROM_OE_PIN, HIGH);
  digitalWrite(EEPROM_CE_PIN, HIGH);

  return data;
}

bool EEPROMProgrammer::writeByte(uint16_t address, uint8_t data)
{
  if (address >= EEPROM_SIZE)
    return false;

  digitalWrite(EEPROM_OE_PIN, HIGH);
  digitalWrite(EEPROM_CE_PIN, LOW);

  setDataBusOutput();

  setAddress(address);
  writeData(data);

  digitalWrite(EEPROM_WE_PIN, LOW);

  delayMicroseconds(1);
  digitalWrite(EEPROM_WE_PIN, HIGH);

  bool result = waitForWriteComplete(data);

  // Debug output for failed writes
  if (!result)
  {
    Serial.print("Write failed at 0x");
    Serial.print(address, HEX);
    Serial.print(" with data 0x");
    Serial.println(data, HEX);
  }
  else
  {
    Serial.print("Write successful at 0x");
    Serial.print(address, HEX);
    Serial.print(" with data 0x");
    Serial.println(data, HEX);
  }

  return result;
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

void EEPROMProgrammer::disableSoftwareDataProtection()
{
  // 28C256 Software Data Protection disable sequence
  // Write 0xAA to address 0x5555
  // Write 0x55 to address 0x2AAA
  // Write 0x80 to address 0x5555
  // Write 0xAA to address 0x5555
  // Write 0x55 to address 0x2AAA
  // Write 0x20 to address 0x5555

  Serial.println("Sending SDP disable sequence...");
  digitalWrite(EEPROM_OE_PIN, HIGH);
  setDataBusOutput();
  digitalWrite(EEPROM_CE_PIN, LOW);

  // Step 1: Write 0xAA to address 0x5555
  setAddress(0x5555);
  writeData(0xAA);
  digitalWrite(EEPROM_WE_PIN, LOW);
  digitalWrite(EEPROM_WE_PIN, HIGH);

  // Step 2: Write 0x55 to address 0x2AAA
  setAddress(0x2AAA);
  writeData(0x55);
  digitalWrite(EEPROM_WE_PIN, LOW);
  digitalWrite(EEPROM_WE_PIN, HIGH);

  // Step 3: Write 0x80 to address 0x5555
  setAddress(0x5555);
  writeData(0x80);
  digitalWrite(EEPROM_WE_PIN, LOW);
  digitalWrite(EEPROM_WE_PIN, HIGH);

  // Step 4: Write 0xAA to address 0x5555
  setAddress(0x5555);
  writeData(0xAA);
  digitalWrite(EEPROM_WE_PIN, LOW);
  digitalWrite(EEPROM_WE_PIN, HIGH);

  // Step 5: Write 0x55 to address 0x2AAA
  setAddress(0x2AAA);
  writeData(0x55);
  digitalWrite(EEPROM_WE_PIN, LOW);
  digitalWrite(EEPROM_WE_PIN, HIGH);

  // Step 6: Write 0x20 to address 0x5555
  setAddress(0x5555);
  writeData(0x20);
  digitalWrite(EEPROM_WE_PIN, LOW);
  digitalWrite(EEPROM_WE_PIN, HIGH);

  digitalWrite(EEPROM_CE_PIN, HIGH);
  Serial.println("SDP disable sequence completed");
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
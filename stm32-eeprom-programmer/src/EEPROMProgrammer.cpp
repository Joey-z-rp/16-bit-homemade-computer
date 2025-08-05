#include "EEPROMProgrammer.h"

EEPROMProgrammer::EEPROMProgrammer()
{
  // Initialize GPIO port pointers
  addressPortA = GPIOA;
  addressPortB = GPIOB;
  dataPort = GPIOB;
  dataPortC = GPIOC; // For D2 which is now on PC15
  controlPort = GPIOA;
  ledPort = GPIOC;

  // Initialize data pin array
  dataPins[0] = DATA_PIN_D0;
  dataPins[1] = DATA_PIN_D1;
  dataPins[2] = DATA_PIN_D2;
  dataPins[3] = DATA_PIN_D3;
  dataPins[4] = DATA_PIN_D4;
  dataPins[5] = DATA_PIN_D5;
  dataPins[6] = DATA_PIN_D6;
  dataPins[7] = DATA_PIN_D7;
}

void EEPROMProgrammer::begin()
{
  // Enable GPIO clocks
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  configureGPIO();

  // Initialize control pins to inactive state
  setPinHigh(controlPort, EEPROM_WE_PIN); // WE inactive (high)
  setPinHigh(controlPort, EEPROM_OE_PIN); // OE inactive (high)
  setPinHigh(controlPort, EEPROM_CE_PIN); // CE inactive (high)

  // Blink LED to indicate initialization
  blinkLED(3);
}

void EEPROMProgrammer::configureGPIO()
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // Configure address pins A0-A9 (PA0-PA4, PA8-PA12) as outputs
  GPIO_InitStruct.Pin = ADDRESS_PIN_A0 | ADDRESS_PIN_A1 | ADDRESS_PIN_A2 |
                        ADDRESS_PIN_A3 | ADDRESS_PIN_A4 | ADDRESS_PIN_A5 |
                        ADDRESS_PIN_A6 | ADDRESS_PIN_A7 | ADDRESS_PIN_A8 | ADDRESS_PIN_A9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(addressPortA, &GPIO_InitStruct);

  // Configure address pins A10-A14 as outputs
  GPIO_InitStruct.Pin = ADDRESS_PIN_A10 | ADDRESS_PIN_A11 | ADDRESS_PIN_A12 |
                        ADDRESS_PIN_A13 | ADDRESS_PIN_A14;
  HAL_GPIO_Init(addressPortB, &GPIO_InitStruct);

  // Configure control pins (PA5-PA7) as outputs
  GPIO_InitStruct.Pin = EEPROM_WE_PIN | EEPROM_OE_PIN | EEPROM_CE_PIN;
  HAL_GPIO_Init(controlPort, &GPIO_InitStruct);

  // Configure LED pin (PC13) as output
  GPIO_InitStruct.Pin = STATUS_LED_PIN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(ledPort, &GPIO_InitStruct);

  // Configure data pins as inputs initially
  setDataBusInput();
}

void EEPROMProgrammer::setDataBusInput()
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

  // Configure data pins D0-D1, D3-D7 on GPIOB as inputs
  GPIO_InitStruct.Pin = DATA_PIN_D0 | DATA_PIN_D1 | DATA_PIN_D3 |
                        DATA_PIN_D4 | DATA_PIN_D5 | DATA_PIN_D6 | DATA_PIN_D7;
  HAL_GPIO_Init(dataPort, &GPIO_InitStruct);

  // Configure data pin D2 on GPIOC as input
  GPIO_InitStruct.Pin = DATA_PIN_D2;
  HAL_GPIO_Init(dataPortC, &GPIO_InitStruct);
}

void EEPROMProgrammer::setDataBusOutput()
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

  // Configure data pins D0-D1, D3-D7 on GPIOB as outputs
  GPIO_InitStruct.Pin = DATA_PIN_D0 | DATA_PIN_D1 | DATA_PIN_D3 |
                        DATA_PIN_D4 | DATA_PIN_D5 | DATA_PIN_D6 | DATA_PIN_D7;
  HAL_GPIO_Init(dataPort, &GPIO_InitStruct);

  // Configure data pin D2 on GPIOC as output
  GPIO_InitStruct.Pin = DATA_PIN_D2;
  HAL_GPIO_Init(dataPortC, &GPIO_InitStruct);
}

void EEPROMProgrammer::setAddress(uint16_t address)
{
  // Set all 15 address pins (A0-A14) using direct register access for speed

  // A0-A4 (PA0-PA4)
  if (address & 0x0001)
    setPinHigh(addressPortA, ADDRESS_PIN_A0);
  else
    setPinLow(addressPortA, ADDRESS_PIN_A0);

  if (address & 0x0002)
    setPinHigh(addressPortA, ADDRESS_PIN_A1);
  else
    setPinLow(addressPortA, ADDRESS_PIN_A1);

  if (address & 0x0004)
    setPinHigh(addressPortA, ADDRESS_PIN_A2);
  else
    setPinLow(addressPortA, ADDRESS_PIN_A2);

  if (address & 0x0008)
    setPinHigh(addressPortA, ADDRESS_PIN_A3);
  else
    setPinLow(addressPortA, ADDRESS_PIN_A3);

  if (address & 0x0010)
    setPinHigh(addressPortA, ADDRESS_PIN_A4);
  else
    setPinLow(addressPortA, ADDRESS_PIN_A4);

  // A5-A9 (PA8-PA12)
  if (address & 0x0020)
    setPinHigh(addressPortA, ADDRESS_PIN_A5);
  else
    setPinLow(addressPortA, ADDRESS_PIN_A5);

  if (address & 0x0040)
    setPinHigh(addressPortA, ADDRESS_PIN_A6);
  else
    setPinLow(addressPortA, ADDRESS_PIN_A6);

  if (address & 0x0080)
    setPinHigh(addressPortA, ADDRESS_PIN_A7);
  else
    setPinLow(addressPortA, ADDRESS_PIN_A7);

  if (address & 0x0100)
    setPinHigh(addressPortA, ADDRESS_PIN_A8);
  else
    setPinLow(addressPortA, ADDRESS_PIN_A8);

  if (address & 0x0200)
    setPinHigh(addressPortA, ADDRESS_PIN_A9);
  else
    setPinLow(addressPortA, ADDRESS_PIN_A9);

  // A10-A14 (PB2-PB6)
  if (address & 0x0400)
    setPinHigh(addressPortB, ADDRESS_PIN_A10);
  else
    setPinLow(addressPortB, ADDRESS_PIN_A10);

  if (address & 0x0800)
    setPinHigh(addressPortB, ADDRESS_PIN_A11);
  else
    setPinLow(addressPortB, ADDRESS_PIN_A11);

  if (address & 0x1000)
    setPinHigh(addressPortB, ADDRESS_PIN_A12);
  else
    setPinLow(addressPortB, ADDRESS_PIN_A12);

  if (address & 0x2000)
    setPinHigh(addressPortB, ADDRESS_PIN_A13);
  else
    setPinLow(addressPortB, ADDRESS_PIN_A13);

  if (address & 0x4000)
    setPinHigh(addressPortB, ADDRESS_PIN_A14);
  else
    setPinLow(addressPortB, ADDRESS_PIN_A14);
}

uint8_t EEPROMProgrammer::readData()
{
  uint8_t data = 0;

  // Read each data pin and construct the byte
  if (readPin(dataPort, DATA_PIN_D0))
    data |= 0x01;
  if (readPin(dataPort, DATA_PIN_D1))
    data |= 0x02;
  if (readPin(dataPortC, DATA_PIN_D2))
    data |= 0x04;
  if (readPin(dataPort, DATA_PIN_D3))
    data |= 0x08;
  if (readPin(dataPort, DATA_PIN_D4))
    data |= 0x10;
  if (readPin(dataPort, DATA_PIN_D5))
    data |= 0x20;
  if (readPin(dataPort, DATA_PIN_D6))
    data |= 0x40;
  if (readPin(dataPort, DATA_PIN_D7))
    data |= 0x80;

  return data;
}

void EEPROMProgrammer::writeData(uint8_t data)
{
  // Write each bit to the corresponding data pin
  if (data & 0x01)
    setPinHigh(dataPort, DATA_PIN_D0);
  else
    setPinLow(dataPort, DATA_PIN_D0);

  if (data & 0x02)
    setPinHigh(dataPort, DATA_PIN_D1);
  else
    setPinLow(dataPort, DATA_PIN_D1);

  if (data & 0x04)
    setPinHigh(dataPortC, DATA_PIN_D2);
  else
    setPinLow(dataPortC, DATA_PIN_D2);

  if (data & 0x08)
    setPinHigh(dataPort, DATA_PIN_D3);
  else
    setPinLow(dataPort, DATA_PIN_D3);

  if (data & 0x10)
    setPinHigh(dataPort, DATA_PIN_D4);
  else
    setPinLow(dataPort, DATA_PIN_D4);

  if (data & 0x20)
    setPinHigh(dataPort, DATA_PIN_D5);
  else
    setPinLow(dataPort, DATA_PIN_D5);

  if (data & 0x40)
    setPinHigh(dataPort, DATA_PIN_D6);
  else
    setPinLow(dataPort, DATA_PIN_D6);

  if (data & 0x80)
    setPinHigh(dataPort, DATA_PIN_D7);
  else
    setPinLow(dataPort, DATA_PIN_D7);
}

bool EEPROMProgrammer::waitForWriteComplete(uint8_t expectedData)
{
  // Wait for the minimum write time before checking
  volatile uint32_t delay_count;
  for (delay_count = 0; delay_count < 10000; delay_count++)
  {
  }

  setDataBusInput();
  setPinLow(controlPort, EEPROM_OE_PIN);

  // Poll the data bus for write completion
  // During write, reading the same address should return the complement of written data
  // When write is complete, it returns the actual written data
  uint8_t currentData;
  int stableCount = 0;
  const int requiredStableReads = 3;

  for (int i = 0; i < 100; i++) // Timeout after ~100ms
  {
    currentData = readData();

    if (currentData == expectedData)
    {
      stableCount++;
      if (stableCount >= requiredStableReads)
      {
        setPinHigh(controlPort, EEPROM_OE_PIN);
        return true;
      }
    }
    else
    {
      stableCount = 0; // Reset counter if data doesn't match expected
    }

    // Small delay between reads
    for (delay_count = 0; delay_count < 100; delay_count++)
    {
    }
  }

  setPinHigh(controlPort, EEPROM_OE_PIN);
  return false;
}

void EEPROMProgrammer::disableSoftwareDataProtection()
{
  setPinHigh(controlPort, EEPROM_WE_PIN);
  setPinLow(controlPort, EEPROM_CE_PIN);

  setAddress(0x5555);
  writeData(0xAA);

  setPinLow(controlPort, EEPROM_WE_PIN);
  setPinHigh(controlPort, EEPROM_WE_PIN);

  setAddress(0x2AAA);
  writeData(0x55);

  setPinLow(controlPort, EEPROM_WE_PIN);
  setPinHigh(controlPort, EEPROM_WE_PIN);

  setAddress(0x5555);
  writeData(0x80);

  setPinLow(controlPort, EEPROM_WE_PIN);
  setPinHigh(controlPort, EEPROM_WE_PIN);

  setAddress(0x5555);
  writeData(0xAA);

  setPinLow(controlPort, EEPROM_WE_PIN);
  setPinHigh(controlPort, EEPROM_WE_PIN);

  setAddress(0x2AAA);
  writeData(0x55);

  setPinLow(controlPort, EEPROM_WE_PIN);
  setPinHigh(controlPort, EEPROM_WE_PIN);

  setAddress(0x5555);
  writeData(0x20);

  setPinLow(controlPort, EEPROM_WE_PIN);
  setPinHigh(controlPort, EEPROM_WE_PIN);
}

uint8_t EEPROMProgrammer::readByte(uint16_t address)
{
  setPinLow(controlPort, EEPROM_CE_PIN);
  setPinLow(controlPort, EEPROM_OE_PIN);

  setAddress(address);
  setDataBusInput();

  uint8_t data = readData();

  setPinHigh(controlPort, EEPROM_OE_PIN);
  setPinHigh(controlPort, EEPROM_CE_PIN);

  return data;
}

bool EEPROMProgrammer::writeByte(uint16_t address, uint8_t data)
{
  setDataBusOutput();
  setPinLow(controlPort, EEPROM_CE_PIN);

  // disableSoftwareDataProtection();

  setAddress(address);
  writeData(data);

  setPinLow(controlPort, EEPROM_WE_PIN);
  setPinHigh(controlPort, EEPROM_WE_PIN);

  // Wait for write completion
  bool success = waitForWriteComplete(data);

  if (!success)
  {
    blinkLED(1);
  }

  return success;
}

bool EEPROMProgrammer::writeDataBlock(uint16_t startAddress, const uint8_t *data, uint16_t length)
{
  for (uint16_t i = 0; i < length; i++)
  {
    if (!writeByte(startAddress + i, data[i]))
    {
      return false;
    }

    // Small delay between writes
    volatile uint32_t delay_count;
    for (delay_count = 0; delay_count < 100; delay_count++)
    {
    }
  }

  return verifyData(startAddress, data, length);
}

bool EEPROMProgrammer::verifyData(uint16_t startAddress, const uint8_t *data, uint16_t length)
{
  for (uint16_t i = 0; i < length; i++)
  {
    uint8_t readData = readByte(startAddress + i);
    if (readData != data[i])
    {
      return false;
    }
    // Small delay between read
    volatile uint32_t delay_count;
    for (delay_count = 0; delay_count < 100; delay_count++)
    {
    }
  }

  return true;
}

uint8_t *EEPROMProgrammer::dumpMemory(uint16_t startAddress, uint16_t length)
{
  // Allocate memory for the data array
  uint8_t *dataArray = new uint8_t[length];

  // Read data from EEPROM into the array
  for (uint16_t i = 0; i < length; i++)
  {
    dataArray[i] = readByte(startAddress + i);

    // Small delay between reads
    volatile uint32_t delay_count;
    for (delay_count = 0; delay_count < 100; delay_count++)
    {
    }
  }

  return dataArray;
}

void EEPROMProgrammer::blinkLED(int times)
{
  for (int i = 0; i < times; i++)
  {
    setPinLow(ledPort, STATUS_LED_PIN); // LED ON (active low)

    volatile uint32_t delay_count;
    for (delay_count = 0; delay_count < 100000; delay_count++)
    {
    }

    setPinHigh(ledPort, STATUS_LED_PIN); // LED OFF

    for (delay_count = 0; delay_count < 100000; delay_count++)
    {
    }
  }
}

// Helper functions
void EEPROMProgrammer::setPinHigh(GPIO_TypeDef *port, uint16_t pin)
{
  port->BSRR = pin;
}

void EEPROMProgrammer::setPinLow(GPIO_TypeDef *port, uint16_t pin)
{
  port->BSRR = pin << 16;
}

uint8_t EEPROMProgrammer::readPin(GPIO_TypeDef *port, uint16_t pin)
{
  return (port->IDR & pin) ? 1 : 0;
}
#include <Arduino.h>
#include <stdint.h>
#include "EEPROMProgrammer.h"

EEPROMProgrammer programmer;

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
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
};

// Program configuration
const unsigned int PROGRAM_START_ADDRESS = 0x0000; // Start address in EEPROM
const unsigned int PROGRAM_SIZE = sizeof(programData);

void programEEPROM()
{
  Serial.println("Starting EEPROM programming...");
  Serial.println("Writing program data...");
  if (!programmer.writeDataBlock(PROGRAM_START_ADDRESS, programData, PROGRAM_SIZE))
  {
    Serial.println("ERROR: Failed to write program data!");
    return;
  }

  Serial.println("Verifying program data...");
  if (!programmer.verifyData(PROGRAM_START_ADDRESS, programData, PROGRAM_SIZE))
  {
    Serial.println("ERROR: Program verification failed!");
    return;
  }

  Serial.println("SUCCESS: EEPROM programming completed!");
  programmer.blinkLED(5); // Success indicator
}

void setup()
{
  Serial.begin(9600);

  // Initialize the EEPROM programmer
  programmer.begin();

  Serial.println("EEPROM Programmer Ready");

  // programEEPROM();
  delay(1000);

  programmer.setDataBusInput();

  digitalWrite(programmer.EEPROM_CE_PIN, LOW);
  digitalWrite(programmer.EEPROM_OE_PIN, LOW);

  programmer.dumpMemory(0x0000, 0x001F);
}

void loop()
{

  delay(500);
}

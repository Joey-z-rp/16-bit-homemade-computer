#include <Arduino.h>
#include <stdint.h>
#include "EEPROMProgrammer.h"

EEPROMProgrammer programmer;

const uint8_t programData[] = {
    0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA, 0xF9, 0xF8};

// Program configuration
const unsigned int PROGRAM_START_ADDRESS = 0x0000; // Start address in EEPROM
const unsigned int PROGRAM_SIZE = sizeof(programData);

void setup()
{
  // Initialize serial communication
  Serial.begin(56000);
  Serial.println("EEPROM Programmer Starting...");

  // Initialize the EEPROM programmer
  programmer.begin();

  // Blink LED to indicate ready
  programmer.blinkLED(3);

  Serial.println("EEPROM Programmer Ready");
  Serial.print("Program size: ");
  Serial.print(PROGRAM_SIZE);
  Serial.println(" bytes");

  // Start programming process
  programEEPROM();
}

void loop()
{
  // Main loop - can be used for monitoring or additional functionality
  // The programming is done in setup(), so this can be empty or used for status updates

  // TODO: Read the data from the EEPROM
}

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
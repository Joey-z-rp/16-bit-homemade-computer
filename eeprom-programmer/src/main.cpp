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
  // Initialize serial communication
  Serial.begin(9600);
  // Serial.println("EEPROM Programmer Starting...");

  // Initialize the EEPROM programmer
  programmer.begin();

  // Blink LED to indicate ready
  // programmer.blinkLED(3);

  Serial.println("EEPROM Programmer Ready");
  // Serial.print("Program size: ");
  // Serial.print(PROGRAM_SIZE);
  // Serial.println(" bytes");

  // Start programming process
  // programEEPROM();
  // delay(1);

  // programmer.disableSoftwareDataProtection();
  // programmer.writeByte(0x1001, 0x13);
  // delay(100);
  // digitalWrite(programmer.EEPROM_OE_PIN, LOW);
  // digitalWrite(programmer.EEPROM_CE_PIN, HIGH);
  // digitalWrite(programmer.EEPROM_WE_PIN, LOW);

  // Serial.println(programmer.readByte(0x5555));
  // programmer.dumpMemory(0x0f00, 0x1100);
  // programmer.setAddress(0x1000);
  programmer.setDataBusInput();

  digitalWrite(programmer.EEPROM_CE_PIN, LOW);
  digitalWrite(programmer.EEPROM_OE_PIN, LOW);

  digitalWrite(A0, LOW);
  // digitalWrite(A1, LOW);
  delay(1000);
  // programmer.writeByte(0x1000, 0x0076);
}

void loop()
{

  digitalWrite(A0, LOW);
  Serial.println(programmer.readData(), HEX);
  digitalWrite(A0, HIGH);
  Serial.println(programmer.readData(), HEX);
  Serial.println("--------------------------------");

  delay(500);
}

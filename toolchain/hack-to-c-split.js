#!/usr/bin/env node

const fs = require("fs");

function convertHackToSplitC(hackFile) {
  try {
    // Read the hack file
    const content = fs.readFileSync(hackFile, "utf8");
    const lines = content.trim().split("\n");

    // Convert each line to lower and upper bytes
    const lowerBytes = [];
    const upperBytes = [];

    for (let i = 0; i < lines.length; i++) {
      const line = lines[i].trim();
      if (line.length === 16) {
        // Convert binary to 16-bit integer
        const instruction = parseInt(line, 2);

        // Split into lower and upper bytes
        const lowerByte = instruction & 0xff; // Bits 0-7
        const upperByte = (instruction >> 8) & 0xff; // Bits 8-15

        lowerBytes.push(
          `0x${lowerByte.toString(16).toUpperCase().padStart(2, "0")}`
        );
        upperBytes.push(
          `0x${upperByte.toString(16).toUpperCase().padStart(2, "0")}`
        );
      } else {
        throw new Error(`Invalid instruction: ${line}`);
      }
    }

    // Generate C arrays
    const cArrays = `// Converted from ${hackFile}
// Lower 8 bits of each instruction (bits 0-7)
const uint8_t hackProgramLower[] = {
    ${lowerBytes.join(",\n    ")}
};

// Upper 8 bits of each instruction (bits 8-15)
const uint8_t hackProgramUpper[] = {
    ${upperBytes.join(",\n    ")}
};

const uint16_t HACK_PROGRAM_SIZE = ${lowerBytes.length};
`;

    // Write output file
    fs.writeFileSync(hackFile.replace(".hack", "-split.h"), cArrays);

    console.log(`Converted ${lowerBytes.length} instructions from ${hackFile}`);
  } catch (error) {
    console.error("Error:", error.message);
    process.exit(1);
  }
}

// Command line usage
if (process.argv.length !== 3) {
  console.log("Usage: node hack-to-c-split.js <input.hack>");
  console.log("Example: node hack-to-c-split.js simple-loop.hack");
  process.exit(1);
}

const inputFile = process.argv[2];

convertHackToSplitC(inputFile);

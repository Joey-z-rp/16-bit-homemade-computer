#!/usr/bin/env node

const fs = require("fs");
const path = require("path");
const { execSync } = require("child_process");

class BuildPipeline {
  constructor() {
    this.config = {
      jackCompiler: "jack-compiler/cli.ts",
      vmTranslator: "vm-translator.js",
      assembler: "assembler.js",
      hackSplitter: "hack-to-c-split.js",
      stm32Programmer: "../stm32-eeprom-programmer",
      outputDir: "build",
    };
  }

  log(message, type = "INFO") {
    const timestamp = new Date().toISOString();
    console.log(`[${timestamp}] [${type}] ${message}`);
  }

  error(message) {
    this.log(message, "ERROR");
    process.exit(1);
  }

  success(message) {
    this.log(message, "SUCCESS");
  }

  ensureDirectory(dir) {
    if (!fs.existsSync(dir)) {
      fs.mkdirSync(dir, { recursive: true });
      this.log(`Created directory: ${dir}`);
    }
  }

  runCommand(command, description) {
    try {
      this.log(`Running: ${description}`);
      const result = execSync(command, {
        cwd: __dirname,
        encoding: "utf8",
        stdio: "pipe",
      });
      this.log(`Completed: ${description}`);
      return result;
    } catch (error) {
      this.error(`Failed: ${description}\n${error.message}`);
    }
  }

  sanitizeProgramName(name) {
    // Replace invalid characters with underscores and ensure it starts with a letter
    let sanitized = name.replace(/[^a-zA-Z0-9_]/g, "_");

    // Ensure it starts with a letter (C++ requirement)
    if (/^[0-9]/.test(sanitized)) {
      sanitized = "Program_" + sanitized;
    }

    // Remove consecutive underscores
    sanitized = sanitized.replace(/_+/g, "_");

    // Remove leading/trailing underscores
    sanitized = sanitized.replace(/^_+|_+$/g, "");

    return sanitized;
  }

  async buildFromInput(inputFileOrDir) {
    this.log("Starting build pipeline...");

    this.ensureDirectory(this.config.outputDir);

    const inputPath = path.resolve(inputFileOrDir);
    const inputName = path.basename(inputPath, path.extname(inputPath));
    const sanitizedName = this.sanitizeProgramName(inputName);
    const inputExt = path.extname(inputPath).toLowerCase();

    this.log(`Processing input: ${inputPath}`);
    this.log(`Sanitized program name: ${sanitizedName}`);

    let hackFile;

    if (inputExt === ".asm") {
      // Assembly file provided - skip compilation and translation
      this.log("Assembly file detected - skipping compilation and translation");

      // Step 1: Assemble to Machine Code
      this.log("Step 1: Assembling to Machine Code");
      const assemblerCmd = `node ${this.config.assembler} "${inputPath}" "${this.config.outputDir}"`;
      this.runCommand(assemblerCmd, "Assembly");

      // Get the generated hack file (assembler uses original filename)
      hackFile = path.join(this.config.outputDir, `${inputName}.hack`);
    } else {
      // Jack file provided - run full pipeline
      // Step 1: Compile Jack to VM
      this.log("Step 1: Compiling Jack to VM code");
      const jackCompilerCmd = `npx ts-node ${this.config.jackCompiler} "${inputPath}" "${this.config.outputDir}"`;
      this.runCommand(jackCompilerCmd, "Jack compilation");

      // VM file is now generated directly in build directory
      const vmFile = path.join(this.config.outputDir, `${inputName}.vm`);
      this.log(`VM file generated in build: ${vmFile}`);

      // Step 2: Translate VM to Assembly
      this.log("Step 2: Translating VM to Assembly");
      // Check if we have multiple VM files (directory input) or single VM file
      const vmFiles = fs
        .readdirSync(this.config.outputDir)
        .filter((file) => file.endsWith(".vm"));

      if (vmFiles.length > 1) {
        // Multiple VM files - pass directory to VM translator
        const vmTranslatorCmd = `node ${this.config.vmTranslator} "${this.config.outputDir}" "${this.config.outputDir}"`;
        this.runCommand(vmTranslatorCmd, "VM translation");

        // VM translator uses directory name for output
        const asmFile = path.join(this.config.outputDir, `build.asm`);
        this.log(`Assembly file generated in build: ${asmFile}`);

        // Step 3: Assemble to Machine Code
        this.log("Step 3: Assembling to Machine Code");
        const assemblerCmd = `node ${this.config.assembler} "${asmFile}" "${this.config.outputDir}"`;
        this.runCommand(assemblerCmd, "Assembly");

        // Assembler uses assembly filename
        hackFile = path.join(this.config.outputDir, `build.hack`);
      } else {
        // Single VM file - pass the specific file
        const vmFile = path.join(this.config.outputDir, `${inputName}.vm`);
        const vmTranslatorCmd = `node ${this.config.vmTranslator} "${vmFile}" "${this.config.outputDir}"`;
        this.runCommand(vmTranslatorCmd, "VM translation");

        // VM translator uses original filename
        const asmFile = path.join(this.config.outputDir, `${inputName}.asm`);
        this.log(`Assembly file generated in build: ${asmFile}`);

        // Step 3: Assemble to Machine Code
        this.log("Step 3: Assembling to Machine Code");
        const assemblerCmd = `node ${this.config.assembler} "${asmFile}" "${this.config.outputDir}"`;
        this.runCommand(assemblerCmd, "Assembly");

        // Assembler uses original filename
        hackFile = path.join(this.config.outputDir, `${inputName}.hack`);
      }
    }

    this.log(`Hack file generated in build: ${hackFile}`);

    // Step 4: Split Machine Code
    this.log("Step 4: Splitting machine code for EEPROM");
    const splitterCmd = `node ${this.config.hackSplitter} "${hackFile}" "${this.config.outputDir}"`;
    this.runCommand(splitterCmd, "Machine code splitting");

    // Step 5: Generate EEPROM Programming Files
    this.log("Step 5: Generating EEPROM programming files");
    this.generateEEPROMFiles(hackFile, sanitizedName);

    this.success("Build pipeline completed successfully!");
    this.log(
      `Output files available in: ${path.resolve(this.config.outputDir)}`
    );
  }

  generateEEPROMFiles(hackFile, programName) {
    try {
      // Read the hack file
      const content = fs.readFileSync(hackFile, "utf8");
      const lines = content.trim().split("\n");

      // Convert to upper and lower bytes
      const lowerBytes = [];
      const upperBytes = [];

      for (let i = 0; i < lines.length; i++) {
        const line = lines[i].trim();
        if (line.length === 16) {
          const instruction = parseInt(line, 2);
          const lowerByte = instruction & 0xff;
          const upperByte = (instruction >> 8) & 0xff;

          lowerBytes.push(lowerByte);
          upperBytes.push(upperByte);
        }
      }

      // Generate single STM32 file with both halves
      this.generateSTM32File(programName, lowerBytes, upperBytes);

      this.log(`Generated EEPROM files for ${lowerBytes.length} instructions`);
    } catch (error) {
      this.error(`Failed to generate EEPROM files: ${error.message}`);
    }
  }

  generateSTM32File(programName, lowerBytes, upperBytes) {
    const outputFile = path.join(this.config.outputDir, `${programName}.cpp`);

    const cppContent = `// Auto-generated EEPROM programming file for ${programName}
// Generated on: ${new Date().toISOString()}

#include "stm32f1xx_hal.h"
#include "EEPROMProgrammer.h"

// Global EEPROM programmer instance
EEPROMProgrammer eeprom;

// Program data for lower half (bits 0-7)
const uint8_t ${programName}ProgramLower[] = {
${lowerBytes
  .map((byte, index) => {
    const hex = `0x${byte.toString(16).toUpperCase().padStart(2, "0")}`;
    return `    ${hex}${index < lowerBytes.length - 1 ? "," : ""}`;
  })
  .join("\n")}
};

// Program data for upper half (bits 8-15)
const uint8_t ${programName}ProgramUpper[] = {
${upperBytes
  .map((byte, index) => {
    const hex = `0x${byte.toString(16).toUpperCase().padStart(2, "0")}`;
    return `    ${hex}${index < upperBytes.length - 1 ? "," : ""}`;
  })
  .join("\n")}
};

const uint16_t ${programName.toUpperCase()}_PROGRAM_SIZE = ${lowerBytes.length};

// Programming configuration
const unsigned int PROGRAM_START_ADDRESS = 0x0000;
const unsigned int PROGRAM_SIZE = sizeof(${programName}ProgramLower);

// Control flags
const bool UPLOAD_ENABLED = false;  // Set to true to upload, false to verify only
const bool UPLOAD_LOWER = true;     // Set to true to upload lower half, false for upper half

int main(void)
{
  // Initialize EEPROM programmer
  eeprom.begin();

  // Simple delay function
  volatile uint32_t delay_count;

  if (UPLOAD_ENABLED) {
    // Upload program to EEPROM
    volatile bool writeSuccess;
    
    if (UPLOAD_LOWER) {
      writeSuccess = eeprom.writeDataBlock(PROGRAM_START_ADDRESS, ${programName}ProgramLower, PROGRAM_SIZE);
    } else {
      writeSuccess = eeprom.writeDataBlock(PROGRAM_START_ADDRESS, ${programName}ProgramUpper, PROGRAM_SIZE);
    }
    
    if (writeSuccess) {
      // Upload successful - LED stays on
      eeprom.setPinHigh(eeprom.ledPort, eeprom.STATUS_LED_PIN);
    } else {
      // Upload failed - LED blinks rapidly
      while (1) {
        eeprom.blinkLED(1);
        for (delay_count = 0; delay_count < 500000; delay_count++) {}
      }
    }
  } else {
    // Verify program data
    volatile bool verified;
    
    if (UPLOAD_LOWER) {
      verified = eeprom.verifyData(PROGRAM_START_ADDRESS, ${programName}ProgramLower, PROGRAM_SIZE);
    } else {
      verified = eeprom.verifyData(PROGRAM_START_ADDRESS, ${programName}ProgramUpper, PROGRAM_SIZE);
    }
    
    if (verified) {
      // Verification successful - LED stays on
      eeprom.setPinHigh(eeprom.ledPort, eeprom.STATUS_LED_PIN);
    } else {
      // Verification failed - LED blinks rapidly
      while (1) {
        eeprom.blinkLED(1);
        for (delay_count = 0; delay_count < 500000; delay_count++) {}
      }
    }
  }

  // Success - LED stays on
  while (1) {
    for (delay_count = 0; delay_count < 2000000; delay_count++) {}
  }
}
`;

    fs.writeFileSync(outputFile, cppContent);
    this.log(`Generated STM32 file: ${outputFile}`);
  }

  copyToSTM32Programmer(programName) {
    this.log(`Copying program to STM32 programmer...`);

    // Copy the generated STM32 file to the programmer
    const sourceFile = path.join(this.config.outputDir, `${programName}.cpp`);
    const targetFile = path.join(
      this.config.stm32Programmer,
      "src",
      "main.cpp"
    );

    if (fs.existsSync(sourceFile)) {
      fs.copyFileSync(sourceFile, targetFile);
      this.log(`Copied program to STM32 programmer: ${targetFile}`);
      this.log(
        `To upload: Set UPLOAD_ENABLED = true and UPLOAD_LOWER = true/false in the copied file and flash to STM32`
      );
    } else {
      this.error(`Source file not found: ${sourceFile}`);
    }
  }

  showUsage() {
    console.log(`
16-bit Computer Build Pipeline
==============================

Usage:
  node build-pipeline.js <input-file> [options]

Input files:
  .jack file or directory - Full pipeline (Jack → VM → Assembly → Machine Code)
  .asm file             - Assembly only (Assembly → Machine Code)

Options:
  --copy            Copy program to STM32 programmer after build
  --help           Show this help message

Examples:
  node build-pipeline.js myprogram.jack
  node build-pipeline.js myprogram.jack --copy
  node build-pipeline.js myprogram.asm --copy
  node build-pipeline.js src/ --copy

The pipeline will:
For .jack files:
1. Compile Jack code to VM
2. Translate VM to Assembly
3. Assemble to Machine Code
4. Split into upper/lower bytes for EEPROM
5. Generate STM32 programming file with both halves
6. Optionally copy to STM32 programmer

For .asm files:
1. Assemble to Machine Code
2. Split into upper/lower bytes for EEPROM
3. Generate STM32 programming file with both halves
4. Optionally copy to STM32 programmer
`);
  }
}

// Main execution
async function main() {
  const pipeline = new BuildPipeline();
  const args = process.argv.slice(2);

  if (args.length === 0 || args.includes("--help")) {
    pipeline.showUsage();
    return;
  }

  const inputPath = args[0];
  const shouldCopy = args.includes("--copy");

  try {
    await pipeline.buildFromInput(inputPath);

    const programName = path.basename(inputPath, path.extname(inputPath));
    const sanitizedName = pipeline.sanitizeProgramName(programName);

    if (shouldCopy) {
      pipeline.copyToSTM32Programmer(sanitizedName);
    }

    pipeline.success("Build pipeline completed!");
  } catch (error) {
    pipeline.error(`Build failed: ${error.message}`);
  }
}

if (require.main === module) {
  main();
}

module.exports = BuildPipeline;

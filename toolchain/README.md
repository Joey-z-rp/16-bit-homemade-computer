# 16-bit Computer Build Pipeline

This automated build pipeline streamlines the process of going from Jack code to EEPROM programming for the 16-bit homemade computer.

## Overview

The pipeline automates the entire flow and supports two input types:

### For Jack files (.jack):

1. **Jack Compilation** → VM code
2. **VM Translation** → Assembly code
3. **Assembly** → Machine code (.hack)
4. **Code Splitting** → Upper/lower bytes for EEPROM
5. **EEPROM Programming** → Upload to chips

### For Assembly files (.asm):

1. **Assembly** → Machine code (.hack)
2. **Code Splitting** → Upper/lower bytes for EEPROM
3. **EEPROM Programming** → Upload to chips

## Usage

### Basic Build

```bash
# Build from a single Jack file
node build-pipeline.js myprogram.jack

# Build from a directory containing Jack files
node build-pipeline.js src/

# Build from an assembly file (skips compilation and translation)
node build-pipeline.js myprogram.asm
```

### Build with STM32 Copy

```bash
# Copy program to STM32 programmer
node build-pipeline.js myprogram.jack --copy
node build-pipeline.js myprogram.asm --copy
```

## Output Files

The pipeline generates output files in the `build/` directory:

- `program.cpp` - STM32 code with both upper and lower halves
- `program.vm` - VM code (for Jack files only)
- `program.asm` - Assembly code
- `program.hack` - Machine code

Note: File names are sanitized for valid C++ variable names (e.g., `my-program-123` becomes `my_program_123`).

## Manual Steps (if needed)

If you prefer to run steps manually:

### 1. Jack to VM

```bash
# Output to same directory as input
npx ts-node jack-compiler/cli.ts myprogram.jack

# Output to specified directory
npx ts-node jack-compiler/cli.ts myprogram.jack output-directory/
```

### 2. VM to Assembly

```bash
node vm-translator.js myprogram.vm
```

### 3. Assembly to Machine Code

```bash
node assembler.js myprogram.asm
```

### 4. Split for EEPROM

```bash
node hack-to-c-split.js myprogram.hack
```

**Note**: When used manually, scripts output to the same directory as the input file. When used by the pipeline, they output to the `build/` directory.

## STM32 EEPROM Programming

### Hardware Setup

- Connect two 28C256 EEPROM chips
- Lower chip: bits 0-7 of each instruction
- Upper chip: bits 8-15 of each instruction
- Use the STM32 EEPROM programmer

### Programming Process

1. Run the build pipeline with `--copy`
2. The pipeline will:
   - Generate STM32 code with both halves
   - Copy to the STM32 programmer directory
   - Set `UPLOAD_ENABLED = false` and `UPLOAD_LOWER = true` by default

### Manual Programming

To upload to EEPROM:

1. The generated file is copied to `stm32-eeprom-programmer/src/main.cpp`
2. Set `UPLOAD_ENABLED = true` in the copied file
3. Set `UPLOAD_LOWER = true` for lower half or `UPLOAD_LOWER = false` for upper half
4. Flash the STM32 with your preferred method
5. LED behavior:
   - **Solid ON**: Upload/verification successful
   - **Rapid blinking**: Upload/verification failed

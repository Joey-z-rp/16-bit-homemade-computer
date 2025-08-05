#!/usr/bin/env node
const fs = require("fs");
const path = require("path");

const COMP = {
  0: "0101010",
  1: "0111111",
  "-1": "0111010",
  D: "0001100",
  A: "0110000",
  M: "1110000",
  "!D": "0001101",
  "!A": "0110001",
  "!M": "1110001",
  "-D": "0001111",
  "-A": "0110011",
  "-M": "1110011",
  "D+1": "0011111",
  "A+1": "0110111",
  "M+1": "1110111",
  "D-1": "0001110",
  "A-1": "0110010",
  "M-1": "1110010",
  "D+A": "0000010",
  "D+M": "1000010",
  "D-A": "0010011",
  "D-M": "1010011",
  "A-D": "0000111",
  "M-D": "1000111",
  "D&A": "0000000",
  "D&M": "1000000",
  "D|A": "0010101",
  "D|M": "1010101",
};

const DEST = {
  null: "000",
  M: "001",
  D: "010",
  MD: "011",
  A: "100",
  AM: "101",
  AD: "110",
  AMD: "111",
};

const JUMP = {
  null: "000",
  JGT: "001",
  JEQ: "010",
  JGE: "011",
  JLT: "100",
  JNE: "101",
  JLE: "110",
  JMP: "111",
};

const predefinedSymbols = {
  SP: 0,
  LCL: 1,
  ARG: 2,
  THIS: 3,
  THAT: 4,
  SCREEN: 16384,
  KBD: 24576,
};

for (let i = 0; i <= 15; i++) {
  predefinedSymbols[`R${i}`] = i;
}

function parse(lines) {
  return lines
    .map((line) => line.split("//")[0].trim())
    .filter((line) => line.length > 0);
}

function firstPass(lines, symbolTable) {
  let romAddress = 0;
  return lines.filter((line) => {
    if (line.startsWith("(")) {
      const label = line.slice(1, -1);
      symbolTable[label] = romAddress;
      return false;
    }
    romAddress++;
    return true;
  });
}

function toBinary(value) {
  return value.toString(2).padStart(16, "0");
}

function assemble(lines, symbolTable) {
  let nextVariableAddress = 16;
  const output = [];

  for (const line of lines) {
    if (line.startsWith("@")) {
      const symbol = line.slice(1);
      let address;
      if (/^\d+$/.test(symbol)) {
        address = parseInt(symbol);
      } else {
        if (!(symbol in symbolTable)) {
          symbolTable[symbol] = nextVariableAddress;
          nextVariableAddress++;
        }
        address = symbolTable[symbol];
      }
      output.push(toBinary(address));
    } else {
      let dest = "null",
        comp = "",
        jump = "null";
      let [left, right] = line.split("=");
      if (right !== undefined) {
        dest = left.trim();
        comp = right.trim();
      } else {
        [comp, jump] = line.split(";").map((s) => s.trim());
      }

      const bin = "111" + COMP[comp] + DEST[dest] + JUMP[jump];
      output.push(bin);
    }
  }
  return output;
}

function main() {
  const [, , filepath, outputDir] = process.argv;
  if (!filepath) {
    console.error(
      "Usage: node assembler.js [path/to/file.asm] [output-directory]"
    );
    process.exit(1);
  }

  const raw = fs.readFileSync(filepath, "utf8").split("\n");
  const symbolTable = { ...predefinedSymbols };
  const parsed = parse(raw);
  const withoutLoopLabels = firstPass(parsed, symbolTable);
  const binary = assemble(withoutLoopLabels, symbolTable);

  // Determine output path
  let outputPath;
  if (outputDir) {
    // Use specified output directory
    const filename = path.basename(filepath, ".asm") + ".hack";
    outputPath = path.join(outputDir, filename);
  } else {
    // Use same directory as input (default behavior)
    outputPath = filepath.replace(/\.asm$/, ".hack");
  }

  fs.writeFileSync(outputPath, binary.join("\n"), "utf8");
  console.log(`Assembled: ${outputPath}`);
}

main();

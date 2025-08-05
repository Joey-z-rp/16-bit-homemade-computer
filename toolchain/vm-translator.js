const fs = require("fs");
const path = require("path");

class Parser {
  constructor(filePath) {
    this.lines = fs
      .readFileSync(filePath, "utf8")
      .split(/\r?\n/)
      .map((line) => line.split("//")[0].trim()) // Remove comments
      .filter((line) => line.length > 0); // Remove empty lines
    this.currentIndex = 0;
  }

  hasMoreCommands() {
    return this.currentIndex < this.lines.length;
  }

  advance() {
    this.currentCommand = this.lines[this.currentIndex];
    this.currentIndex++;
  }

  commandType() {
    if (this.currentCommand.startsWith("push")) return "C_PUSH";
    if (this.currentCommand.startsWith("pop")) return "C_POP";
    if (
      ["add", "sub", "neg", "eq", "gt", "lt", "and", "or", "not"].includes(
        this.currentCommand
      )
    )
      return "C_ARITHMETIC";
    if (this.currentCommand.startsWith("label")) return "C_LABEL";
    if (this.currentCommand.startsWith("goto")) return "C_GOTO";
    if (this.currentCommand.startsWith("if-goto")) return "C_IF";
    if (this.currentCommand.startsWith("function")) return "C_FUNCTION";
    if (this.currentCommand.startsWith("call")) return "C_CALL";
    if (this.currentCommand.startsWith("return")) return "C_RETURN";
    return null;
  }

  arg1() {
    if (this.commandType() === "C_ARITHMETIC") return this.currentCommand;
    return this.currentCommand.split(" ")[1];
  }

  arg2() {
    const parts = this.currentCommand.split(" ");
    return parseInt(parts[2]);
  }
}

class CodeWriter {
  static segmentToAddress = {
    local: "LCL",
    argument: "ARG",
    this: "THIS",
    that: "THAT",
  };

  constructor(outputPath) {
    this.file = fs.createWriteStream(outputPath, { flags: "w" });
    this.outputFileName = outputPath.split("/").pop().split(".")[0];
    this.currentFunction = ""; // Track current function
    this.functionCallCount = {}; // Count of calls per function
  }

  initialCommand() {
    this.file.write(`@261\nD=A\n@SP\nM=D\n`);
    this.file.write(`@261\nD=A\n@LCL\nM=D\n`);
    this.file.write(`@256\nD=A\n@ARG\nM=D\n`);
    this.file.write(`@Sys.init\n0;JMP\n`);
  }

  writePush(arg1, arg2, processingFileName) {
    let asm = "";
    switch (arg1) {
      case "local":
      case "argument":
      case "this":
      case "that":
        asm += `@${CodeWriter.segmentToAddress[arg1]}\nD=M\n@${arg2}\nA=D+A\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n`;
        break;
      case "constant":
        asm += `@${arg2}\nD=A\n@SP\nA=M\nM=D\n@SP\nM=M+1\n`;
        break;
      case "static":
        asm += `@${processingFileName}.${arg2}\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n`;
        break;
      case "temp":
        asm += `@${arg2 + 5}\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n`;
        break;
      case "pointer":
        asm += `@${arg2 + 3}\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n`;
        break;
    }
    return asm;
  }

  writePop(arg1, arg2, processingFileName) {
    let asm = "";
    switch (arg1) {
      case "local":
      case "argument":
      case "this":
      case "that":
        asm += `@${CodeWriter.segmentToAddress[arg1]}\nD=M\n@${arg2}\nD=D+A\n@R13\nM=D\n@SP\nAM=M-1\nD=M\n@R13\nA=M\nM=D\n`;
        break;
      case "static":
        asm += `@${processingFileName}.${arg2}\nD=A\n@R13\nM=D\n@SP\nAM=M-1\nD=M\n@R13\nA=M\nM=D\n`;
        break;
      case "temp":
        asm += `@${
          arg2 + 5
        }\nD=A\n@R13\nM=D\n@SP\nAM=M-1\nD=M\n@R13\nA=M\nM=D\n`;
        break;
      case "pointer":
        asm += `@${
          arg2 + 3
        }\nD=A\n@R13\nM=D\n@SP\nAM=M-1\nD=M\n@R13\nA=M\nM=D\n`;
        break;
    }
    return asm;
  }

  writeArithmetic(arg1, index, processingFileName) {
    let asm = "";
    switch (arg1) {
      case "add":
        asm += `@SP\nAM=M-1\nD=M\n@SP\nAM=M-1\nD=D+M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n`;
        break;
      case "sub":
        asm += `@SP\nAM=M-1\nD=M\n@SP\nAM=M-1\nD=M-D\n@SP\nA=M\nM=D\n@SP\nM=M+1\n`;
        break;
      case "neg":
        asm += `@SP\nA=M-1\nM=-M\n`;
        break;
      case "eq":
        asm += `@SP\nAM=M-1\nD=M\n@SP\nAM=M-1\nD=M-D\n@TRUE_EQ_${processingFileName}_${index}\nD;JEQ\n@SP\nA=M\nM=0\n@END_EQ_${processingFileName}_${index}\n0;JMP\n(TRUE_EQ_${processingFileName}_${index})\n@SP\nA=M\nM=-1\n(END_EQ_${processingFileName}_${index})\n@SP\nM=M+1\n`;
        break;
      case "gt":
        asm += `@SP\nAM=M-1\nD=M\n@SP\nAM=M-1\nD=M-D\n@TRUE_GT_${processingFileName}_${index}\nD;JGT\n@SP\nA=M\nM=0\n@END_GT_${processingFileName}_${index}\n0;JMP\n(TRUE_GT_${processingFileName}_${index})\n@SP\nA=M\nM=-1\n(END_GT_${processingFileName}_${index})\n@SP\nM=M+1\n`;
        break;
      case "lt":
        asm += `@SP\nAM=M-1\nD=M\n@SP\nAM=M-1\nD=M-D\n@TRUE_LT_${processingFileName}_${index}\nD;JLT\n@SP\nA=M\nM=0\n@END_LT_${processingFileName}_${index}\n0;JMP\n(TRUE_LT_${processingFileName}_${index})\n@SP\nA=M\nM=-1\n(END_LT_${processingFileName}_${index})\n@SP\nM=M+1\n`;
        break;
      case "and":
        asm += `@SP\nAM=M-1\nD=M\n@SP\nAM=M-1\nD=D&M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n`;
        break;
      case "or":
        asm += `@SP\nAM=M-1\nD=M\n@SP\nAM=M-1\nD=D|M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n`;
        break;
      case "not":
        asm += `@SP\nAM=M-1\nM=!M\n@SP\nM=M+1\n`;
        break;
    }
    return asm;
  }

  writeLabel(arg1) {
    return `(${
      this.currentFunction ? `${this.currentFunction}$` : ""
    }${arg1})\n`;
  }

  writeGoto(arg1) {
    return `@${
      this.currentFunction ? `${this.currentFunction}$` : ""
    }${arg1}\n0;JMP\n`;
  }

  writeIf(arg1) {
    return `@SP\nAM=M-1\nD=M\n@${
      this.currentFunction ? `${this.currentFunction}$` : ""
    }${arg1}\nD;JNE\n`;
  }

  writeFunction(arg1, arg2) {
    let asm = "";
    const functionName = arg1;
    this.currentFunction = functionName; // Set current function
    this.functionCallCount[functionName] = 0;
    asm += `(${functionName})\n`;
    for (let i = 0; i < arg2; i++) {
      asm += `@SP\nA=M\nM=0\n@SP\nM=M+1\n`;
    }
    return asm;
  }

  writeCall(arg1, arg2) {
    let asm = "";
    const returnAddressLabel = `${this.currentFunction}$ret.${
      this.functionCallCount[this.currentFunction]
    }`;
    this.functionCallCount[this.currentFunction]++;
    asm += `@${returnAddressLabel}\nD=A\n@SP\nA=M\nM=D\n@SP\nM=M+1\n`;
    asm += `@LCL\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n`;
    asm += `@ARG\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n`;
    asm += `@THIS\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n`;
    asm += `@THAT\nD=M\n@SP\nA=M\nM=D\n@SP\nM=M+1\n`;
    asm += `@SP\nD=M\n@5\nD=D-A\n@${arg2}\nD=D-A\n@ARG\nM=D\n`;
    asm += `@SP\nD=M\n@LCL\nM=D\n`;
    asm += `@${arg1}\n0;JMP\n`;
    asm += `(${returnAddressLabel})\n`;
    return asm;
  }

  writeReturn() {
    let asm = "";
    asm += `@LCL\nD=M\n@R13\nM=D\n`;
    asm += `@R13\nD=M\n@5\nA=D-A\nD=M\n@R14\nM=D\n`;
    asm += `@SP\nAM=M-1\nD=M\n@ARG\nA=M\nM=D\n`;
    asm += `@ARG\nD=M\n@SP\nM=D+1\n`;
    asm += `@R13\nD=M\n@1\nA=D-A\nD=M\n@THAT\nM=D\n`;
    asm += `@R13\nD=M\n@2\nA=D-A\nD=M\n@THIS\nM=D\n`;
    asm += `@R13\nD=M\n@3\nA=D-A\nD=M\n@ARG\nM=D\n`;
    asm += `@R13\nD=M\n@4\nA=D-A\nD=M\n@LCL\nM=D\n`;
    asm += `@R14\nA=M\n0;JMP\n`;
    return asm;
  }

  writeCommand({
    commandType,
    command,
    arg1,
    arg2,
    index,
    processingFileName,
  }) {
    let asm = `// ${command}\n`;

    switch (commandType) {
      case "C_PUSH":
        asm += this.writePush(arg1, arg2, processingFileName);
        break;
      case "C_POP":
        asm += this.writePop(arg1, arg2, processingFileName);
        break;
      case "C_ARITHMETIC":
        asm += this.writeArithmetic(arg1, index, processingFileName);
        break;
      case "C_LABEL":
        asm += this.writeLabel(arg1);
        break;
      case "C_GOTO":
        asm += this.writeGoto(arg1);
        break;
      case "C_IF":
        asm += this.writeIf(arg1);
        break;
      case "C_FUNCTION":
        asm += this.writeFunction(arg1, arg2);
        break;
      case "C_CALL":
        asm += this.writeCall(arg1, arg2);
        break;
      case "C_RETURN":
        asm += this.writeReturn();
        break;
    }

    this.file.write(asm + "\n");
  }

  close() {
    this.file.end();
  }
}

function processFile(inputFile, writer, index) {
  if (index === 0) {
    writer.initialCommand();
  }

  const parser = new Parser(inputFile);

  while (parser.hasMoreCommands()) {
    parser.advance();
    const type = parser.commandType();
    const arg1 = type === "C_RETURN" ? null : parser.arg1();
    const arg2 = ["C_PUSH", "C_POP", "C_FUNCTION", "C_CALL"].includes(type)
      ? parser.arg2()
      : null;
    writer.writeCommand({
      commandType: type,
      command: parser.currentCommand,
      arg1,
      arg2,
      index: parser.currentIndex - 1,
      processingFileName: inputFile.split("/").pop().split(".")[0],
    });
  }
}

const inputPath = process.argv[2];
const outputDir = process.argv[3]; // Optional output directory

if (!inputPath) {
  console.error(
    "Usage: node vm-translator.js [path/to/file.vm] [output-directory]"
  );
  process.exit(1);
}

const isDirectory = fs.statSync(inputPath).isDirectory();

if (isDirectory) {
  // If input is a directory, process all .vm files
  const dirName = path.basename(inputPath);
  let outputFile;
  if (outputDir) {
    outputFile = path.join(outputDir, `${dirName}.asm`);
  } else {
    outputFile = path.join(inputPath, `${dirName}.asm`);
  }
  const writer = new CodeWriter(outputFile);

  const files = fs
    .readdirSync(inputPath)
    .filter((file) => file.endsWith(".vm"))
    .map((file) => path.join(inputPath, file));

  files.forEach((file, index) => {
    processFile(file, writer, index);
  });

  writer.close();
} else {
  // If input is a single file
  let outputFile;
  if (outputDir) {
    const filename = path.basename(inputPath, ".vm") + ".asm";
    outputFile = path.join(outputDir, filename);
  } else {
    outputFile = inputPath.replace(/\.vm$/, ".asm");
  }
  const writer = new CodeWriter(outputFile);
  processFile(inputPath, writer, 0);
  writer.close();
}

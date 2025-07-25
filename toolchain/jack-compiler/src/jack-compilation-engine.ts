import { JackTokenizer } from "./jack-tokenizer";
import { SymbolTable } from "./symbol-table";
import { VMWriter } from "./vm-writer";
import * as fs from "fs";

export class JackCompilationEngine {
  private tokenizer: JackTokenizer;
  private vmWriter: VMWriter;
  private symbolTable: SymbolTable;
  private currentClassName: string;
  private currentSubroutineName: string;
  private currentSubroutineKind: string | null;
  private labelCounter: number;
  private compilingClassName: string;

  constructor(source: string, private outFilePath: string) {
    this.tokenizer = new JackTokenizer(source);
    this.symbolTable = new SymbolTable();
    this.vmWriter = new VMWriter();
    this.currentClassName = "";
    this.currentSubroutineName = "";
    this.currentSubroutineKind = null;
    this.labelCounter = 0;
    this.compilingClassName = "";
  }

  private getNextLabel(): string {
    return `${this.compilingClassName}_${this.labelCounter++}`;
  }

  private isKeyword(keyword: string): boolean {
    return (
      this.tokenizer.tokenType() === "KEYWORD" &&
      this.tokenizer.keyword().toLowerCase() === keyword
    );
  }

  private isOneOfKeywords(keywords: string[]): boolean {
    return (
      this.tokenizer.tokenType() === "KEYWORD" &&
      keywords.includes(this.tokenizer.keyword().toLowerCase())
    );
  }

  private isOneOfSymbols(symbols: string[]): boolean {
    return (
      this.tokenizer.tokenType() === "SYMBOL" &&
      symbols.includes(this.tokenizer.symbol())
    );
  }

  private isSymbol(symbol: string): boolean {
    return (
      this.tokenizer.tokenType() === "SYMBOL" &&
      this.tokenizer.symbol() === symbol
    );
  }

  private isIdentifier(): boolean {
    return this.tokenizer.tokenType() === "IDENTIFIER";
  }

  private isType(): boolean {
    return (
      this.isOneOfKeywords(["int", "boolean", "char", "void"]) ||
      this.isIdentifier()
    );
  }

  private expectKeyword(keyword: string): void {
    if (!this.isKeyword(keyword)) {
      throw new Error(`Expected keyword: ${keyword}`);
    }
    this.tokenizer.advance();
  }

  private expectSymbol(symbol: string): void {
    if (!this.isSymbol(symbol)) {
      throw new Error(`Expected symbol: ${symbol}`);
    }
    this.tokenizer.advance();
  }

  compileClass(): void {
    this.tokenizer.advance();

    // class keyword
    this.expectKeyword("class");

    // class name
    if (!this.isIdentifier()) {
      throw new Error("Expected class name");
    }
    this.compilingClassName = this.tokenizer.identifier();
    this.currentClassName = this.compilingClassName;
    this.tokenizer.advance();

    // opening brace
    this.expectSymbol("{");

    // class variable declarations
    while (this.isOneOfKeywords(["static", "field"])) {
      this.compileClassVarDec();
    }

    // subroutine declarations
    while (this.isOneOfKeywords(["constructor", "function", "method"])) {
      this.compileSubroutineDec();
    }

    // closing brace
    this.expectSymbol("}");

    // Write VM code to file
    fs.writeFileSync(
      this.outFilePath,
      this.vmWriter.getOutput().join("\n"),
      "utf8"
    );
  }

  compileClassVarDec(): void {
    const kind = this.tokenizer.keyword().toLowerCase();
    this.tokenizer.advance();

    // type
    if (!this.isType()) {
      throw new Error("Expected type");
    }
    const type = this.tokenizer.getValue();
    this.tokenizer.advance();

    // varName
    while (true) {
      if (!this.isIdentifier()) {
        throw new Error("Expected identifier");
      }
      const name = this.tokenizer.identifier();
      this.symbolTable.define(name, type, kind as "static" | "field");
      this.tokenizer.advance();

      if (this.isSymbol(";")) {
        this.tokenizer.advance();
        break;
      }

      this.expectSymbol(",");
    }
  }

  compileSubroutineDec(): void {
    // Reset subroutine symbol table
    this.symbolTable.startSubroutine();

    // constructor, function, or method
    this.currentSubroutineKind = this.tokenizer.keyword().toLowerCase();
    this.tokenizer.advance();

    // return type
    if (!this.isType()) {
      throw new Error("Expected return type");
    }
    this.tokenizer.advance();

    // subroutine name
    if (!this.isIdentifier()) {
      throw new Error("Expected subroutine name");
    }
    this.currentSubroutineName = this.tokenizer.identifier();
    this.tokenizer.advance();

    // parameter list
    this.expectSymbol("(");

    // For methods, add implicit this argument to symbol table
    if (this.currentSubroutineKind === "method") {
      this.symbolTable.define("this", this.currentClassName, "arg");
    }

    this.compileParameterList();
    this.expectSymbol(")");

    // subroutine body
    this.compileSubroutineBody();
  }

  compileParameterList(): void {
    if (this.isSymbol(")")) {
      return;
    }

    while (true) {
      // type
      if (!this.isType()) {
        throw new Error("Expected type");
      }
      const type = this.tokenizer.getValue();
      this.tokenizer.advance();

      // varName
      if (!this.isIdentifier()) {
        throw new Error("Expected parameter name");
      }
      const name = this.tokenizer.identifier();
      this.symbolTable.define(name, type, "arg");
      this.tokenizer.advance();

      if (this.isSymbol(")")) {
        break;
      }

      this.expectSymbol(",");
    }
  }

  compileSubroutineBody(): void {
    this.expectSymbol("{");

    // variable declarations
    while (this.isKeyword("var")) {
      this.compileVarDec();
    }

    // Write function declaration
    const nLocals = this.symbolTable.varCount("var");
    this.vmWriter.writeFunction(
      `${this.currentClassName}.${this.currentSubroutineName}`,
      nLocals
    );

    // Handle constructor
    if (this.currentSubroutineKind === "constructor") {
      const fieldCount = this.symbolTable.varCount("field");
      this.vmWriter.writePush("constant", fieldCount);
      this.vmWriter.writeCall("Memory.alloc", 1);
      this.vmWriter.writePop("pointer", 0);
    }
    // Handle method
    else if (this.currentSubroutineKind === "method") {
      this.vmWriter.writePush("argument", 0);
      this.vmWriter.writePop("pointer", 0);
    }

    // statements
    this.compileStatements();

    this.expectSymbol("}");
  }

  compileVarDec(): void {
    // var keyword
    this.tokenizer.advance();

    // type
    if (!this.isType()) {
      throw new Error("Expected type");
    }
    const type = this.tokenizer.getValue();
    this.tokenizer.advance();

    // varName
    while (true) {
      if (!this.isIdentifier()) {
        throw new Error("Expected identifier");
      }
      const name = this.tokenizer.identifier();
      this.symbolTable.define(name, type, "var");
      this.tokenizer.advance();

      if (this.isSymbol(";")) {
        this.tokenizer.advance();
        break;
      }

      this.expectSymbol(",");
    }
  }

  compileStatements(): void {
    while (this.isOneOfKeywords(["let", "if", "while", "do", "return"])) {
      const keyword = this.tokenizer.keyword().toLowerCase();
      switch (keyword) {
        case "let":
          this.compileLet();
          break;
        case "if":
          this.compileIf();
          break;
        case "while":
          this.compileWhile();
          break;
        case "do":
          this.compileDo();
          break;
        case "return":
          this.compileReturn();
          break;
      }
    }
  }

  compileLet(): void {
    // let keyword
    this.tokenizer.advance();

    // varName
    if (!this.isIdentifier()) {
      throw new Error("Expected variable name");
    }
    const varName = this.tokenizer.identifier();
    const varKind = this.symbolTable.kindOf(varName);
    const varIndex = this.symbolTable.indexOf(varName);
    if (!varKind || varIndex === null) {
      throw new Error(`Variable ${varName} not found in symbol table`);
    }
    this.tokenizer.advance();

    // array access
    if (this.isSymbol("[")) {
      this.tokenizer.advance();
      this.vmWriter.writePush(this.getSegment(varKind), varIndex);
      this.compileExpression();
      this.vmWriter.writeArithmetic("add");
      this.expectSymbol("]");

      // equals sign
      this.expectSymbol("=");

      // expression
      this.compileExpression();

      // Store result in temp
      this.vmWriter.writePop("temp", 0);
      // Store array address in that
      this.vmWriter.writePop("pointer", 1);
      // Push result
      this.vmWriter.writePush("temp", 0);
      // Store in array
      this.vmWriter.writePop("that", 0);
    } else {
      // equals sign
      this.expectSymbol("=");

      // expression
      this.compileExpression();

      // Store result
      this.vmWriter.writePop(this.getSegment(varKind), varIndex);
    }

    // semicolon
    this.expectSymbol(";");
  }

  compileIf(): void {
    // if keyword
    this.tokenizer.advance();

    const endLabel = this.getNextLabel();
    const elseLabel = this.getNextLabel();

    // condition
    this.expectSymbol("(");
    this.compileExpression();
    this.expectSymbol(")");

    // Negate condition
    this.vmWriter.writeArithmetic("not");
    this.vmWriter.writeIf(elseLabel);

    // then statements
    this.expectSymbol("{");
    this.compileStatements();
    this.expectSymbol("}");

    this.vmWriter.writeGoto(endLabel);
    this.vmWriter.writeLabel(elseLabel);

    // else clause
    if (this.isKeyword("else")) {
      this.tokenizer.advance();
      this.expectSymbol("{");
      this.compileStatements();
      this.expectSymbol("}");
    }

    this.vmWriter.writeLabel(endLabel);
  }

  compileWhile(): void {
    // while keyword
    this.tokenizer.advance();

    const startLabel = this.getNextLabel();
    const endLabel = this.getNextLabel();

    this.vmWriter.writeLabel(startLabel);

    // condition
    this.expectSymbol("(");
    this.compileExpression();
    this.expectSymbol(")");

    // Negate condition
    this.vmWriter.writeArithmetic("not");
    this.vmWriter.writeIf(endLabel);

    // statements
    this.expectSymbol("{");
    this.compileStatements();
    this.expectSymbol("}");

    this.vmWriter.writeGoto(startLabel);
    this.vmWriter.writeLabel(endLabel);
  }

  compileDo(): void {
    // do keyword
    this.tokenizer.advance();

    // subroutine call
    this.compileSubroutineCall();

    // Pop return value (void)
    this.vmWriter.writePop("temp", 0);

    // semicolon
    this.expectSymbol(";");
  }

  compileReturn(): void {
    // return keyword
    this.tokenizer.advance();

    // expression (optional)
    if (!this.isSymbol(";")) {
      this.compileExpression();
    } else {
      // Push 0 for void return
      this.vmWriter.writePush("constant", 0);
    }

    this.vmWriter.writeReturn();

    // semicolon
    this.expectSymbol(";");
  }

  compileExpression(): void {
    this.compileTerm();

    while (this.isOneOfSymbols(["+", "-", "*", "/", "&", "|", "<", ">", "="])) {
      const op = this.tokenizer.symbol();
      this.tokenizer.advance();
      this.compileTerm();

      switch (op) {
        case "+":
          this.vmWriter.writeArithmetic("add");
          break;
        case "-":
          this.vmWriter.writeArithmetic("sub");
          break;
        case "*":
          this.vmWriter.writeCall("Math.multiply", 2);
          break;
        case "/":
          this.vmWriter.writeCall("Math.divide", 2);
          break;
        case "&":
          this.vmWriter.writeArithmetic("and");
          break;
        case "|":
          this.vmWriter.writeArithmetic("or");
          break;
        case "<":
          this.vmWriter.writeArithmetic("lt");
          break;
        case ">":
          this.vmWriter.writeArithmetic("gt");
          break;
        case "=":
          this.vmWriter.writeArithmetic("eq");
          break;
      }
    }
  }

  compileTerm(): void {
    const type = this.tokenizer.tokenType();

    const compileConstant = () => {
      this.vmWriter.writePush("constant", this.tokenizer.intVal());
      this.tokenizer.advance();
    };

    const compileString = () => {
      const str = this.tokenizer.stringVal();
      this.vmWriter.writePush("constant", str.length);
      this.vmWriter.writeCall("String.new", 1);
      for (let i = 0; i < str.length; i++) {
        this.vmWriter.writePush("constant", str.charCodeAt(i));
        this.vmWriter.writeCall("String.appendChar", 2);
      }
      this.tokenizer.advance();
    };

    const compileKeyword = () => {
      const keyword = this.tokenizer.keyword().toLowerCase();
      switch (keyword) {
        case "true":
          this.vmWriter.writePush("constant", 1);
          this.vmWriter.writeArithmetic("neg");
          break;
        case "false":
        case "null":
          this.vmWriter.writePush("constant", 0);
          break;
        case "this":
          this.vmWriter.writePush("pointer", 0);
          break;
      }
      this.tokenizer.advance();
    };

    const compileVariable = () => {
      const name = this.tokenizer.identifier();
      this.tokenizer.advance();

      if (this.isSymbol("[")) {
        // array access
        const kind = this.symbolTable.kindOf(name);
        const index = this.symbolTable.indexOf(name);
        if (!kind || index === null) {
          throw new Error(`Variable ${name} not found in symbol table`);
        }
        this.vmWriter.writePush(this.getSegment(kind), index);
        this.tokenizer.advance();
        this.compileExpression();
        this.expectSymbol("]");
        this.vmWriter.writeArithmetic("add");
        this.vmWriter.writePop("pointer", 1);
        this.vmWriter.writePush("that", 0);
      } else if (this.isSymbol("(") || this.isSymbol(".")) {
        // subroutine call
        this.compileSubroutineCall(name);
      } else {
        // variable
        const kind = this.symbolTable.kindOf(name);
        const index = this.symbolTable.indexOf(name);
        if (!kind || index === null) {
          throw new Error(`Variable ${name} not found in symbol table`);
        }
        this.vmWriter.writePush(this.getSegment(kind), index);
      }
    };

    const compileExpressionInParentheses = () => {
      this.tokenizer.advance();
      this.compileExpression();
      this.expectSymbol(")");
    };

    const compileUnary = () => {
      const op = this.tokenizer.symbol();
      this.tokenizer.advance();
      this.compileTerm();
      if (op === "-") {
        this.vmWriter.writeArithmetic("neg");
      } else {
        this.vmWriter.writeArithmetic("not");
      }
    };

    if (type === "INT_CONST") {
      compileConstant();
    } else if (type === "STRING_CONST") {
      compileString();
    } else if (this.isOneOfKeywords(["true", "false", "null", "this"])) {
      compileKeyword();
    } else if (this.isIdentifier()) {
      compileVariable();
    } else if (this.isSymbol("(")) {
      compileExpressionInParentheses();
    } else if (this.isOneOfSymbols(["-", "~"])) {
      compileUnary();
    } else {
      throw new Error("Invalid term");
    }
  }

  compileSubroutineCall(name?: string): void {
    let className = this.currentClassName;
    const getIdentifier = () => {
      if (name) return name;
      const identifier = this.tokenizer.identifier();
      this.tokenizer.advance();
      return identifier;
    };
    let subroutineName = getIdentifier();
    let nArgs = 0;

    const compileObjectMethodOrStaticFunctionCall = () => {
      const kind = this.symbolTable.kindOf(subroutineName);
      if (kind) {
        // Method call on object
        const type = this.symbolTable.typeOf(subroutineName);
        if (!type) {
          throw new Error(`Type not found for variable ${subroutineName}`);
        }
        className = type;
        const index = this.symbolTable.indexOf(subroutineName);
        if (index === null) {
          throw new Error(`Index not found for variable ${subroutineName}`);
        }
        this.vmWriter.writePush(this.getSegment(kind), index);
        nArgs = 1;
      } else {
        // Function call on class
        className = subroutineName;
      }
      this.tokenizer.advance();
      subroutineName = this.tokenizer.identifier();
      this.tokenizer.advance();
    };

    if (this.isSymbol(".")) {
      compileObjectMethodOrStaticFunctionCall();
    } else {
      // Method call on this
      this.vmWriter.writePush("pointer", 0);
      nArgs = 1;
    }

    this.expectSymbol("(");
    nArgs += this.compileExpressionList();
    this.expectSymbol(")");

    this.vmWriter.writeCall(`${className}.${subroutineName}`, nArgs);
  }

  compileExpressionList(): number {
    if (this.isSymbol(")")) {
      return 0;
    }

    let nArgs = 0;
    while (true) {
      this.compileExpression();
      nArgs++;

      if (this.isSymbol(")")) {
        break;
      }

      this.expectSymbol(",");
    }

    return nArgs;
  }

  private getSegment(kind: string): "argument" | "local" | "static" | "this" {
    switch (kind) {
      case "arg":
        return "argument";
      case "var":
        return "local";
      case "static":
        return "static";
      case "field":
        return "this";
      default:
        throw new Error(`Invalid kind: ${kind}`);
    }
  }
}

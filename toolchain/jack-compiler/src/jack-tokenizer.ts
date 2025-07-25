type Keyword =
  | "CLASS"
  | "METHOD"
  | "FUNCTION"
  | "CONSTRUCTOR"
  | "INT"
  | "BOOLEAN"
  | "CHAR"
  | "VOID"
  | "VAR"
  | "STATIC"
  | "FIELD"
  | "LET"
  | "DO"
  | "IF"
  | "ELSE"
  | "WHILE"
  | "RETURN"
  | "TRUE"
  | "FALSE"
  | "NULL"
  | "THIS";

export class JackTokenizer {
  private source: string;
  private currentPosition: number;
  private currentToken: {
    type: "KEYWORD" | "SYMBOL" | "IDENTIFIER" | "INT_CONST" | "STRING_CONST";
    value: string;
  } | null;

  private static readonly KEYWORDS: Set<string> = new Set([
    "class",
    "method",
    "function",
    "constructor",
    "int",
    "boolean",
    "char",
    "void",
    "var",
    "static",
    "field",
    "let",
    "do",
    "if",
    "else",
    "while",
    "return",
    "true",
    "false",
    "null",
    "this",
  ]);

  private static readonly SYMBOLS: Set<string> = new Set([
    "{",
    "}",
    "(",
    ")",
    "[",
    "]",
    ".",
    ",",
    ";",
    "+",
    "-",
    "*",
    "/",
    "&",
    "|",
    "<",
    ">",
    "=",
    "~",
  ]);

  constructor(source: string) {
    this.source = source;
    this.currentPosition = 0;
    this.currentToken = null;
  }

  private skipWhitespace(): void {
    while (this.currentPosition < this.source.length) {
      const char = this.source[this.currentPosition];
      if (char === " " || char === "\t" || char === "\n" || char === "\r") {
        this.currentPosition++;
      } else if (
        char === "/" &&
        this.source[this.currentPosition + 1] === "/"
      ) {
        // Skip single-line comments
        while (
          this.currentPosition < this.source.length &&
          this.source[this.currentPosition] !== "\n"
        ) {
          this.currentPosition++;
        }
      } else if (
        char === "/" &&
        this.source[this.currentPosition + 1] === "*"
      ) {
        // Skip multi-line comments
        this.currentPosition += 2;
        while (this.currentPosition < this.source.length - 1) {
          if (
            this.source[this.currentPosition] === "*" &&
            this.source[this.currentPosition + 1] === "/"
          ) {
            this.currentPosition += 2;
            break;
          }
          this.currentPosition++;
        }
      } else {
        break;
      }
    }
  }

  hasMoreTokens(): boolean {
    this.skipWhitespace();
    return this.currentPosition < this.source.length;
  }

  advance(): void {
    this.skipWhitespace();
    if (!this.hasMoreTokens()) {
      return;
    }

    const char = this.source[this.currentPosition];

    // Handle string constants
    if (char === '"') {
      this.currentPosition++;
      let value = "";
      while (
        this.currentPosition < this.source.length &&
        this.source[this.currentPosition] !== '"'
      ) {
        value += this.source[this.currentPosition];
        this.currentPosition++;
      }
      this.currentPosition++; // Skip closing quote
      this.currentToken = { type: "STRING_CONST", value };
      return;
    }

    // Handle symbols
    if (JackTokenizer.SYMBOLS.has(char)) {
      this.currentToken = { type: "SYMBOL", value: char };
      this.currentPosition++;
      return;
    }

    // Handle identifiers and keywords
    if (/^[a-zA-Z_]/.test(char)) {
      let value = "";
      while (
        this.currentPosition < this.source.length &&
        /^[a-zA-Z0-9_]/.test(this.source[this.currentPosition])
      ) {
        value += this.source[this.currentPosition];
        this.currentPosition++;
      }
      const upperValue = value.toUpperCase();
      if (JackTokenizer.KEYWORDS.has(value.toLowerCase())) {
        this.currentToken = { type: "KEYWORD", value: upperValue };
      } else {
        this.currentToken = { type: "IDENTIFIER", value };
      }
      return;
    }

    // Handle integer constants
    if (/^[0-9]/.test(char)) {
      let value = "";
      while (
        this.currentPosition < this.source.length &&
        /^[0-9]/.test(this.source[this.currentPosition])
      ) {
        value += this.source[this.currentPosition];
        this.currentPosition++;
      }
      this.currentToken = { type: "INT_CONST", value };
      return;
    }

    throw new Error(`Invalid character: ${char}`);
  }

  tokenType():
    | "KEYWORD"
    | "SYMBOL"
    | "IDENTIFIER"
    | "INT_CONST"
    | "STRING_CONST" {
    if (!this.currentToken) {
      throw new Error("No current token");
    }
    return this.currentToken.type;
  }

  getValue(): string {
    if (!this.currentToken) {
      throw new Error("No current token");
    }
    return this.currentToken.value;
  }

  keyword(): Keyword {
    if (!this.currentToken || this.currentToken.type !== "KEYWORD") {
      throw new Error("Current token is not a keyword");
    }
    return this.currentToken.value as Keyword;
  }

  symbol(): string {
    if (!this.currentToken || this.currentToken.type !== "SYMBOL") {
      throw new Error("Current token is not a symbol");
    }
    return this.currentToken.value;
  }

  identifier(): string {
    if (!this.currentToken || this.currentToken.type !== "IDENTIFIER") {
      throw new Error("Current token is not an identifier");
    }
    return this.currentToken.value;
  }

  intVal(): number {
    if (!this.currentToken || this.currentToken.type !== "INT_CONST") {
      throw new Error("Current token is not an integer constant");
    }
    return parseInt(this.currentToken.value);
  }

  stringVal(): string {
    if (!this.currentToken || this.currentToken.type !== "STRING_CONST") {
      throw new Error("Current token is not a string constant");
    }
    return this.currentToken.value;
  }
}

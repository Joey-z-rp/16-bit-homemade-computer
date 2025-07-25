type Segment =
  | "argument"
  | "local"
  | "static"
  | "constant"
  | "this"
  | "that"
  | "pointer"
  | "temp";

type Command =
  | "add"
  | "sub"
  | "neg"
  | "eq"
  | "gt"
  | "lt"
  | "and"
  | "or"
  | "not";

export class VMWriter {
  private output: string[];

  constructor() {
    this.output = [];
  }

  public writePush(segment: Segment, index: number): void {
    this.output.push(`push ${segment} ${index}`);
  }

  public writePop(segment: Segment, index: number): void {
    this.output.push(`pop ${segment} ${index}`);
  }

  public writeArithmetic(command: Command): void {
    this.output.push(command);
  }

  public writeLabel(label: string): void {
    this.output.push(`label ${label}`);
  }

  public writeGoto(label: string): void {
    this.output.push(`goto ${label}`);
  }

  public writeIf(label: string): void {
    this.output.push(`if-goto ${label}`);
  }

  public writeCall(name: string, nArgs: number): void {
    this.output.push(`call ${name} ${nArgs}`);
  }

  public writeFunction(name: string, nLocals: number): void {
    this.output.push(`function ${name} ${nLocals}`);
  }

  public writeReturn(): void {
    this.output.push("return");
  }

  public getOutput(): string[] {
    return this.output;
  }
}

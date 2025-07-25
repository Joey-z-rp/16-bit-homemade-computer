type Kind = "static" | "field" | "arg" | "var";

export class SymbolTable {
  private classTable: Map<string, { type: string; kind: Kind; index: number }>;
  private subroutineTable: Map<
    string,
    { type: string; kind: Kind; index: number }
  >;

  constructor() {
    this.classTable = new Map();
    this.subroutineTable = new Map();
  }

  public startSubroutine(): void {
    this.subroutineTable.clear();
  }

  public define(name: string, type: string, kind: Kind): void {
    if (kind === "static" || kind === "field") {
      this.classTable.set(name, { type, kind, index: this.varCount(kind) });
    } else {
      this.subroutineTable.set(name, {
        type,
        kind,
        index: this.varCount(kind),
      });
    }
  }

  public varCount(kind: Kind): number {
    const table =
      kind === "static" || kind === "field"
        ? this.classTable
        : this.subroutineTable;
    return Array.from(table.values()).filter((item) => item.kind === kind)
      .length;
  }

  public kindOf(name: string): Kind | null {
    if (this.subroutineTable.has(name)) {
      return this.subroutineTable.get(name)?.kind ?? null;
    }
    return this.classTable.get(name)?.kind ?? null;
  }

  public typeOf(name: string): string | null {
    if (this.subroutineTable.has(name)) {
      return this.subroutineTable.get(name)?.type ?? null;
    }
    return this.classTable.get(name)?.type ?? null;
  }

  public indexOf(name: string): number | null {
    if (this.subroutineTable.has(name)) {
      return this.subroutineTable.get(name)?.index ?? null;
    }
    return this.classTable.get(name)?.index ?? null;
  }
}

import * as fs from "fs";
import * as path from "path";
import { JackCompilationEngine } from "./jack-compilation-engine";

export class JackCompiler {
  static compile(inputPath: string, outputDir?: string): void {
    const stat = fs.statSync(inputPath);
    const jackFiles: string[] = stat.isDirectory()
      ? fs
          .readdirSync(inputPath)
          .filter((f) => f.endsWith(".jack"))
          .map((f) => path.join(inputPath, f))
      : [inputPath];

    jackFiles.forEach((filePath) => {
      const source = fs.readFileSync(filePath, "utf8");
      const filename = path.basename(filePath, ".jack");

      let outPath: string;
      if (outputDir) {
        // Ensure output directory exists
        if (!fs.existsSync(outputDir)) {
          fs.mkdirSync(outputDir, { recursive: true });
        }
        outPath = path.join(outputDir, `${filename}.vm`);
      } else {
        // Default behavior: output in same directory as input
        outPath = filePath.replace(/\.jack$/, ".vm");
      }

      const engine = new JackCompilationEngine(source, outPath);
      engine.compileClass();
      console.log(`Compiled: ${filePath} → ${outPath}`);
    });
  }
}

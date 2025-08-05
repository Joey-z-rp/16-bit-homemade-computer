#!/usr/bin/env ts-node

import { JackCompiler } from "./src/jack-compiler";
import * as path from "path";

const args = process.argv.slice(2);
if (args.length < 1 || args.length > 2) {
  console.error(
    "Usage: jack-analyzer <file.jack | directory> [output-directory]"
  );
  process.exit(1);
}

const inputPath = path.resolve(args[0]);
const outputDir = args[1] ? path.resolve(args[1]) : undefined;

JackCompiler.compile(inputPath, outputDir);

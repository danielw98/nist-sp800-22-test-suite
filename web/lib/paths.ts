import fs from "node:fs";
import path from "node:path";

import { NIST_FILES } from "./spectral";

const isWindows = process.platform === "win32";
const exe = (name: string) => (isWindows ? `${name}.exe` : name);

/**
 * Directory holding the compiled C++ binaries.
 * Override with DFT_BIN_DIR (set to /app/bin in the Docker image); the dev
 * default is the CMake build dir one level up from the web/ project.
 */
export function binDir(): string {
  return process.env.DFT_BIN_DIR ?? path.resolve(process.cwd(), "..", "build");
}

/** Directory holding the NIST data files (override with DFT_DATA_DIR). */
export function dataDir(): string {
  return process.env.DFT_DATA_DIR ?? path.resolve(process.cwd(), "..", "data");
}

/** The generic single-run binary: `nist_test --test <id>`. */
export function nistTestPath(): string {
  return path.join(binDir(), exe("nist_test"));
}

/** The generic multi-stream assessment binary: `nist_assess --test <id>`. */
export function nistAssessPath(): string {
  return path.join(binDir(), exe("nist_assess"));
}

/** True if both binaries are present (used by the health check). */
export function binariesExist(): boolean {
  return fs.existsSync(nistTestPath()) && fs.existsSync(nistAssessPath());
}

/** Resolve an allowlisted NIST data file to an absolute path, or null if unknown. */
export function dataFilePath(name: string): string | null {
  if (!(NIST_FILES as readonly string[]).includes(name)) {
    return null;
  }
  return path.join(dataDir(), name);
}

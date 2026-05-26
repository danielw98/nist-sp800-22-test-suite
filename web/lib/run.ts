import { spawn } from "node:child_process";

export interface SpawnResult {
  stdout: string;
  stderr: string;
  code: number | null;
}

/**
 * Run a binary with the given args, optionally piping `stdin`, killing it after
 * `timeoutMs`. Resolves with captured stdout/stderr/exit-code; rejects on spawn
 * error or timeout.
 */
export function runBinary(
  bin: string,
  args: string[],
  stdin: string | null,
  timeoutMs = 30000,
): Promise<SpawnResult> {
  return new Promise((resolve, reject) => {
    const child = spawn(bin, args, { windowsHide: true });
    let stdout = "";
    let stderr = "";
    const timer = setTimeout(() => {
      child.kill("SIGKILL");
      reject(new Error("timeout"));
    }, timeoutMs);

    child.stdout.on("data", (d) => {
      stdout += d.toString();
    });
    child.stderr.on("data", (d) => {
      stderr += d.toString();
    });
    child.on("error", (err) => {
      clearTimeout(timer);
      reject(err);
    });
    child.on("close", (code) => {
      clearTimeout(timer);
      resolve({ stdout, stderr, code });
    });

    if (stdin !== null) {
      child.stdin.write(stdin);
      child.stdin.end();
    }
  });
}

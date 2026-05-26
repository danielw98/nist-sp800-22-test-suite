import { fileURLToPath } from "node:url";

import { defineConfig } from "vitest/config";

// Resolve "@/..." the same way Next/tsconfig does, so tests can import route
// handlers and lib modules by their app-style paths.
const root = fileURLToPath(new URL(".", import.meta.url)).replace(/[\\/]$/, "");

export default defineConfig({
  resolve: {
    alias: { "@": root },
  },
  test: {
    environment: "node",
  },
});

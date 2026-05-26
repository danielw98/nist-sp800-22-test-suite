import { describe, expect, it } from "vitest";

import { assessSchema, runBodySchema, sanitizeBits } from "./validation";

describe("validation", () => {
  it("keeps only 0/1 characters", () => {
    expect(sanitizeBits("1 0\n1abc0\t1")).toBe("10101");
  });

  it("applies run defaults", () => {
    const r = runBodySchema.parse({ bits: "0101" });
    expect(r.method).toBe("auto");
    expect(r.alpha).toBe(0.01);
  });

  it("rejects an unknown method", () => {
    expect(runBodySchema.safeParse({ bits: "01", method: "nope" }).success).toBe(false);
  });

  it("rejects alpha out of range", () => {
    expect(runBodySchema.safeParse({ bits: "01", alpha: 0.9 }).success).toBe(false);
  });

  it("assess requires at least one file", () => {
    expect(assessSchema.safeParse({ files: [] }).success).toBe(false);
  });

  it("assess applies length default", () => {
    const r = assessSchema.parse({ files: ["data.pi"] });
    expect(r.length).toBe(10000);
  });
});

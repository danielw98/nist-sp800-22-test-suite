// Route-level API tests. These call the real route handlers, which spawn the
// compiled C++ binary (../build), so they double as an end-to-end check that the
// API returns what the suite computes. Requests carry no X-Forwarded-For, so the
// client IP is "unknown" (allowlisted) and the rate limiter never trips.
import { describe, expect, it } from "vitest";

import { GET as healthGet } from "@/app/api/health/route";
import { POST as runPost } from "@/app/api/run/[test]/route";
import { NIST_EXAMPLE_100 } from "@/lib/spectral";

type RunCtx = { params: Promise<{ test: string }> };
const asReq = (r: Request) => r as unknown as Parameters<typeof runPost>[0];

function postRun(testId: string, body: unknown, raw?: string) {
  const req = asReq(
    new Request(`http://localhost/api/run/${testId}`, {
      method: "POST",
      headers: { "content-type": "application/json" },
      body: raw ?? JSON.stringify(body),
    }),
  );
  const ctx: RunCtx = { params: Promise.resolve({ test: testId }) };
  return runPost(req, ctx);
}

describe("POST /api/run/{test} (spawns the real binary)", () => {
  it("dft reproduces the NIST example: stats.N1=48, p=0.646355", async () => {
    const res = await postRun("dft", { bits: NIST_EXAMPLE_100 });
    expect(res.status).toBe(200);
    const json = await res.json();
    expect(json.test).toBe("dft");
    expect(json.stats.N1).toBe(48);
    expect(json.p_value).toBeCloseTo(0.646355, 5);
    expect(json.passed).toBe(true);
    expect(Array.isArray(json.spectrum)).toBe(true);
  });

  it("monobit reproduces the NIST example p-value 0.109599", async () => {
    const res = await postRun("monobit", { bits: NIST_EXAMPLE_100 });
    expect(res.status).toBe(200);
    const json = await res.json();
    expect(json.test).toBe("monobit");
    expect(json.p_value).toBeCloseTo(0.109599, 5);
  });

  it("returns the critical-value verdict fields (statistic, critical, distribution)", async () => {
    // All 15 tests are now implemented, so 501 is no longer reachable via a real
    // slug; instead verify the critical-value envelope the verdict is framed on.
    const res = await postRun("dft", { bits: NIST_EXAMPLE_100 });
    expect(res.status).toBe(200);
    const json = await res.json();
    expect(json.statistic).toBeCloseTo(0.458831, 4);
    expect(typeof json.critical).toBe("number");
    expect(json.distribution).toBe("normal (two-sided)");
  });

  it("an unknown test returns 404", async () => {
    const res = await postRun("no-such-test", { bits: NIST_EXAMPLE_100 });
    expect(res.status).toBe(404);
    expect((await res.json()).error).toBe("unknown_test");
  });

  it("rejects fewer than 2 bits with 400 no_bits", async () => {
    const res = await postRun("monobit", { bits: "1" });
    expect(res.status).toBe(400);
    expect((await res.json()).error).toBe("no_bits");
  });

  it("rejects malformed JSON with 400", async () => {
    const res = await postRun("monobit", undefined, "{ not json");
    expect(res.status).toBe(400);
  });
});

describe("GET /api/health", () => {
  it("reports ok, binaries present, and the test catalog", async () => {
    const res = await healthGet();
    expect(res.status).toBe(200);
    const json = await res.json();
    expect(json.status).toBe("ok");
    expect(json.binaries).toBe(true);
    expect(Array.isArray(json.tests)).toBe(true);
    expect(json.tests.length).toBe(15);
  });
});

import { describe, expect, it } from "vitest";

import {
  __resetRateLimit,
  checkRateLimit,
  isAllowlisted,
  rateLimitHeaders,
  recordTraffic,
} from "./ratelimit";

const T = 1_700_000_000_000;

describe("rate limiter", () => {
  it("never limits allowlisted callers (localhost / unknown)", () => {
    __resetRateLimit({ perMin: 1, perIpReq: 1, perIpBytes: 1, globalBytes: 1 });
    for (let i = 0; i < 50; i++) {
      expect(checkRateLimit("unknown", T).ok).toBe(true);
      expect(checkRateLimit("127.0.0.1", T).ok).toBe(true);
    }
    expect(isAllowlisted("unknown")).toBe(true);
    expect(isAllowlisted("8.8.8.8")).toBe(false);
  });

  it("allows a burst up to the per-minute cap, then 429", () => {
    __resetRateLimit({ perMin: 3, perIpReq: 1000, perIpBytes: 1e12, globalBytes: 1e15 });
    for (let i = 0; i < 3; i++) {
      expect(checkRateLimit("9.9.9.9", T).ok).toBe(true);
    }
    const d = checkRateLimit("9.9.9.9", T);
    expect(d.ok).toBe(false);
    expect(d.reason).toBe("per_minute");
    expect(rateLimitHeaders(d)["Retry-After"]).toBeDefined();
  });

  it("resets the per-minute window after a minute", () => {
    __resetRateLimit({ perMin: 3, perIpReq: 1000, perIpBytes: 1e12, globalBytes: 1e15 });
    for (let i = 0; i < 3; i++) checkRateLimit("8.8.8.8", T);
    expect(checkRateLimit("8.8.8.8", T).ok).toBe(false);
    expect(checkRateLimit("8.8.8.8", T + 60_000).ok).toBe(true);
  });

  it("enforces the per-IP request backstop", () => {
    __resetRateLimit({ perMin: 100, perIpReq: 5, perIpBytes: 1e12, globalBytes: 1e15 });
    for (let i = 0; i < 5; i++) {
      expect(checkRateLimit("4.4.4.4", T).ok).toBe(true);
    }
    const d = checkRateLimit("4.4.4.4", T);
    expect(d.ok).toBe(false);
    expect(d.reason).toBe("per_ip_requests");
  });

  it("enforces the per-IP traffic budget once it is spent", () => {
    __resetRateLimit({ perMin: 100, perIpReq: 1000, perIpBytes: 1000, globalBytes: 1e15 });
    recordTraffic("3.3.3.3", 1000, T);
    const d = checkRateLimit("3.3.3.3", T);
    expect(d.ok).toBe(false);
    expect(d.reason).toBe("per_ip_bytes");
    expect(d.bytesRemaining).toBe(0);
  });

  it("enforces the global traffic ceiling", () => {
    __resetRateLimit({ perMin: 100, perIpReq: 1000, perIpBytes: 1e9, globalBytes: 1000 });
    recordTraffic("a.a.a.a", 1000, T);
    const d = checkRateLimit("b.b.b.b", T);
    expect(d.ok).toBe(false);
    expect(d.reason).toBe("global_bytes");
  });

  it("does not charge traffic to allowlisted callers", () => {
    __resetRateLimit({ perMin: 100, perIpReq: 1000, perIpBytes: 1e9, globalBytes: 1000 });
    recordTraffic("127.0.0.1", 1e12, T);
    expect(checkRateLimit("c.c.c.c", T).ok).toBe(true);
  });

  it("sets the X-RateLimit headers including the byte budget", () => {
    __resetRateLimit({ perMin: 100, perIpReq: 5, perIpBytes: 4096, globalBytes: 1e15 });
    const d = checkRateLimit("5.5.5.5", T);
    const h = rateLimitHeaders(d);
    expect(h["X-RateLimit-Limit"]).toBe("5");
    expect(Number(h["X-RateLimit-Remaining"])).toBe(4);
    expect(h["X-RateLimit-Bytes-Limit"]).toBe("4096");
    expect(Number(h["X-RateLimit-Bytes-Remaining"])).toBe(4096);
  });
});

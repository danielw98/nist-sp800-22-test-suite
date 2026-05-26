/**
 * In-memory rate limiter for the public API. Suitable for a single container
 * (counters reset on restart) - fine for safety-net ceilings. Generous,
 * abuse-oriented defaults on a rolling 30-day window, all env-configurable:
 *   - 120 requests / minute / IP        (RL_PER_MIN)       -> bursts are fine
 *   - 100000 requests / 30d / IP        (RL_PER_IP_REQ)    -> floor vs tiny-request floods
 *   - 5 GiB traffic / 30d / IP          (RL_PER_IP_BYTES)  -> hard per-user cap
 *   - 1 TiB traffic / 30d global        (RL_GLOBAL_BYTES)  -> global ceiling
 * Traffic = request bytes + response bytes, charged via recordTraffic() after
 * the response (charging the next request is fine for a ceiling). Allowlisted
 * IPs (the owner, localhost) bypass everything.
 */
const MONTH_MS = 30 * 24 * 60 * 60 * 1000;
const MINUTE_MS = 60 * 1000;
const GIB = 1024 ** 3;
const TIB = 1024 ** 4;

let perMin = Number(process.env.RL_PER_MIN ?? 120);
let perIpReq = Number(process.env.RL_PER_IP_REQ ?? 100_000);
let perIpBytes = Number(process.env.RL_PER_IP_BYTES ?? 5 * GIB);
let globalBytes = Number(process.env.RL_GLOBAL_BYTES ?? TIB);

// IPs that bypass all limits. Local/direct requests (no X-Forwarded-For ->
// "unknown") and loopback are always exempt; behind nginx the real client IP is
// always set, so "unknown" never appears in production. Add your own public IP
// via RL_ALLOWLIST (comma-separated) so the owner is never throttled.
const ALLOWLIST = new Set<string>([
  "unknown",
  "127.0.0.1",
  "::1",
  "localhost",
  ...(process.env.RL_ALLOWLIST ?? "")
    .split(",")
    .map((s) => s.trim())
    .filter(Boolean),
]);

/** True if `ip` bypasses the rate limiter. */
export function isAllowlisted(ip: string): boolean {
  return ALLOWLIST.has(ip);
}

interface IpState {
  minuteStart: number;
  minuteCount: number;
  monthStart: number;
  monthCount: number; // requests in the 30-day window
  monthBytes: number; // traffic in the 30-day window
}

const ipMap = new Map<string, IpState>();
let globalMonthStart = Date.now();
let globalBytesUsed = 0;

export interface RateDecision {
  ok: boolean;
  status: number; // 200 if allowed, 429 if limited
  reason?: "per_minute" | "per_ip_requests" | "per_ip_bytes" | "global_bytes";
  retryAfter: number; // seconds
  limit: number; // per-IP request backstop (for headers)
  remaining: number; // per-IP requests remaining
  reset: number; // epoch seconds when the per-IP 30-day window resets
  bytesLimit: number; // per-IP traffic budget
  bytesRemaining: number; // per-IP traffic remaining
}

/** Best-effort client IP, trusting nginx's X-Forwarded-For (first hop). */
export function getClientIp(req: Request): string {
  const xff = req.headers.get("x-forwarded-for");
  if (xff) {
    return xff.split(",")[0].trim();
  }
  return req.headers.get("x-real-ip")?.trim() ?? "unknown";
}

/** Fetch the IP's state, rolling the minute/month windows if they have elapsed. */
function freshState(ip: string, now: number): IpState {
  let s = ipMap.get(ip);
  if (!s || now - s.monthStart >= MONTH_MS) {
    s = { minuteStart: now, minuteCount: 0, monthStart: now, monthCount: 0, monthBytes: 0 };
    ipMap.set(ip, s);
  }
  if (now - s.minuteStart >= MINUTE_MS) {
    s.minuteStart = now;
    s.minuteCount = 0;
  }
  return s;
}

/** Decide whether a request from `ip` is allowed, updating request counters if so.
 *  Byte budgets are checked here but charged later via recordTraffic(). */
export function checkRateLimit(ip: string, now: number = Date.now()): RateDecision {
  // Allowlisted callers (owner, localhost) are never limited.
  if (ALLOWLIST.has(ip)) {
    return {
      ok: true, status: 200, retryAfter: 0,
      limit: perIpReq, remaining: perIpReq,
      reset: Math.ceil((now + MONTH_MS) / 1000),
      bytesLimit: perIpBytes, bytesRemaining: perIpBytes,
    };
  }

  if (now - globalMonthStart >= MONTH_MS) {
    globalMonthStart = now;
    globalBytesUsed = 0;
  }
  const s = freshState(ip, now);

  const reset = Math.ceil((s.monthStart + MONTH_MS) / 1000);
  const remaining = Math.max(0, perIpReq - s.monthCount);
  const bytesRemaining = Math.max(0, perIpBytes - s.monthBytes);
  const base = { limit: perIpReq, remaining, reset, bytesLimit: perIpBytes, bytesRemaining };

  if (globalBytesUsed >= globalBytes) {
    return {
      ok: false, status: 429, reason: "global_bytes",
      retryAfter: Math.ceil((globalMonthStart + MONTH_MS - now) / 1000), ...base,
    };
  }
  if (s.monthBytes >= perIpBytes) {
    return {
      ok: false, status: 429, reason: "per_ip_bytes",
      retryAfter: Math.ceil((s.monthStart + MONTH_MS - now) / 1000), ...base,
    };
  }
  if (s.minuteCount >= perMin) {
    return {
      ok: false, status: 429, reason: "per_minute",
      retryAfter: Math.max(1, Math.ceil((s.minuteStart + MINUTE_MS - now) / 1000)), ...base,
    };
  }
  if (s.monthCount >= perIpReq) {
    return {
      ok: false, status: 429, reason: "per_ip_requests",
      retryAfter: Math.ceil((s.monthStart + MONTH_MS - now) / 1000), ...base,
    };
  }

  s.minuteCount += 1;
  s.monthCount += 1;
  return {
    ok: true, status: 200, retryAfter: 0, ...base,
    remaining: Math.max(0, perIpReq - s.monthCount),
  };
}

/** Charge `bytes` (request + response) against the IP's and the global traffic
 *  budgets, after the response is known. Allowlisted callers are not charged. */
export function recordTraffic(ip: string, bytes: number, now: number = Date.now()): void {
  if (ALLOWLIST.has(ip) || bytes <= 0) {
    return;
  }
  if (now - globalMonthStart >= MONTH_MS) {
    globalMonthStart = now;
    globalBytesUsed = 0;
  }
  const s = freshState(ip, now);
  s.monthBytes += bytes;
  globalBytesUsed += bytes;
}

/** Build the X-RateLimit-* (and Retry-After when limited) response headers. */
export function rateLimitHeaders(d: RateDecision): Record<string, string> {
  const h: Record<string, string> = {
    "X-RateLimit-Limit": String(d.limit),
    "X-RateLimit-Remaining": String(d.remaining),
    "X-RateLimit-Reset": String(d.reset),
    "X-RateLimit-Bytes-Limit": String(d.bytesLimit),
    "X-RateLimit-Bytes-Remaining": String(d.bytesRemaining),
  };
  if (!d.ok) {
    h["Retry-After"] = String(d.retryAfter);
  }
  return h;
}

/** Current limit configuration (for the API docs / health endpoint). */
export function rateLimitConfig() {
  return {
    windowDays: 30,
    perMinutePerIp: perMin,
    perIpRequestsPerWindow: perIpReq,
    perIpBytesPerWindow: perIpBytes,
    globalBytesPerWindow: globalBytes,
  };
}

/** Reset counters (and optionally override limits). Test-only helper. */
export function __resetRateLimit(overrides?: {
  perMin?: number;
  perIpReq?: number;
  perIpBytes?: number;
  globalBytes?: number;
}): void {
  ipMap.clear();
  globalMonthStart = Date.now();
  globalBytesUsed = 0;
  if (overrides?.perMin !== undefined) perMin = overrides.perMin;
  if (overrides?.perIpReq !== undefined) perIpReq = overrides.perIpReq;
  if (overrides?.perIpBytes !== undefined) perIpBytes = overrides.perIpBytes;
  if (overrides?.globalBytes !== undefined) globalBytes = overrides.globalBytes;
}

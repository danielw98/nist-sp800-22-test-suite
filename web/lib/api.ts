import { NextResponse } from "next/server";

import { checkRateLimit, getClientIp, rateLimitHeaders } from "./ratelimit";

export { recordTraffic } from "./ratelimit";

/**
 * Apply the public rate limit to a request. Returns a ready 429 response in
 * `limited` when the caller is over a limit, the X-RateLimit-* `headers` to
 * attach to whatever response the route sends, and the resolved client `ip` so
 * the route can charge response traffic via recordTraffic().
 */
export function rateLimit(req: Request): {
  limited: NextResponse | null;
  headers: Record<string, string>;
  ip: string;
} {
  const ip = getClientIp(req);
  const decision = checkRateLimit(ip);
  const headers = rateLimitHeaders(decision);
  if (!decision.ok) {
    return {
      limited: NextResponse.json(
        {
          error: "rate_limited",
          reason: decision.reason,
          retryAfter: decision.retryAfter,
        },
        { status: 429, headers },
      ),
      headers,
      ip,
    };
  }
  return { limited: null, headers, ip };
}

/** Shorthand JSON error response with optional headers. */
export function jsonError(
  status: number,
  error: string,
  extra?: Record<string, unknown>,
  headers?: Record<string, string>,
) {
  return NextResponse.json({ error, ...extra }, { status, headers });
}

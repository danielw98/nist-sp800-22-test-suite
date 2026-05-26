import { NextResponse } from "next/server";

import { binariesExist } from "@/lib/paths";
import { rateLimitConfig } from "@/lib/ratelimit";
import { NIST_FILES } from "@/lib/spectral";
import { TEST_REGISTRY } from "@/lib/tests";

export const runtime = "nodejs";
export const dynamic = "force-dynamic";

/** GET /api/health - liveness + capability probe (not rate-limited). */
export async function GET() {
  return NextResponse.json({
    status: "ok",
    binaries: binariesExist(),
    rateLimit: rateLimitConfig(),
    files: NIST_FILES,
    tests: TEST_REGISTRY.map((t) => ({ id: t.id, name: t.name, status: t.status })),
  });
}

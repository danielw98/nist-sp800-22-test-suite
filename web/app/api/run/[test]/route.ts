import { NextResponse, type NextRequest } from "next/server";

import { jsonError, rateLimit, recordTraffic } from "@/lib/api";
import { nistTestPath } from "@/lib/paths";
import { runBinary } from "@/lib/run";
import { getTest } from "@/lib/tests";
import { MAX_BITS, runBodySchema, sanitizeBits } from "@/lib/validation";

export const runtime = "nodejs";
export const dynamic = "force-dynamic";

/** POST /api/run/{test} - run one NIST SP 800-22 test on a supplied bit string. */
export async function POST(req: NextRequest, ctx: { params: Promise<{ test: string }> }) {
  const { limited, headers, ip } = rateLimit(req);
  if (limited) {
    return limited;
  }

  const { test: testId } = await ctx.params;
  const def = getTest(testId);
  if (!def) {
    return jsonError(404, "unknown_test", { test: testId }, headers);
  }
  if (def.status !== "implemented") {
    return jsonError(
      501,
      "not_implemented",
      { test: testId, message: "this test is planned but not yet runnable" },
      headers,
    );
  }

  let body: unknown;
  try {
    body = await req.json();
  } catch {
    return jsonError(400, "invalid_json", undefined, headers);
  }

  const parsed = runBodySchema.safeParse(body);
  if (!parsed.success) {
    return jsonError(400, "invalid_request", { details: parsed.error.issues }, headers);
  }

  const bits = sanitizeBits(parsed.data.bits);
  if (bits.length < 2) {
    return jsonError(400, "no_bits", { message: "provide at least 2 bits (0/1)" }, headers);
  }
  if (bits.length > MAX_BITS) {
    return jsonError(400, "too_many_bits", { max: MAX_BITS, got: bits.length }, headers);
  }

  const args = ["--json", "-t", testId, "-a", String(parsed.data.alpha)];
  if (parsed.data.block !== undefined) {
    args.push("-b", String(parsed.data.block));
  }
  if (testId === "dft") {
    args.push("-m", parsed.data.method, "--spectrum");
  }
  args.push("-"); // read bits from stdin

  try {
    const res = await runBinary(nistTestPath(), args, bits, 30000);
    if (!res.stdout) {
      return jsonError(500, "binary_failed", { stderr: res.stderr.slice(0, 500) }, headers);
    }
    recordTraffic(ip, bits.length + res.stdout.length);
    return NextResponse.json(JSON.parse(res.stdout), { headers });
  } catch (err) {
    const msg = err instanceof Error ? err.message : String(err);
    if (msg === "timeout") {
      return jsonError(504, "timeout", { message: "computation exceeded 30s" }, headers);
    }
    return jsonError(500, "run_error", { message: msg }, headers);
  }
}

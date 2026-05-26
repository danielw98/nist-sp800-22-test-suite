import { NextResponse, type NextRequest } from "next/server";

import { jsonError, rateLimit, recordTraffic } from "@/lib/api";
import { dataFilePath, nistAssessPath } from "@/lib/paths";
import { runBinary } from "@/lib/run";
import { getTest } from "@/lib/tests";
import { assessSchema } from "@/lib/validation";

export const runtime = "nodejs";
export const dynamic = "force-dynamic";

/** POST /api/assess/{test} - multi-stream assessment over allowlisted NIST files. */
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
    return jsonError(501, "not_implemented", { test: testId }, headers);
  }

  let body: unknown;
  try {
    body = await req.json();
  } catch {
    return jsonError(400, "invalid_json", undefined, headers);
  }

  const parsed = assessSchema.safeParse(body);
  if (!parsed.success) {
    return jsonError(400, "invalid_request", { details: parsed.error.issues }, headers);
  }

  const paths: string[] = [];
  for (const name of parsed.data.files) {
    const p = dataFilePath(name);
    if (!p) {
      return jsonError(400, "unknown_file", { file: name }, headers);
    }
    paths.push(p);
  }

  const args = ["--json", "-t", testId, "-n", String(parsed.data.length), "-a", String(parsed.data.alpha)];
  if (parsed.data.maxStreams > 0) {
    args.push("-s", String(parsed.data.maxStreams));
  }
  if (parsed.data.block !== undefined) {
    args.push("-b", String(parsed.data.block));
  }
  args.push(...paths);

  try {
    const res = await runBinary(nistAssessPath(), args, null, 60000);
    if (!res.stdout) {
      return jsonError(500, "binary_failed", { stderr: res.stderr.slice(0, 500) }, headers);
    }
    recordTraffic(ip, res.stdout.length);
    return NextResponse.json(JSON.parse(res.stdout), { headers });
  } catch (err) {
    const msg = err instanceof Error ? err.message : String(err);
    if (msg === "timeout") {
      return jsonError(504, "timeout", { message: "assessment exceeded 60s" }, headers);
    }
    return jsonError(500, "assess_error", { message: msg }, headers);
  }
}

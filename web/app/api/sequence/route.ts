import fs from "node:fs/promises";

import { NextResponse, type NextRequest } from "next/server";

import { jsonError, rateLimit, recordTraffic } from "@/lib/api";
import { dataFilePath } from "@/lib/paths";

export const runtime = "nodejs";
export const dynamic = "force-dynamic";

const MAX_LEN = 65536;

/** Convert a file's raw bytes to a 0/1 bit string (ASCII filter or binary unpack). */
function toBits(name: string, raw: Buffer): string {
  if (name.endsWith(".sha1")) {
    // Binary file: unpack each byte MSB-first.
    let out = "";
    for (const b of raw) {
      for (let k = 7; k >= 0; k--) {
        out += (b >> k) & 1;
      }
    }
    return out;
  }
  // ASCII file: keep only '0'/'1'.
  return raw.toString("latin1").replace(/[^01]/g, "");
}

/** GET /api/sequence?file=&offset=&len= - a bounded slice of a NIST data file. */
export async function GET(req: NextRequest) {
  const { limited, headers, ip } = rateLimit(req);
  if (limited) {
    return limited;
  }

  const sp = req.nextUrl.searchParams;
  const name = sp.get("file") ?? "";
  const path = dataFilePath(name);
  if (!path) {
    return jsonError(400, "unknown_file", { file: name }, headers);
  }

  const offset = Math.max(0, Number(sp.get("offset") ?? 0) | 0);
  const len = Math.min(MAX_LEN, Math.max(1, Number(sp.get("len") ?? 1024) | 0));

  try {
    const raw = await fs.readFile(path);
    const bits = toBits(name, raw);
    const slice = bits.slice(offset, offset + len);
    recordTraffic(ip, slice.length);
    return NextResponse.json(
      {
        file: name,
        totalBits: bits.length,
        offset,
        len: Math.min(len, Math.max(0, bits.length - offset)),
        bits: slice,
      },
      { headers },
    );
  } catch (err) {
    const msg = err instanceof Error ? err.message : String(err);
    return jsonError(500, "read_error", { message: msg }, headers);
  }
}

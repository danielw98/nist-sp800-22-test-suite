import { z } from "zod";

/** Hard cap on input length so a request stays memory- and time-bounded.
 *  DFT (the heaviest test) pads via Bluestein to the next power of two >= 2n-1;
 *  measured peak RSS at 8,000,000 bits is ~1.0 GB in ~4.7 s, safely under the 2g
 *  container limit. Above ~8.38M the FFT padding doubles (2^24 -> 2^25), so this
 *  is the top of the safe band. See web/measure_dft_rss.py for the measurement. */
export const MAX_BITS = 8_000_000;

/** Request body for POST /api/run/{test}. Per-test params are optional; the
 *  binary applies its own default when one is omitted. */
export const runBodySchema = z.object({
  bits: z.string().max(MAX_BITS * 2),
  alpha: z.number().min(0.0001).max(0.5).optional().default(0.01),
  block: z.number().int().min(1).max(100000).optional(), // M/m for block tests
  method: z.enum(["auto", "fft", "direct"]).optional().default("auto"), // dft only
});

/** Request body for POST /api/assess/{test}. */
export const assessSchema = z.object({
  files: z.array(z.string()).min(1).max(6),
  length: z.number().int().min(1000).max(1_000_000).optional().default(10000),
  maxStreams: z.number().int().min(0).max(2000).optional().default(0),
  alpha: z.number().min(0.0001).max(0.5).optional().default(0.01),
  block: z.number().int().min(1).max(100000).optional(),
});

/** Strip everything except '0' and '1' from arbitrary text. */
export function sanitizeBits(text: string): string {
  return text.replace(/[^01]/g, "");
}

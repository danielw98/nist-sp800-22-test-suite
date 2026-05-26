/** Shared types for the spectral-test API and UI. */

/** One (downsampled) spectrum point: frequency index, magnitude, above-threshold flag. */
export interface SpectrumPoint {
  k: number;
  mag: number;
  above: boolean;
}

/** One per-state / per-template p-value (random excursions, variant, templates). */
export interface SubPValue {
  label: string;
  p: number;
}

/** Uniform result envelope from `nist_test --test <id> --json` (any test). */
export interface TestRunResult {
  test: string;
  n: number;
  p_value: number;
  passed: boolean;
  // Critical-value verdict (present when the statistic's distribution is known):
  // reject if |statistic| > critical (normal/half-normal) or statistic > critical
  // (chi-square). The p-value is kept as secondary info.
  statistic?: number;
  critical?: number;
  distribution?: string; // "normal (two-sided)" | "half-normal (upper)" | "chi-square (upper)"
  dof?: number; // chi-square only
  stats: Record<string, number>;
  p_values?: SubPValue[]; // multi-p tests (random excursions, variant)
  spectrum?: SpectrumPoint[]; // DFT only
}

/** Per-file outcome of a multi-stream assessment (mirrors `nist_assess --json`). */
export interface AssessFile {
  file: string;
  bitsAvailable: number;
  streamLength: number;
  streamsUsed: number;
  bins?: number[];
  uniformity?: number;
  uniformityPassed?: boolean;
  passCount?: number;
  sampleCount?: number;
  proportion?: number;
  proportionMin?: number;
  proportionMax?: number;
  proportionPassed?: boolean;
  verdict?: string;
}

export interface AssessResult {
  files: AssessFile[];
}

/** The NIST data files exposed by the app (allowlist). */
export const NIST_FILES = [
  "data.e",
  "data.pi",
  "data.sqrt2",
  "data.sqrt3",
  "data.sha1",
  "bits_nist_example.txt",
] as const;

export type NistFile = (typeof NIST_FILES)[number];

/** The 100-bit validation example from NIST SP 800-22 sec. 2.6.8. */
export const NIST_EXAMPLE_100 =
  "11001001000011111101101010100010001000010110100011" +
  "00001000110100110001001100011001100010100010111000";

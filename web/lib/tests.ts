// The NIST SP 800-22 test registry: the single source of truth for the suite.
// Drives the /tests catalog, the per-test pages, the API validation and the API
// docs. Adding a test = flip its `status` to "implemented" once the C++ side
// (one *_test.cpp + one registry line) lands; the slug must match the C++ id.
import { NIST_EXAMPLE_100 } from "./spectral";

export type TestStatus = "implemented" | "planned";

export interface ParamSpec {
  key: "alpha" | "block" | "method";
  label: string;
  kind: "number" | "select";
  default: number | string;
  min?: number;
  max?: number;
  step?: number;
  options?: { value: string; label: string }[];
  help: string;
}

export interface TestDef {
  id: string; // url slug; matches the C++ registry id and nist_test --test <id>
  number: number; // NIST SP 800-22 section (1..15)
  name: string;
  category: string;
  status: TestStatus;
  purpose: string;
  input: string;
  statistic: string;
  decision: string;
  params: ParamSpec[];
  viz: "spectrum" | "stats";
  minBits: number;
}

const ALPHA: ParamSpec = {
  key: "alpha",
  label: "alpha",
  kind: "number",
  default: 0.01,
  min: 0.0001,
  max: 0.5,
  step: 0.001,
  help: "Nivelul de semnificație: respinge dacă p < alpha.",
};
const blockParam = (label: string, def: number, help: string): ParamSpec => ({
  key: "block",
  label,
  kind: "number",
  default: def,
  min: 1,
  max: 100000,
  step: 1,
  help,
});
const METHOD: ParamSpec = {
  key: "method",
  label: "metoda",
  kind: "select",
  default: "auto",
  options: [
    { value: "auto", label: "auto (FFT)" },
    { value: "fft", label: "FFT" },
    { value: "direct", label: "direct O(n^2)" },
  ],
  help: "Motorul de transformare (doar testul spectral).",
};

export const TEST_REGISTRY: TestDef[] = [
  {
    id: "monobit",
    number: 1,
    name: "Frequency (Monobit)",
    category: "Frecvență",
    status: "implemented",
    purpose: "Verifică dacă proporția de 1 și 0 este apropiată de 1/2 pe toată secvența.",
    input: "O secvență de n biți.",
    statistic: "s_obs = |#1 - #0| / sqrt(n), apoi p = erfc(s_obs / sqrt(2)).",
    decision: "p < alpha => prea dezechilibrat, nealeator.",
    params: [ALPHA],
    viz: "stats",
    minBits: 100,
  },
  {
    id: "block-frequency",
    number: 2,
    name: "Frequency within a Block",
    category: "Frecvență",
    status: "implemented",
    purpose: "Verifică dacă proporția de 1 este apropiată de 1/2 în fiecare bloc de M biți.",
    input: "n biți, împărțiți în N = n/M blocuri.",
    statistic: "chi^2 = 4M sum (pi_i - 1/2)^2; p = igamc(N/2, chi^2/2).",
    decision: "p < alpha => dezechilibru local, nealeator.",
    params: [blockParam("M (biți/bloc)", 128, "Dimensiunea blocului M."), ALPHA],
    viz: "stats",
    minBits: 100,
  },
  {
    id: "runs",
    number: 3,
    name: "Runs",
    category: "Runs",
    status: "implemented",
    purpose: "Numără secvențele maximale de biți identici (runs) și le compară cu așteptarea.",
    input: "O secvență de n biți (cu proporție de 1 apropiată de 1/2).",
    statistic: "V_obs = numărul de runs; p = erfc(|V_obs - 2n pi(1-pi)| / (2 sqrt(2n) pi(1-pi))).",
    decision: "p < alpha => oscilație prea rapidă sau prea lentă, nealeator.",
    params: [ALPHA],
    viz: "stats",
    minBits: 100,
  },
  {
    id: "longest-run",
    number: 4,
    name: "Longest Run of Ones in a Block",
    category: "Runs",
    status: "implemented",
    purpose: "Compară cea mai lungă succesiune de 1 din fiecare bloc cu distribuția așteptată.",
    input: "n biți; M și clasele se aleg după n (M=8 pentru n<6272).",
    statistic: "chi^2 pe histograma celor mai lungi runs; p = igamc(K/2, chi^2/2).",
    decision: "p < alpha => grupări anormale de 1, nealeator.",
    params: [ALPHA],
    viz: "stats",
    minBits: 128,
  },
  {
    id: "rank",
    number: 5,
    name: "Binary Matrix Rank",
    category: "Algebră liniară",
    status: "implemented",
    purpose: "Verifică dependențele liniare între subșiruri de lungime fixă, prin rangul unor matrice peste GF(2).",
    input: "n biți, aranjați în matrice 32x32.",
    statistic: "Distribuția rangurilor (full / full-1 / mai mic); chi^2 -> p.",
    decision: "p < alpha => dependențe liniare, nealeator.",
    params: [ALPHA],
    viz: "stats",
    minBits: 38912,
  },
  {
    id: "dft",
    number: 6,
    name: "Discrete Fourier Transform (Spectral)",
    category: "Spectral",
    status: "implemented",
    purpose: "Detectează componente periodice în domeniul frecvenței. Testul tratat în profunzime aici.",
    input: "n biți, transformați în +/-1 și trecuți prin DFT.",
    statistic: "N1 = vârfuri sub pragul T = sqrt(ln(20)n); d = (N1 - N0)/sigma; p = erfc(|d|/sqrt(2)).",
    decision: "p < alpha => periodicitate, nealeator.",
    params: [METHOD, ALPHA],
    viz: "spectrum",
    minBits: 1000,
  },
  {
    id: "non-overlapping",
    number: 7,
    name: "Non-overlapping Template Matching",
    category: "Template",
    status: "implemented",
    purpose: "Numără aparițiile non-suprapuse ale unui șablon dat și le compară cu așteptarea.",
    input: "n biți + un șablon de m biți.",
    statistic: "chi^2 pe numărul de potriviri per bloc -> p.",
    decision: "p < alpha => prea multe/puține apariții, nealeator.",
    params: [blockParam("m (lungime sablon)", 9, "Lungimea șablonului m; șablonul folosit este 0^(m-1)1."), ALPHA],
    viz: "stats",
    minBits: 1000000,
  },
  {
    id: "overlapping",
    number: 8,
    name: "Overlapping Template Matching",
    category: "Template",
    status: "implemented",
    purpose: "Ca testul anterior, dar potrivirile pot fi suprapuse.",
    input: "n biți + un șablon de m biți (de obicei numai 1).",
    statistic: "chi^2 pe distribuția numărului de potriviri -> p.",
    decision: "p < alpha => abatere de la așteptare, nealeator.",
    params: [ALPHA],
    viz: "stats",
    minBits: 1000000,
  },
  {
    id: "universal",
    number: 9,
    name: "Maurer's Universal Statistical",
    category: "Compresie",
    status: "implemented",
    purpose: "Estimează compresibilitatea: o secvență aleatoare nu poate fi comprimată semnificativ.",
    input: "n biți, împărțiți în blocuri de L biți (inițializare + test).",
    statistic: "Distanța medie între apariții repetate de blocuri -> statistică normalizată -> p.",
    decision: "p < alpha => comprimabilă, nealeator.",
    params: [ALPHA],
    viz: "stats",
    minBits: 387840,
  },
  {
    id: "linear-complexity",
    number: 10,
    name: "Linear Complexity",
    category: "Complexitate",
    status: "implemented",
    purpose: "Lungimea celui mai scurt LFSR care generează secvența (prin Berlekamp-Massey).",
    input: "n biți, împărțiți în blocuri de M biți.",
    statistic: "chi^2 pe distribuția complexității liniare per bloc -> p.",
    decision: "p < alpha => complexitate prea mică/mare, nealeator.",
    params: [blockParam("M (lungime bloc)", 500, "Lungimea blocului M (NIST recomandă 500..5000)."), ALPHA],
    viz: "stats",
    minBits: 1000000,
  },
  {
    id: "serial",
    number: 11,
    name: "Serial",
    category: "Tipare",
    status: "implemented",
    purpose: "Verifică dacă toate tiparele de m biți (suprapuse) apar cu frecvențe egale.",
    input: "n biți; m = lungimea tiparului.",
    statistic: "psi^2 pe frecvențele tiparelor de m, m-1, m-2; două p-values (igamc).",
    decision: "p < alpha => tipare ne-uniforme, nealeator.",
    params: [blockParam("m (lungime tipar)", 2, "Lungimea tiparului m."), ALPHA],
    viz: "stats",
    minBits: 100,
  },
  {
    id: "approx-entropy",
    number: 12,
    name: "Approximate Entropy",
    category: "Tipare",
    status: "implemented",
    purpose: "Compară frecvența tiparelor de lungime m și m+1; aleator => entropie aproape maximă.",
    input: "n biți; m = lungimea blocului.",
    statistic: "ApEn = phi^m - phi^(m+1); chi^2 = 2n(ln2 - ApEn); p = igamc(2^(m-1), chi^2/2).",
    decision: "p < alpha => regularitate, nealeator.",
    params: [blockParam("m (lungime bloc)", 2, "Lungimea blocului m."), ALPHA],
    viz: "stats",
    minBits: 100,
  },
  {
    id: "cusum",
    number: 13,
    name: "Cumulative Sums (Cusum)",
    category: "Mers aleator",
    status: "implemented",
    purpose: "Măsoară cât de departe ajunge mersul aleator al sumelor parțiale de +/-1.",
    input: "n biți, transformați în +/-1.",
    statistic: "z = max |suma parțială|; p (înainte și înapoi) prin CDF normală.",
    decision: "p < alpha => derivă sistematică, nealeator.",
    params: [ALPHA],
    viz: "stats",
    minBits: 100,
  },
  {
    id: "random-excursions",
    number: 14,
    name: "Random Excursions",
    category: "Mers aleator",
    status: "implemented",
    purpose: "Numără vizitele la fiecare stare în ciclurile mersului aleator (stări -4..-1, 1..4).",
    input: "n biți -> mers aleator cumulativ.",
    statistic: "chi^2 per stare pe distribuția vizitelor -> opt p-values.",
    decision: "p < alpha pentru o stare => abatere, nealeator.",
    params: [ALPHA],
    viz: "stats",
    minBits: 1000000,
  },
  {
    id: "random-excursions-variant",
    number: 15,
    name: "Random Excursions Variant",
    category: "Mers aleator",
    status: "implemented",
    purpose: "Variantă: numărul total de vizite la 18 stări (-9..-1, 1..9) în tot mersul.",
    input: "n biți -> mers aleator cumulativ.",
    statistic: "Statistică normalizată per stare -> 18 p-values.",
    decision: "p < alpha pentru o stare => abatere, nealeator.",
    params: [ALPHA],
    viz: "stats",
    minBits: 1000000,
  },
];

export function getTest(id: string): TestDef | undefined {
  return TEST_REGISTRY.find((t) => t.id === id);
}

export const IMPLEMENTED_TESTS = TEST_REGISTRY.filter((t) => t.status === "implemented");

// A handy paste-in example for the runnable small-n tests.
export const EXAMPLE_PRESET = { label: "Exemplul NIST (100 biți)", bits: NIST_EXAMPLE_100 };

/** Group the registry by category, preserving first-appearance order. */
export function groupTestsByCategory(
  tests: TestDef[] = TEST_REGISTRY,
): { category: string; tests: TestDef[] }[] {
  const order: string[] = [];
  const byCat = new Map<string, TestDef[]>();
  for (const t of tests) {
    if (!byCat.has(t.category)) {
      byCat.set(t.category, []);
      order.push(t.category);
    }
    byCat.get(t.category)!.push(t);
  }
  return order.map((category) => ({ category, tests: byCat.get(category)! }));
}

/** NIST SP 800-22 section for a test (the suite is sections 2.1..2.15). */
export function nistSection(def: TestDef): string {
  return `2.${def.number}`;
}

/** What is notable in the JSON response for this test, beyond the standard fields
 *  (n, p_value, passed, statistic, critical, distribution). */
export function responseNote(def: TestDef): string {
  if (def.viz === "spectrum")
    return "Adaugă `spectrum[]` (până la ~4000 de puncte) plus stats cu N1/N0/d/threshold.";
  if (def.id === "random-excursions")
    return "Adaugă `p_values[]` pentru 8 stări (x = -4..-1, 1..4) plus stats.J.";
  if (def.id === "random-excursions-variant")
    return "Adaugă `p_values[]` pentru 18 stări (x = -9..-1, 1..9) plus stats.J.";
  if (def.id === "serial") return "Acceptă `block` (m); întoarce două p-values în stats.";
  if (def.params.some((p) => p.key === "block"))
    return "Acceptă parametrul `block`; răspuns standard.";
  return "Răspuns standard: n, p_value, statistic, critical, distribution.";
}

"use client";

import { useEffect, useRef, useState } from "react";

import { SpectrumChart } from "@/components/SpectrumChart";
import { NIST_EXAMPLE_100 as NIST_EXAMPLE, type TestRunResult } from "@/lib/spectral";

/** Deterministic pseudo-random bits (LCG), so the case is reproducible. */
function lcgBits(n: number, seed: number): string {
  let s = seed >>> 0;
  let out = "";
  for (let i = 0; i < n; i++) {
    s = (1664525 * s + 1013904223) >>> 0;
    out += (s >>> 31) & 1;
  }
  return out;
}

interface Case {
  name: string;
  desc: string;
  bits: string;
  expected: string;
  check: (r: TestRunResult) => boolean;
}

const CASES: Case[] = [
  {
    name: "Exemplul NIST (n=100)",
    desc: "Trebuie să coincidă cu codul de referință NIST.",
    bits: NIST_EXAMPLE,
    expected: "N1 = 48, p = 0.646355, aleator",
    check: (r) => r.stats.N1 === 48 && Math.abs(r.p_value - 0.646355) < 1e-4,
  },
  {
    name: "Numai 1 (n=4096)",
    desc: "Componenta DC domină spectrul.",
    bits: "1".repeat(4096),
    expected: "respins (p ~ 0)",
    check: (r) => !r.passed,
  },
  {
    name: "Periodic 0001 (n=4096)",
    desc: "Perioada 4: vârf spectral puternic în bandă.",
    bits: "0001".repeat(1024),
    expected: "respins (p ~ 0)",
    check: (r) => !r.passed,
  },
  {
    name: "Alternant 01 - perioada 2 (n=4096)",
    desc: "Vârful cade pe Nyquist (exclus); respins indirect: toate componentele numărate sunt nule.",
    bits: "01".repeat(2048),
    expected: "respins (p ~ 0)",
    check: (r) => !r.passed,
  },
  {
    name: "Aleator determinist (n=8192)",
    desc: "Generator bun (LCG cu sămânță fixă).",
    bits: lcgBits(8192, 12345),
    expected: "aleator (nu se respinge)",
    check: (r) => r.passed,
  },
];

type Row = {
  status: "pending" | "running" | "done" | "error";
  result?: TestRunResult;
  pass?: boolean;
  error?: string;
};

export default function ValidationPage() {
  const [rows, setRows] = useState<Row[]>(CASES.map(() => ({ status: "pending" })));
  const [expanded, setExpanded] = useState<number | null>(0);
  const started = useRef(false);

  useEffect(() => {
    if (started.current) return; // run once (guards React strict-mode double effect)
    started.current = true;

    (async () => {
      for (let i = 0; i < CASES.length; i++) {
        setRows((r) => r.map((x, j) => (j === i ? { status: "running" } : x)));
        try {
          const res = await fetch("/api/run/dft", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ bits: CASES[i].bits }),
          });
          const data = await res.json();
          if (!res.ok) {
            setRows((r) => r.map((x, j) => (j === i ? { status: "error", error: data.error } : x)));
          } else {
            const pass = CASES[i].check(data as TestRunResult);
            setRows((r) =>
              r.map((x, j) => (j === i ? { status: "done", result: data, pass } : x)),
            );
          }
        } catch (e) {
          setRows((r) =>
            r.map((x, j) =>
              j === i ? { status: "error", error: e instanceof Error ? e.message : String(e) } : x,
            ),
          );
        }
        // Small gap between cases (limits are generous; localhost is exempt).
        await new Promise((res) => setTimeout(res, 350));
      }
    })();
  }, []);

  const allDone = rows.every((r) => r.status === "done" || r.status === "error");
  const allPass = rows.every((r) => r.status === "done" && r.pass);

  return (
    <div className="space-y-5">
      <div>
        <h1 className="text-2xl font-bold">Validare live - testul spectral (DFT)</h1>
        <p className="text-muted mt-1 max-w-3xl">
          Cazuri cu răspuns cunoscut pentru <strong className="text-foreground">testul
          spectral (DFT)</strong>, rulate prin API-ul real (binarul C++) și secvențiate
          pentru a respecta limitele de trafic. Verificăm că fiecare rezultat corespunde
          așteptării. Validarea celorlalte 14 teste se face din pagina fiecărui test.
        </p>
      </div>

      {allDone && (
        <div
          className={`rounded-lg px-4 py-3 font-medium ${
            allPass ? "bg-ok-weak text-ok border border-ok" : "bg-danger-weak text-danger border border-danger"
          }`}
        >
          {allPass ? "Toate cazurile au trecut." : "Unele cazuri au eșuat."}
        </div>
      )}

      <div className="space-y-2">
        {CASES.map((c, i) => {
          const row = rows[i];
          return (
            <div key={c.name} className="rounded-lg border border-border bg-surface">
              <button
                onClick={() => setExpanded(expanded === i ? null : i)}
                className="w-full flex items-center gap-3 px-4 py-3 text-left"
              >
                <Badge row={row} />
                <div className="flex-1 min-w-0">
                  <div className="font-medium">{c.name}</div>
                  <div className="text-xs text-muted">{c.desc}</div>
                </div>
                <div className="text-xs text-muted text-right whitespace-nowrap">
                  așteptat: {c.expected}
                  {row.result && (
                    <div className="font-mono">
                      N1={row.result.stats.N1}, p={row.result.p_value.toFixed(4)}
                    </div>
                  )}
                </div>
              </button>
              {expanded === i && row.result?.spectrum && (
                <div className="border-t border-border p-4">
                  <SpectrumChart points={row.result.spectrum} threshold={row.result.stats.threshold} height={280} />
                </div>
              )}
            </div>
          );
        })}
      </div>
    </div>
  );
}

function Badge({ row }: { row: Row }) {
  if (row.status === "pending") return <span className="text-muted text-sm w-16">în așteptare</span>;
  if (row.status === "running") return <span className="text-accent text-sm w-16">rulează...</span>;
  if (row.status === "error") return <span className="text-danger text-sm w-16">eroare</span>;
  return (
    <span className={`text-sm font-bold w-16 ${row.pass ? "text-ok" : "text-danger"}`}>
      {row.pass ? "PASS" : "FAIL"}
    </span>
  );
}

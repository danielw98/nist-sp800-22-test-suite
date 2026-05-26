"use client";

import { useState } from "react";

import { ResultCard } from "@/components/ResultCard";
import { SpectrumChart } from "@/components/SpectrumChart";
import { Button, fieldClasses } from "@/components/ui";
import { NIST_EXAMPLE_100, type TestRunResult } from "@/lib/spectral";
import type { ParamSpec, TestDef } from "@/lib/tests";

type Params = Record<string, number | string>;

function ParamControl({
  spec,
  value,
  onChange,
}: {
  spec: ParamSpec;
  value: number | string;
  onChange: (v: number | string) => void;
}) {
  return (
    <label className="flex items-center gap-2 text-sm" title={spec.help}>
      <span className="text-muted">{spec.label}</span>
      {spec.kind === "select" ? (
        <select value={String(value)} onChange={(e) => onChange(e.target.value)} className={fieldClasses}>
          {spec.options?.map((o) => (
            <option key={o.value} value={o.value}>
              {o.label}
            </option>
          ))}
        </select>
      ) : (
        <input
          type="number"
          step={spec.step}
          min={spec.min}
          max={spec.max}
          value={value}
          onChange={(e) => onChange(Number(e.target.value))}
          className={`${fieldClasses} w-28`}
        />
      )}
    </label>
  );
}

/** Universal "paste a sequence and run" UI, driven by a test's registry entry. */
export function TestRunner({ def }: { def: TestDef }) {
  const [bits, setBits] = useState(NIST_EXAMPLE_100);
  const [params, setParams] = useState<Params>(() => {
    const initial: Params = {};
    for (const spec of def.params) {
      initial[spec.key] = spec.default;
    }
    return initial;
  });
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [result, setResult] = useState<TestRunResult | null>(null);

  const bitCount = (bits.match(/[01]/g) || []).length;
  const alpha = Number(params.alpha ?? 0.01);
  const setParam = (key: string, v: number | string) => setParams((p) => ({ ...p, [key]: v }));

  async function run() {
    setLoading(true);
    setError(null);
    setResult(null);
    try {
      const body: Record<string, unknown> = { bits };
      for (const spec of def.params) {
        body[spec.key] = params[spec.key];
      }
      const res = await fetch(`/api/run/${def.id}`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(body),
      });
      const data = await res.json();
      if (!res.ok) {
        setError(`${data.error}${data.message ? ": " + data.message : ""}`);
      } else {
        setResult(data as TestRunResult);
      }
    } catch (e) {
      setError(e instanceof Error ? e.message : String(e));
    } finally {
      setLoading(false);
    }
  }

  function randomBits() {
    const len = Math.max(def.minBits, 1024);
    setBits(Array.from({ length: len }, () => (Math.random() < 0.5 ? "0" : "1")).join(""));
  }

  return (
    <div className="space-y-4">
      <textarea
        value={bits}
        onChange={(e) => setBits(e.target.value)}
        spellCheck={false}
        className="w-full h-40 rounded-xl border border-border bg-surface p-4 font-mono text-sm leading-relaxed focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-accent/40"
        placeholder="0101110010..."
      />

      <div className="flex flex-wrap items-center gap-x-6 gap-y-3">
        {def.params.map((spec) => (
          <ParamControl
            key={spec.key}
            spec={spec}
            value={params[spec.key]}
            onChange={(v) => setParam(spec.key, v)}
          />
        ))}
        <Button variant="secondary" size="sm" onClick={() => setBits(NIST_EXAMPLE_100)}>
          Exemplul NIST
        </Button>
        <Button variant="secondary" size="sm" onClick={randomBits}>
          Aleator
        </Button>
        <span className="text-sm text-muted">{bitCount.toLocaleString()} biți</span>
        <Button size="lg" onClick={run} disabled={loading} className="ml-auto">
          {loading ? "Se rulează..." : "Rulează"}
        </Button>
      </div>

      {error && (
        <div className="rounded-xl border border-danger bg-danger-weak text-danger px-4 py-3">
          {error}
        </div>
      )}

      {result && (
        <div className="space-y-5">
          <ResultCard result={result} alpha={alpha} />
          {def.viz === "spectrum" && result.spectrum && (
            <div className="rounded-xl border border-border bg-surface p-5">
              <h3 className="font-semibold mb-1">Spectru de amplitudine</h3>
              <p className="text-sm text-muted mb-3 max-w-3xl">
                Modulele |S[k]| ale primelor n/2 componente. Punctele roșii depășesc pragul T;
                sub H0 ar trebui să fie ~5%. Eșantionat la cel mult 4000 de puncte.
              </p>
              <SpectrumChart points={result.spectrum} threshold={result.stats.threshold} />
            </div>
          )}
        </div>
      )}
    </div>
  );
}

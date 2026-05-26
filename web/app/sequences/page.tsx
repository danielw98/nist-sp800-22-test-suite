"use client";

import { useState } from "react";

import { SpectrumChart } from "@/components/SpectrumChart";
import type { AssessFile, TestRunResult } from "@/lib/spectral";
import nistRaw from "@/data-static/nist-results.json";
import assessRaw from "@/data-static/assess-results.json";

const nist = nistRaw as (TestRunResult & { file: string })[];
const assess = (assessRaw as { files: AssessFile[] }).files;

const LABELS: Record<string, string> = {
  "data.e": "e (numărul lui Euler)",
  "data.pi": "pi",
  "data.sqrt2": "radical din 2",
  "data.sqrt3": "radical din 3",
  "data.sha1": "generator SHA-1",
};

function basename(p: string): string {
  return p.split(/[\\/]/).pop() ?? p;
}

export default function SequencesPage() {
  const [selected, setSelected] = useState("data.pi");
  const current = nist.find((r) => r.file === selected);

  return (
    <div className="space-y-6">
      <div>
        <h1 className="text-2xl font-bold">Secvențele de date NIST - analiză spectrală (DFT)</h1>
        <p className="text-muted mt-1 max-w-3xl">
          Aceleași secvențe pe care NIST și-a validat suita: dezvoltările binare ale
          constantelor e, pi, radical din 2 și 3, plus un generator bazat pe SHA-1. Aici sunt
          trecute prin <strong className="text-foreground">testul spectral (DFT)</strong>:
          tabelul arată analiza pe 100 de fluxuri de 10000 de biți (proporție de treceri +
          uniformitatea valorilor p). Toate trec. Rularea oricăruia dintre cele 15 teste se
          face din pagina fiecărui test.
        </p>
      </div>

      <div className="overflow-x-auto rounded-lg border border-border">
        <table className="w-full text-sm">
          <thead className="bg-accent-weak text-left">
            <tr>
              <th className="px-3 py-2">Fișier</th>
              <th className="px-3 py-2">Biți</th>
              <th className="px-3 py-2">Fluxuri</th>
              <th className="px-3 py-2">Proporție</th>
              <th className="px-3 py-2">Uniformitate p</th>
              <th className="px-3 py-2">Verdict</th>
            </tr>
          </thead>
          <tbody>
            {assess.map((f) => {
              const name = basename(f.file);
              return (
                <tr key={name} className="border-t border-border">
                  <td className="px-3 py-2">
                    <span className="font-mono">{name}</span>
                    <span className="text-muted"> - {LABELS[name]}</span>
                  </td>
                  <td className="px-3 py-2">{f.bitsAvailable?.toLocaleString()}</td>
                  <td className="px-3 py-2">{f.streamsUsed}</td>
                  <td className="px-3 py-2 font-mono">
                    {f.passCount}/{f.sampleCount}
                  </td>
                  <td className="px-3 py-2 font-mono">{f.uniformity?.toFixed(6)}</td>
                  <td className="px-3 py-2">
                    <span
                      className={`rounded px-2 py-0.5 text-xs font-medium ${
                        f.verdict === "PASS" ? "bg-ok-weak text-ok" : "bg-danger-weak text-danger"
                      }`}
                    >
                      {f.verdict}
                    </span>
                  </td>
                </tr>
              );
            })}
          </tbody>
        </table>
      </div>

      <div className="flex flex-wrap gap-2">
        {nist.map((r) => (
          <button
            key={r.file}
            onClick={() => setSelected(r.file)}
            className={`rounded-lg border px-3 py-1.5 text-sm font-mono ${
              selected === r.file ? "border-accent bg-accent-weak text-accent" : "border-border"
            }`}
          >
            {r.file}
          </button>
        ))}
      </div>

      {current && (
        <div className="rounded-lg border border-border bg-surface p-4 space-y-3">
          <h2 className="font-medium">
            Testul pe întregul fișier <span className="font-mono">{current.file}</span> (
            {current.n.toLocaleString()} biți)
          </h2>
          <div className="grid grid-cols-2 sm:grid-cols-4 lg:grid-cols-6 gap-3 text-sm">
            <Stat label="N1" value={current.stats.N1.toLocaleString()} />
            <Stat label="N0" value={Math.round(current.stats.N0).toLocaleString()} />
            <Stat label="d" value={current.stats.d.toFixed(4)} />
            <Stat label="p-value" value={current.p_value.toFixed(6)} />
            <Stat label="T" value={current.stats.threshold.toFixed(1)} />
            <Stat label="verdict" value={current.passed ? "PASS" : "FAIL"} />
          </div>
          {current.spectrum && (
            <SpectrumChart points={current.spectrum} threshold={current.stats.threshold} />
          )}
        </div>
      )}
    </div>
  );
}

function Stat({ label, value }: { label: string; value: string }) {
  return (
    <div className="rounded-lg border border-border px-3 py-2">
      <div className="text-xs text-muted">{label}</div>
      <div className="font-mono">{value}</div>
    </div>
  );
}

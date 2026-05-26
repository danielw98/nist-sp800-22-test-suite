import katex from "katex";
import "katex/dist/katex.min.css";

import type { TestRunResult } from "@/lib/spectral";

function fmt(value: number): string {
  if (Number.isInteger(value)) {
    return value.toLocaleString();
  }
  return parseFloat(value.toPrecision(6)).toString();
}

/** Inline KaTeX (the runner verdict is rendered as real math, like the paper pages). */
function KMath({ tex }: { tex: string }) {
  const html = katex.renderToString(tex, { throwOnError: false });
  return <span dangerouslySetInnerHTML={{ __html: html }} />;
}

function Stat({ label, value }: { label: string; value: string }) {
  return (
    <div className="rounded-xl border border-border bg-surface px-4 py-3">
      <div className="text-xs text-muted uppercase tracking-wide">{label}</div>
      <div className="font-mono text-xl mt-0.5">{value}</div>
    </div>
  );
}

/** The critical-value comparison as a LaTeX string (statistic vs table critical value). */
function verdictTex(result: TestRunResult, alpha: number): string {
  if (result.statistic !== undefined && result.critical !== undefined && result.distribution) {
    const rel = result.passed ? "<" : "\\ge";
    if (result.distribution.startsWith("chi")) {
      return `\\chi^2 = ${fmt(result.statistic)} ${rel} \\chi^2_{${alpha},\\,${result.dof}} = ${fmt(result.critical)}`;
    }
    return `\\lvert T\\rvert = ${fmt(Math.abs(result.statistic))} ${rel} z_{1-\\alpha/2} = ${fmt(result.critical)}`;
  }
  // No tabulated critical value (e.g. cusum): fall back to p vs alpha.
  const rel = result.passed ? "\\ge" : "<";
  return `p = ${fmt(result.p_value)} ${rel} \\alpha = ${alpha}`;
}

/** Uniform result panel: stats + critical-value verdict + per-state p-values. */
export function ResultCard({ result, alpha }: { result: TestRunResult; alpha: number }) {
  const hasCritical = result.statistic !== undefined && result.critical !== undefined;
  return (
    <div className="space-y-5">
      <div className="grid grid-cols-2 sm:grid-cols-3 lg:grid-cols-6 gap-3">
        <Stat label="n" value={result.n.toLocaleString()} />
        {Object.entries(result.stats).map(([key, value]) => (
          <Stat key={key} label={key} value={fmt(value)} />
        ))}
        <Stat label="p-value" value={result.p_value.toFixed(6)} />
      </div>

      {hasCritical && (
        <div className="grid grid-cols-2 sm:grid-cols-3 gap-3">
          <Stat label="statistică" value={fmt(result.statistic as number)} />
          <Stat
            label={result.distribution?.startsWith("chi") ? `valoare critică (dof=${result.dof})` : "valoare critică"}
            value={fmt(result.critical as number)}
          />
          <Stat label="distribuție" value={result.distribution ?? ""} />
        </div>
      )}

      {result.p_values && result.p_values.length > 0 && (
        <div>
          <div className="text-xs text-muted uppercase tracking-wide mb-2">p-value per stare</div>
          <div className="grid grid-cols-3 sm:grid-cols-6 gap-2 font-mono text-sm">
            {result.p_values.map((pv) => (
              <div
                key={pv.label}
                className={`rounded-lg border px-3 py-2 flex justify-between gap-2 ${
                  pv.p < alpha ? "border-danger text-danger bg-danger-weak" : "border-border bg-surface"
                }`}
              >
                <span className="text-muted">{pv.label}</span>
                <span>{pv.p.toFixed(4)}</span>
              </div>
            ))}
          </div>
        </div>
      )}

      <div
        className={`rounded-xl px-5 py-4 border ${
          result.passed ? "bg-ok-weak text-ok border-ok" : "bg-danger-weak text-danger border-danger"
        }`}
      >
        <div className="text-lg font-medium">
          {result.passed ? "Aleator - nu se respinge ipoteza nulă" : "Nealeator - se respinge ipoteza nulă"}
        </div>
        <div className="mt-1.5 text-base">
          <KMath tex={verdictTex(result, alpha)} />
          {hasCritical && <span className="text-muted ml-3">(p = {result.p_value.toFixed(6)})</span>}
        </div>
      </div>
    </div>
  );
}

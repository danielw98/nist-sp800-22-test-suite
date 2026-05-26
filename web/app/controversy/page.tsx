"use client";

import { useState } from "react";

import Plot from "@/components/Plot";
import { CHART_FONT, chartColors, useIsDark } from "@/components/useIsDark";
import { erfc } from "@/lib/mathx";
import exampleData from "@/data-static/figures/example_threshold_46_vs_48.json";

const MAGS: number[] = exampleData.mag;
const T: number = exampleData.T;
const N = exampleData.n;
const SORTED = [...MAGS].sort((a, b) => a - b);

export default function ControversyPage() {
  const [scale, setScale] = useState(1.05);
  const tEff = T / scale;

  const n1 = MAGS.filter((m) => m < tEff).length;
  const n0 = (0.95 * N) / 2;
  const d = (n1 - n0) / Math.sqrt((N / 4) * 0.95 * 0.05);
  const p = erfc(Math.abs(d) / Math.SQRT2);
  const c = chartColors(useIsDark());

  const colors = SORTED.map((m) => (m >= tEff && m < T ? "#f59e0b" : c.line));

  return (
    <div className="space-y-6">
      <div>
        <h1 className="text-2xl font-bold">Controversă 46 vs 48</h1>
        <p className="text-muted mt-1 max-w-3xl">
          Pe exemplul de 100 de biți din secțiunea 2.6.8, o transformată corectă dă{" "}
          <span className="font-mono">N1 = 48</span>, iar documentația raportează 46.
          Numărătoarea este foarte sensibilă la prag: deplasarea cursorului coboară
          pragul efectiv (<span className="font-mono">T / factor</span>) și arată cum, la
          un prag cu aproximativ 5% mai mic, două vârfuri sunt excluse și{" "}
          <span className="font-mono">N1</span> devine 46. Reproducerea valorii
          documentate ar cere deci o corecție a pragului - exact genul propus în
          literatură.
        </p>
      </div>

      <div className="rounded-lg border border-border bg-surface p-4">
        <label className="flex items-center gap-4">
          <span className="text-sm font-medium whitespace-nowrap">factor = {scale.toFixed(3)}</span>
          <input
            type="range"
            min={1.0}
            max={1.1}
            step={0.005}
            value={scale}
            onChange={(e) => setScale(Number(e.target.value))}
            className="flex-1"
          />
        </label>
        <div className="grid grid-cols-2 sm:grid-cols-4 gap-3 mt-4">
          <Stat label="prag efectiv T/factor" value={tEff.toFixed(3)} />
          <Stat label="N1" value={String(n1)} highlight={n1 === 46 || n1 === 48} />
          <Stat label="d" value={d.toFixed(4)} />
          <Stat label="p-value" value={p.toFixed(6)} />
        </div>
      </div>

      <div className="rounded-lg border border-border bg-surface p-4">
        <Plot
          data={[
            {
              x: SORTED.map((_, i) => i),
              y: SORTED,
              type: "bar",
              marker: { color: colors },
              hovertemplate: "|S[k]| = %{y:.2f}<extra></extra>",
            },
          ]}
          layout={{
            height: 420,
            margin: { l: 52, r: 16, t: 12, b: 44 },
            paper_bgcolor: "rgba(0,0,0,0)",
            plot_bgcolor: "rgba(0,0,0,0)",
            font: { family: CHART_FONT, color: c.font, size: 14 },
            xaxis: { title: { text: "vârfuri sortate crescător", font: { size: 14 } }, gridcolor: c.grid, tickfont: { size: 14 } },
            yaxis: { title: { text: "magnitudine", font: { size: 14 } }, gridcolor: c.grid, tickfont: { size: 14 } },
            shapes: [
              { type: "line", xref: "paper", x0: 0, x1: 1, y0: T, y1: T, line: { color: c.danger, width: 1.5, dash: "dash" } },
              { type: "line", xref: "paper", x0: 0, x1: 1, y0: tEff, y1: tEff, line: { color: c.accent2, width: 1.5, dash: "dot" } },
            ],
            annotations: [
              { xref: "paper", x: 0, y: T, xanchor: "left", yanchor: "bottom", text: `T corect = ${T.toFixed(2)}`, showarrow: false, font: { color: c.danger, size: 14 } },
              { xref: "paper", x: 0, y: tEff, xanchor: "left", yanchor: "top", text: `T efectiv = ${tEff.toFixed(2)}`, showarrow: false, font: { color: c.accent2, size: 14 } },
            ],
          }}
          config={{ responsive: true, displaylogo: false }}
          useResizeHandler
          style={{ width: "100%" }}
        />
        <p className="text-xs text-muted mt-2">
          Barele portocalii sunt vârfurile aflate între pragul efectiv și pragul corect:
          exact cele care comută numărarea când se aplică factorul de scalare.
        </p>
      </div>
    </div>
  );
}

function Stat({ label, value, highlight }: { label: string; value: string; highlight?: boolean }) {
  return (
    <div className={`rounded-lg border px-3 py-2 ${highlight ? "border-accent bg-accent-weak" : "border-border"}`}>
      <div className="text-xs text-muted">{label}</div>
      <div className="font-mono text-lg">{value}</div>
    </div>
  );
}

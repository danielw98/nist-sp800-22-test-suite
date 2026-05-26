"use client";

import Plot from "./Plot";
import { CHART_FONT, chartColors, useIsDark } from "./useIsDark";
import type { SpectrumPoint } from "@/lib/spectral";

/**
 * Interactive spectrum plot: |S[k]| vs frequency, the 95% threshold as a dashed
 * line, and peaks above the threshold highlighted. Theme-aware.
 */
export function SpectrumChart({
  points,
  threshold,
  height = 420,
}: {
  points: SpectrumPoint[];
  threshold: number;
  height?: number;
}) {
  const c = chartColors(useIsDark());
  const ks = points.map((p) => p.k);
  const mags = points.map((p) => p.mag);
  const above = points.filter((p) => p.above);

  return (
    <Plot
      data={[
        {
          x: ks,
          y: mags,
          type: "scattergl",
          mode: "lines",
          line: { color: c.line, width: 1.4 },
          name: "|S[k]|",
          hovertemplate: "k=%{x}<br>|S[k]|=%{y:.2f}<extra></extra>",
        },
        {
          x: above.map((p) => p.k),
          y: above.map((p) => p.mag),
          type: "scattergl",
          mode: "markers",
          marker: { color: c.danger, size: 6 },
          name: "peste prag",
          hovertemplate: "k=%{x}<br>|S[k]|=%{y:.2f} (peste T)<extra></extra>",
        },
      ]}
      layout={{
        height,
        margin: { l: 56, r: 16, t: 12, b: 46 },
        paper_bgcolor: "rgba(0,0,0,0)",
        plot_bgcolor: "rgba(0,0,0,0)",
        font: { family: CHART_FONT, color: c.font, size: 15 },
        xaxis: { title: { text: "frecvență k", font: { size: 15 } }, gridcolor: c.grid, zerolinecolor: c.grid, tickfont: { size: 15 } },
        yaxis: { title: { text: "|S[k]|", font: { size: 15 } }, gridcolor: c.grid, zerolinecolor: c.grid, tickfont: { size: 15 } },
        legend: { orientation: "h", y: 1.12, font: { size: 15 } },
        shapes: [
          {
            type: "line",
            xref: "paper",
            x0: 0,
            x1: 1,
            y0: threshold,
            y1: threshold,
            line: { color: c.danger, width: 1.5, dash: "dash" },
          },
        ],
        annotations: [
          {
            xref: "paper",
            x: 1,
            y: threshold,
            xanchor: "right",
            yanchor: "bottom",
            text: `prag T = ${threshold.toFixed(2)}`,
            showarrow: false,
            font: { color: c.danger, size: 14 },
          },
        ],
      }}
      config={{ displayModeBar: true, responsive: true, displaylogo: false }}
      useResizeHandler
      style={{ width: "100%" }}
    />
  );
}

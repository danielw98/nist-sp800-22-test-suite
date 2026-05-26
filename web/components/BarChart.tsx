"use client";

import Plot from "./Plot";
import { CHART_FONT, chartColors, useIsDark } from "./useIsDark";

/** Theme-aware interactive bar chart (used for p-value histograms). */
export function BarChart({
  x,
  y,
  expected,
  xtitle = "",
  ytitle = "",
  height = 340,
}: {
  x: (string | number)[];
  y: number[];
  expected?: number;
  xtitle?: string;
  ytitle?: string;
  height?: number;
}) {
  const c = chartColors(useIsDark());
  return (
    <Plot
      data={[
        {
          x,
          y,
          type: "bar",
          marker: { color: c.line },
          hovertemplate: "%{x}: %{y}<extra></extra>",
        },
      ]}
      layout={{
        height,
        margin: { l: 52, r: 16, t: 12, b: 46 },
        bargap: 0.05,
        paper_bgcolor: "rgba(0,0,0,0)",
        plot_bgcolor: "rgba(0,0,0,0)",
        font: { family: CHART_FONT, color: c.font, size: 15 },
        xaxis: { title: { text: xtitle, font: { size: 15 } }, gridcolor: c.grid, tickfont: { size: 15 } },
        yaxis: { title: { text: ytitle, font: { size: 15 } }, gridcolor: c.grid, tickfont: { size: 15 } },
        shapes:
          expected !== undefined
            ? [
                {
                  type: "line",
                  xref: "paper",
                  x0: 0,
                  x1: 1,
                  y0: expected,
                  y1: expected,
                  line: { color: c.danger, width: 1.5, dash: "dash" },
                },
              ]
            : [],
      }}
      config={{ responsive: true, displaylogo: false }}
      useResizeHandler
      style={{ width: "100%" }}
    />
  );
}

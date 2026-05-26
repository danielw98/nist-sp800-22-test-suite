"use client";

import dynamic from "next/dynamic";
import type { ComponentType, CSSProperties } from "react";

/** Props we actually pass to Plotly (kept loose; Plotly's own types are heavy). */
export type PlotProps = {
  data: unknown[];
  layout?: unknown;
  config?: unknown;
  useResizeHandler?: boolean;
  style?: CSSProperties;
};

/** Plotly chart, loaded client-side only (plotly.js is not SSR-safe). */
const Plot = dynamic(() => import("./PlotlyChart"), {
  ssr: false,
  loading: () => <div className="h-80 animate-pulse rounded bg-accent-weak" />,
}) as ComponentType<PlotProps>;

export default Plot;

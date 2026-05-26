"use client";

import { useEffect, useState } from "react";

/** Tracks whether <html> has the `dark` class; updates live when the theme toggles. */
export function useIsDark(): boolean {
  const [dark, setDark] = useState(false);
  useEffect(() => {
    const el = document.documentElement;
    const update = () => setDark(el.classList.contains("dark"));
    update();
    const obs = new MutationObserver(update);
    obs.observe(el, { attributes: true, attributeFilter: ["class"] });
    return () => obs.disconnect();
  }, []);
  return dark;
}

/** Chart font: match the site (Geist), not Plotly's default. */
export const CHART_FONT = "'Geist', ui-sans-serif, system-ui, sans-serif";

/** Theme-aware Plotly colors (readable on both light and dark). */
export function chartColors(dark: boolean) {
  return dark
    ? { font: "#cbd5e1", grid: "rgba(148,163,184,0.20)", line: "#5aa2e0", danger: "#f1857a", accent2: "#a78bfa" }
    : { font: "#475569", grid: "rgba(100,116,139,0.16)", line: "#2b6cb0", danger: "#c0392b", accent2: "#6b46c1" };
}

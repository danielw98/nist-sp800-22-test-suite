"use client";

import Link from "next/link";

import { BarChart } from "@/components/BarChart";
import montecarlo from "@/data-static/figures/montecarlo_pvalue_uniformity.json";
import pvalueHist from "@/data-static/figures/pvalue_hist_datafiles.json";

function n1Centers() {
  const lo = montecarlo.n1Hist.lo;
  return montecarlo.n1Hist.counts.map((_, i) => String(lo + i));
}
function binCenters(n: number) {
  return Array.from({ length: n }, (_, i) => ((i + 0.5) / n).toFixed(3));
}

export default function FiguresPage() {
  return (
    <div className="space-y-8">
      <div>
        <h1 className="text-2xl font-bold">Figuri</h1>
        <p className="text-muted mt-1 max-w-3xl">
          Generate dintr-o singură sursă (<span className="font-mono">docs/make_figures.py</span>):
          PDF pentru referat, SVG și date JSON pentru web. Histogramele de mai jos sunt
          interactive (Plotly); celelalte sunt vectoriale (SVG), identice cu cele din referat.
        </p>
      </div>

      <Figure
        title="Distribuția Monte-Carlo a lui N1 sub H0"
        caption={`${montecarlo.trials.toLocaleString()} secvențe aleatoare de n=${montecarlo.n} (generator PCG64). sigma empiric = ${montecarlo.empSd}, sigma NIST = ${montecarlo.nistSd} (raport ${montecarlo.ratio}); Var(d) = ${montecarlo.varD}; rata de respingere = ${montecarlo.fracPlt01} (nominal 0.01). Varianța reală e ușor mai mare decât presupune formula NIST.`}
      >
        <BarChart
          x={n1Centers()}
          y={montecarlo.n1Hist.counts}
          xtitle="N1"
          ytitle="număr simulări"
          expected={undefined}
        />
      </Figure>

      <Figure
        title="Distribuția valorilor p sub H0"
        caption={`Aproape uniformă, cu un mic exces sub 0.01 (rata de respingere ${montecarlo.fracPlt01}).`}
      >
        <BarChart
          x={binCenters(montecarlo.pHist.bins)}
          y={montecarlo.pHist.counts}
          xtitle="p-value"
          ytitle="număr simulări"
          expected={montecarlo.trials / montecarlo.pHist.bins}
        />
      </Figure>

      <Figure
        title="Uniformitatea valorilor p: pi vs e"
        caption={`${pvalueHist.pi.streams} fluxuri x ${pvalueHist.pi.streamLength} biți fiecare. pi e uniform; e este la limită (vezi referatul).`}
      >
        <div className="grid sm:grid-cols-2 gap-4">
          <div>
            <div className="text-sm font-medium mb-1">pi</div>
            <BarChart x={binCenters(10)} y={pvalueHist.pi.counts} xtitle="p-value" ytitle="fluxuri" expected={pvalueHist.pi.streams / 10} height={240} />
          </div>
          <div>
            <div className="text-sm font-medium mb-1">e</div>
            <BarChart x={binCenters(10)} y={pvalueHist.e.counts} xtitle="p-value" ytitle="fluxuri" expected={pvalueHist.e.streams / 10} height={240} />
          </div>
        </div>
      </Figure>

      <Figure
        title="Spectru: aleator vs periodic"
        caption="Stânga: spectru plat (aleator). Dreapta: vârfuri înalte la armonice (periodic, perioada 8). Linia roșie e pragul de 95%."
      >
        <div className="rounded-lg bg-white p-3">
          {/* eslint-disable-next-line @next/next/no-img-element */}
          <img src="/figures/spectrum_random_vs_periodic.svg" alt="Spectru aleator vs periodic" className="w-full" />
        </div>
      </Figure>

      <Figure
        title="Exemplul NIST: 46 vs 48"
        caption="Pentru a obține valoarea documentată 46 ar fi necesar un prag cu ~5% mai mic decât cel corect; cele două vârfuri portocalii ar fi atunci excluse."
      >
        <div className="rounded-lg bg-white p-3">
          {/* eslint-disable-next-line @next/next/no-img-element */}
          <img src="/figures/example_threshold_46_vs_48.svg" alt="Exemplul NIST 46 vs 48" className="w-full" />
        </div>
        <p className="text-sm mt-2">
          <Link href="/controversy" className="text-accent underline">
            Versiunea interactivă (cu slider)
          </Link>
        </p>
      </Figure>
    </div>
  );
}

function Figure({
  title,
  caption,
  children,
}: {
  title: string;
  caption: string;
  children: React.ReactNode;
}) {
  return (
    <figure className="rounded-lg border border-border bg-surface p-4">
      <h2 className="font-semibold mb-2">{title}</h2>
      {children}
      <figcaption className="text-sm text-muted mt-2 leading-relaxed">{caption}</figcaption>
    </figure>
  );
}

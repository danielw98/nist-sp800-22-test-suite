import Link from "next/link";

import { TestGrid } from "@/components/TestGrid";
import { buttonClasses } from "@/components/ui";
import { TEST_REGISTRY } from "@/lib/tests";

const DFT_LINKS = [
  { href: "/tests/dft", label: "Rulează testul DFT" },
  { href: "/paper", label: "Referat" },
  { href: "/controversy", label: "Controversă 46 vs 48" },
  { href: "/figures", label: "Figuri" },
];

export default function Home() {
  return (
    <div className="space-y-12">
      <section className="relative overflow-hidden rounded-3xl border border-border bg-surface px-6 py-16 sm:px-12 sm:py-20 shadow-[var(--shadow)]">
        <div
          aria-hidden
          className="pointer-events-none absolute -top-24 -right-24 h-72 w-72 rounded-full opacity-20 blur-3xl"
          style={{ background: "radial-gradient(circle, var(--accent), transparent 70%)" }}
        />
        <div className="relative space-y-6">
          <span className="inline-block rounded-full bg-accent-weak px-3 py-1 text-xs font-medium text-accent">
            NIST SP 800-22 &middot; 15 teste implementate
          </span>
          <h1 className="text-4xl sm:text-5xl font-extrabold tracking-tight text-balance">
            Suită completă de teste de aleatorism NIST SP 800-22
          </h1>
          <p className="max-w-2xl text-lg text-muted text-pretty">
            Toate cele 15 teste statistice ale suitei, rulate pe binarul C++ real - din interfață
            sau prin API-ul public. Testul spectral (Transformată Fourier Discretă) este tratat în
            profunzime, cu referat, controverse și simulări Monte-Carlo.
          </p>
          <div className="flex flex-wrap gap-3 pt-1">
            <Link href="/tests" className={buttonClasses("primary", "lg")}>
              Explorează testele
            </Link>
            <Link href="/tests/dft" className={buttonClasses("secondary", "lg")}>
              Testul spectral (DFT)
            </Link>
            <Link href="/paper" className={buttonClasses("secondary", "lg")}>
              Citește referatul
            </Link>
          </div>
        </div>
      </section>

      <section className="space-y-4">
        <div className="flex items-end justify-between gap-3">
          <h2 className="text-2xl font-bold tracking-tight">Cele 15 teste</h2>
          <Link href="/tests" className="text-sm font-medium text-accent hover:underline">
            Toate testele -&gt;
          </Link>
        </div>
        <TestGrid tests={TEST_REGISTRY} />
      </section>

      <section className="rounded-3xl border border-accent/40 bg-accent-weak p-6 sm:p-8">
        <div className="max-w-3xl space-y-3">
          <span className="inline-block rounded-full bg-accent/15 px-3 py-1 text-xs font-medium text-accent">
            Tratat în profunzime
          </span>
          <h2 className="text-2xl font-bold tracking-tight">Testul spectral (DFT)</h2>
          <p className="text-sm leading-relaxed">
            Testul din secțiunea 2.6 mută secvența în domeniul frecvenței și numără câte vârfuri
            spectrale depășesc pragul de 95%. Este și cel mai contestat test din suită: documentația
            NIST raportează N1 = 46 pentru exemplul de 100 de biți, dar codul de referință și o
            transformată corectă dau N1 = 48. Aici îl tratăm în detaliu - matematica, implementarea,
            simulările Monte-Carlo și controversa.
          </p>
        </div>
        <div className="flex flex-wrap gap-2.5 pt-5">
          {DFT_LINKS.map((l, i) => (
            <Link key={l.href} href={l.href} className={buttonClasses(i === 0 ? "primary" : "secondary", "md")}>
              {l.label}
            </Link>
          ))}
        </div>
      </section>
    </div>
  );
}

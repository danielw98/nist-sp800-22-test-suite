import Link from "next/link";
import { notFound } from "next/navigation";

import { TestRunner } from "@/components/TestRunner";
import { getTest, nistSection, responseNote, TEST_REGISTRY, type TestDef } from "@/lib/tests";

export function generateStaticParams() {
  return TEST_REGISTRY.map((t) => ({ test: t.id }));
}

export async function generateMetadata({ params }: { params: Promise<{ test: string }> }) {
  const { test } = await params;
  const def = getTest(test);
  return { title: def ? `${def.name} - NIST SP 800-22` : "Test - NIST SP 800-22" };
}

function Field({ label, text }: { label: string; text: string }) {
  return (
    <div className="rounded-xl border border-border bg-surface p-4">
      <div className="text-xs text-muted uppercase tracking-wide">{label}</div>
      <div className="mt-1.5 text-sm leading-relaxed">{text}</div>
    </div>
  );
}

function curlSnippet(def: TestDef): string {
  return [
    `curl -s https://prng-nist-tests.student-dev.ro/api/run/${def.id} \\`,
    `  -H 'Content-Type: application/json' \\`,
    `  -d '{"bits":"1100100100001111110110101010001000100001011010001100001000110100110001001100011001100010100010111000"}'`,
  ].join("\n");
}

export default async function TestPage({ params }: { params: Promise<{ test: string }> }) {
  const { test } = await params;
  const def = getTest(test);
  if (!def) {
    notFound();
  }

  return (
    <div className="space-y-8">
      <header className="space-y-2">
        <div className="text-sm font-mono text-muted">
          #{def.number} &middot; NIST &sect;{nistSection(def)} &middot; {def.category}
        </div>
        <h1 className="text-3xl font-bold tracking-tight">{def.name}</h1>
        <p className="text-lg text-muted max-w-3xl">{def.purpose}</p>
      </header>

      <section className="grid gap-3 sm:grid-cols-2">
        <Field label="Intrare" text={def.input} />
        <Field label="Statistică" text={def.statistic} />
        <Field label="Decizie" text={def.decision} />
        <Field label="Minim recomandat" text={`${def.minBits.toLocaleString()} biți`} />
      </section>

      {def.id === "dft" && (
        <div className="rounded-xl border border-accent/40 bg-accent-weak p-4 text-sm leading-relaxed">
          Testul spectral este tratat în profunzime: <Link href="/paper" className="underline font-medium">referatul</Link>,{" "}
          <Link href="/controversy" className="underline font-medium">controversă 46 vs 48</Link> și{" "}
          <Link href="/figures" className="underline font-medium">figurile</Link>.
        </div>
      )}

      {def.status === "implemented" ? (
        <section className="space-y-3">
          <h2 className="text-sm font-semibold uppercase tracking-wide text-muted">Rulează</h2>
          <TestRunner def={def} />
        </section>
      ) : (
        <div className="rounded-xl border border-border bg-surface-2 px-5 py-4 text-muted leading-relaxed">
          Acest test este <strong className="text-foreground">planificat</strong>: documentat aici,
          dar încă neimplementat în motor. Contractul API este deja definit
          (<span className="font-mono">POST /api/run/{def.id}</span>) și întoarce
          {" "}<span className="font-mono">501 not_implemented</span> până când este adăugat.
        </div>
      )}

      <section className="space-y-2">
        <h2 className="text-sm font-semibold uppercase tracking-wide text-muted">API</h2>
        <p className="text-sm text-muted">
          <span className="font-mono">POST /api/run/{def.id}</span> - {responseNote(def)} Contractul
          complet: pagina <Link href="/api-docs" className="underline">API</Link>.
        </p>
        <pre className="rounded-lg bg-[#0d1117] text-[#e6edf3] p-3 overflow-x-auto text-xs leading-relaxed">
          {curlSnippet(def)}
        </pre>
      </section>
    </div>
  );
}

import Link from "next/link";

import { nistSection, responseNote, TEST_REGISTRY } from "@/lib/tests";

export const metadata = { title: "API public - NIST SP 800-22" };

function Code({ children }: { children: React.ReactNode }) {
  return (
    <pre className="rounded-lg bg-[#0d1117] text-[#e6edf3] p-3 overflow-x-auto text-xs leading-relaxed my-2">
      {children}
    </pre>
  );
}

function Endpoint({
  method,
  path,
  children,
}: {
  method: string;
  path: string;
  children: React.ReactNode;
}) {
  return (
    <div className="rounded-lg border border-border bg-surface p-4 space-y-2">
      <div className="flex items-center gap-2">
        <span className="rounded bg-accent text-white text-xs font-bold px-2 py-1">{method}</span>
        <span className="font-mono text-sm">{path}</span>
      </div>
      {children}
    </div>
  );
}

export default function ApiDocsPage() {
  return (
    <div className="space-y-6 max-w-5xl mx-auto">
      <div>
        <h1 className="text-2xl font-bold">API public</h1>
        <p className="text-muted mt-1 max-w-3xl">
          API JSON pentru suita NIST SP 800-22. Toate cele 15 teste folosesc același
          contract, testul fiind selectat prin slug-ul din cale. Adresa de bază:{" "}
          <span className="font-mono">https://prng-nist-tests.student-dev.ro/api</span> (local:{" "}
          <span className="font-mono">http://localhost:3000/api</span>).
        </p>
      </div>

      <section className="rounded-lg border border-accent bg-accent-weak p-4 text-sm">
        <h2 className="font-semibold text-accent">Limite de trafic</h2>
        <p className="mt-1">
          Limitele sunt permisive, dimensionate astfel încât să permită utilizarea
          programatică și să prevină abuzul. Contorizarea se realizează pe o fereastră
          glisantă de 30 de zile.
        </p>
        <ul className="list-disc pl-5 mt-2 space-y-0.5">
          <li>maximum 120 de cereri pe minut per adresă IP (sunt admise vârfuri de scurtă durată);</li>
          <li>maximum 5 GiB de trafic la 30 de zile per adresă IP (cerere și răspuns cumulate);</li>
          <li>plafon global de 1 TiB la 30 de zile;</li>
          <li>prag suplimentar de aproximativ 100000 de cereri la 30 de zile per adresă IP;</li>
          <li>
            adresele incluse în <span className="font-mono">RL_ALLOWLIST</span>, precum și cele
            locale, sunt exceptate integral.
          </li>
        </ul>
        <p className="mt-2">
          Fiecare răspuns include antetele <span className="font-mono">X-RateLimit-Limit</span>,{" "}
          <span className="font-mono">X-RateLimit-Remaining</span> și{" "}
          <span className="font-mono">X-RateLimit-Reset</span> (cereri), respectiv{" "}
          <span className="font-mono">X-RateLimit-Bytes-Limit</span> și{" "}
          <span className="font-mono">X-RateLimit-Bytes-Remaining</span> (trafic). La depășirea unei
          limite se returnează codul <span className="font-mono">429</span>, însoțit de{" "}
          <span className="font-mono">Retry-After</span>. Configurația în vigoare poate fi
          consultată la <span className="font-mono">GET /api/health</span>.
        </p>
      </section>

      <Endpoint method="POST" path="/api/run/{test}">
        <p className="text-sm text-muted">
          Rulează un test pe o secvență de biți. Toate cele 15 teste sunt implementate; slug-urile
          sunt în tabelul de mai jos.
        </p>
        <p className="text-sm font-medium">Cerere</p>
        <Code>{`{
  "bits": "1100100100...",   // doar 0/1 sunt păstrate; max 8.000.000 biți
  "alpha": 0.01,             // 0.0001 .. 0.5 (implicit 0.01)
  "block": 10,               // M/m pentru testele pe blocuri (optional)
  "method": "auto"           // "auto" | "fft" | "direct" (doar dft)
}`}</Code>
        <p className="text-sm font-medium">Răspuns 200 (plic uniform)</p>
        <Code>{`{
  "test": "monobit", "n": 100,
  "p_value": 0.109599, "passed": true,
  "statistic": 1.6, "critical": 2.575829, "distribution": "half-normal",
  "stats": { "S_n": -16, "s_obs": 1.6 }
}`}</Code>
        <p className="text-sm font-medium">Exemplu</p>
        <Code>{`curl -s https://prng-nist-tests.student-dev.ro/api/run/monobit \\
  -H 'Content-Type: application/json' \\
  -d '{"bits":"1100100100..."}'`}</Code>
        <p className="text-xs text-muted">
          Verdictul este comparația statisticii cu valoarea critică (p rămâne secundar). Câmpurile
          suplimentare per test sunt în tabel. Coduri: 400 (cerere invalidă / biți), 404 (test
          necunoscut), 429 (limită), 500/504.
        </p>
      </Endpoint>

      <section className="rounded-lg border border-border overflow-x-auto">
        <table className="w-full text-sm">
          <thead className="bg-surface-2 text-left">
            <tr>
              <th className="px-3 py-2">Slug</th>
              <th className="px-3 py-2">Secțiune</th>
              <th className="px-3 py-2">Câmp suplimentar în răspuns</th>
            </tr>
          </thead>
          <tbody>
            {TEST_REGISTRY.map((def) => (
              <tr key={def.id} className="border-t border-border align-top">
                <td className="px-3 py-2">
                  <Link href={`/tests/${def.id}`} className="font-mono text-accent underline">
                    {def.id}
                  </Link>
                </td>
                <td className="px-3 py-2 font-mono whitespace-nowrap">&sect;{nistSection(def)}</td>
                <td className="px-3 py-2 text-muted">{responseNote(def)}</td>
              </tr>
            ))}
          </tbody>
        </table>
      </section>

      <Endpoint method="POST" path="/api/assess/{test}">
        <p className="text-sm text-muted">
          Analiză pe multe fluxuri (proporție + uniformitate) peste fișierele NIST, pentru
          testul ales.
        </p>
        <Code>{`{
  "files": ["data.pi", "data.e"],  // doar fișierele din lista permisă
  "length": 10000,                 // biți / flux (1000 .. 1.000.000)
  "maxStreams": 0,                 // 0 = toate
  "alpha": 0.01, "block": 10       // block optional
}`}</Code>
      </Endpoint>

      <Endpoint method="GET" path="/api/sequence?file=&offset=&len=">
        <p className="text-sm text-muted">
          O felie mărginită dintr-un fișier NIST (max 65536 biți). Doar fișiere din lista
          permisă: data.e, data.pi, data.sqrt2, data.sqrt3, data.sha1, bits_nist_example.txt.
        </p>
        <Code>{`curl -s 'https://prng-nist-tests.student-dev.ro/api/sequence?file=data.pi&offset=0&len=256'`}</Code>
      </Endpoint>

      <Endpoint method="GET" path="/api/health">
        <p className="text-sm text-muted">
          Stare + capabilități: binare prezente, configurația limitelor de trafic și catalogul de
          teste (toate cele 15, cu starea fiecăruia). Fără limită de rată.
        </p>
      </Endpoint>
    </div>
  );
}

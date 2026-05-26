import { TestGrid } from "@/components/TestGrid";
import { TEST_REGISTRY } from "@/lib/tests";

export const metadata = { title: "Suită NIST SP 800-22 - toate testele" };

export default function TestsCatalog() {
  return (
    <div className="space-y-8">
      <header className="space-y-2">
        <h1 className="text-3xl font-bold tracking-tight">Suita NIST SP 800-22</h1>
        <p className="text-lg text-muted max-w-3xl">
          Cele 15 teste statistice de aleatorism, toate implementate. Fiecare se rulează pe o
          secvență lipită sau prin API, cu același contract. Testul spectral (DFT) este cel tratat
          în profunzime - cu referat, controverse și simulări.
        </p>
      </header>
      <TestGrid tests={TEST_REGISTRY} />
    </div>
  );
}

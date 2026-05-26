import Link from "next/link";

import { buttonClasses } from "@/components/ui";

const CHAPTERS = [
  { href: "/paper/introducere", n: 1, title: "Introducere și motivație" },
  { href: "/paper/matematica", n: 2, title: "Partea matematică" },
  { href: "/paper/implementare", n: 3, title: "Modul de implementare" },
  { href: "/paper/simulari", n: 4, title: "Simulări și rezultate" },
  { href: "/paper/discutie", n: 5, title: "Avantaje, limitări, controverse" },
];

export default function PaperPage() {
  return (
    <div>
      <h1 className="text-2xl font-bold">Referatul</h1>
      <p>
        Lucrarea completă despre testul spectral (NIST SP 800-22, secțiunea 2.6):
        teoria, implementarea, simulările și controversele. Citește-o aici, capitol cu
        capitol (cu formule și grafice), sau deschide PDF-ul compilat cu XeLaTeX.
      </p>

      <p>
        <a
          href="/Testul-Spectral-DFT-NIST-SP-800-22.pdf"
          target="_blank"
          rel="noopener noreferrer"
          className={buttonClasses("primary", "lg")}
        >
          Deschide PDF-ul complet
        </a>
      </p>

      <h2>Capitole</h2>
      <ol>
        {CHAPTERS.map((c) => (
          <li key={c.href}>
            <Link href={c.href}>{c.title}</Link>
          </li>
        ))}
      </ol>
    </div>
  );
}

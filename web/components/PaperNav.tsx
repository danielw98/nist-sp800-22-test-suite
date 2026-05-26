"use client";

import Link from "next/link";
import { usePathname } from "next/navigation";

const ITEMS = [
  { href: "/paper", label: "Prezentare + PDF" },
  { href: "/paper/introducere", label: "1. Introducere" },
  { href: "/paper/matematica", label: "2. Partea matematică" },
  { href: "/paper/implementare", label: "3. Implementare" },
  { href: "/paper/simulari", label: "4. Simulări" },
  { href: "/paper/discutie", label: "5. Controverse" },
];

/** Sidebar navigation for the referat chapters. */
export function PaperNav() {
  const path = usePathname();
  return (
    <nav className="sticky top-16 text-sm space-y-1">
      {ITEMS.map((it) => {
        const active = path === it.href;
        return (
          <Link
            key={it.href}
            href={it.href}
            className={`block rounded px-3 py-1.5 ${
              active ? "bg-accent-weak text-accent font-medium" : "text-muted hover:text-foreground"
            }`}
          >
            {it.label}
          </Link>
        );
      })}
    </nav>
  );
}

"use client";

import Link from "next/link";
import { usePathname } from "next/navigation";

import { groupTestsByCategory } from "@/lib/tests";
import { NavDropdown, type DropdownGroup } from "./NavDropdown";
import { ThemeToggle } from "./ThemeToggle";

// All 15 tests, grouped by category, for the "Teste" dropdown.
const TEST_GROUPS: DropdownGroup[] = groupTestsByCategory().map((g) => ({
  heading: g.category,
  items: g.tests.map((t) => ({ href: `/tests/${t.id}`, label: t.name })),
}));

// The DFT-specific deep-dive pages, grouped under one menu.
const DFT_GROUPS: DropdownGroup[] = [
  {
    items: [
      { href: "/paper", label: "Referat" },
      { href: "/controversy", label: "Controversă 46 vs 48" },
      { href: "/figures", label: "Figuri" },
      { href: "/sequences", label: "Secvențe (spectral)" },
      { href: "/validation", label: "Validare (spectral)" },
    ],
  },
];

const FLAT = [
  { href: "/source", label: "Cod" },
  { href: "/api-docs", label: "API" },
  { href: "/references", label: "Referințe" },
];

/** Sticky top navigation: brand, the Teste + DFT dropdowns, flat links, theme toggle. */
export function Nav() {
  const path = usePathname();
  return (
    <header className="sticky top-0 z-20 border-b border-border bg-surface/85 backdrop-blur supports-[backdrop-filter]:bg-surface/70">
      <nav className="max-w-[88rem] mx-auto px-4 sm:px-6 lg:px-10 h-16 flex items-center gap-2">
        <Link href="/" className="flex items-center gap-2.5 shrink-0 mr-1">
          <span className="grid place-items-center h-9 w-9 rounded-xl bg-accent text-accent-fg shadow-[var(--shadow)]">
            <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.2" strokeLinecap="round">
              <path d="M2 12 Q 6.5 3 11 12 T 22 12" />
            </svg>
          </span>
          <span className="font-bold text-lg tracking-tight leading-none">
            SP 800-22 <span className="text-muted font-normal">/ 15 teste</span>
          </span>
        </Link>
        <div className="flex items-center gap-1">
          <NavDropdown label="Teste" groups={TEST_GROUPS} seeAll={{ href: "/tests", label: "Toate testele ->" }} />
          <NavDropdown label="DFT in detaliu" groups={DFT_GROUPS} />
          {FLAT.map((l) => {
            const active = path === l.href || path.startsWith(l.href + "/");
            return (
              <Link
                key={l.href}
                href={l.href}
                className={`px-3 py-2 rounded-lg text-sm font-medium whitespace-nowrap transition-colors ${
                  active ? "bg-accent-weak text-accent" : "text-muted hover:text-foreground hover:bg-surface-2"
                }`}
              >
                {l.label}
              </Link>
            );
          })}
        </div>
        <div className="ml-auto shrink-0">
          <ThemeToggle />
        </div>
      </nav>
    </header>
  );
}

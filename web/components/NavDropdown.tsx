"use client";

import Link from "next/link";
import { usePathname } from "next/navigation";
import { useEffect, useRef, useState } from "react";

export interface DropdownItem {
  href: string;
  label: string;
}
export interface DropdownGroup {
  heading?: string;
  items: DropdownItem[];
}

const isActive = (path: string, href: string) => path === href || path.startsWith(href + "/");

/** A nav menu button that opens a grouped link panel. Closes on outside click,
 *  Escape, or route change. The panel is absolutely positioned, so its parent
 *  in the nav must not be an overflow-clipping container. */
export function NavDropdown({
  label,
  groups,
  seeAll,
}: {
  label: string;
  groups: DropdownGroup[];
  seeAll?: DropdownItem;
}) {
  const path = usePathname();
  const [open, setOpen] = useState(false);
  const ref = useRef<HTMLDivElement>(null);

  const active =
    (seeAll ? isActive(path, seeAll.href) : false) ||
    groups.some((g) => g.items.some((i) => isActive(path, i.href)));

  useEffect(() => setOpen(false), [path]);

  useEffect(() => {
    if (!open) return;
    const onDoc = (e: MouseEvent) => {
      if (ref.current && !ref.current.contains(e.target as Node)) setOpen(false);
    };
    const onKey = (e: KeyboardEvent) => {
      if (e.key === "Escape") setOpen(false);
    };
    document.addEventListener("mousedown", onDoc);
    document.addEventListener("keydown", onKey);
    return () => {
      document.removeEventListener("mousedown", onDoc);
      document.removeEventListener("keydown", onKey);
    };
  }, [open]);

  return (
    <div ref={ref} className="relative">
      <button
        type="button"
        aria-haspopup="true"
        aria-expanded={open}
        onClick={() => setOpen((v) => !v)}
        className={`px-3 py-2 rounded-lg text-sm font-medium whitespace-nowrap inline-flex items-center gap-1 transition-colors ${
          active ? "bg-accent-weak text-accent" : "text-muted hover:text-foreground hover:bg-surface-2"
        }`}
      >
        {label}
        <svg
          width="12"
          height="12"
          viewBox="0 0 24 24"
          fill="none"
          stroke="currentColor"
          strokeWidth="2.5"
          strokeLinecap="round"
          strokeLinejoin="round"
          className={`transition-transform ${open ? "rotate-180" : ""}`}
        >
          <path d="m6 9 6 6 6-6" />
        </svg>
      </button>
      {open && (
        <div className="absolute top-full left-0 mt-1.5 z-30 w-72 max-h-[72vh] overflow-y-auto rounded-xl border border-border bg-surface p-2 shadow-[var(--shadow)]">
          {seeAll && (
            <Link
              href={seeAll.href}
              className="block rounded-lg px-3 py-2 text-sm font-medium text-accent hover:bg-accent-weak"
            >
              {seeAll.label}
            </Link>
          )}
          {groups.map((g, gi) => (
            <div key={gi}>
              {g.heading && (
                <div className="px-3 pt-2.5 pb-1 text-xs uppercase tracking-wide text-muted">
                  {g.heading}
                </div>
              )}
              {g.items.map((i) => (
                <Link
                  key={i.href}
                  href={i.href}
                  className={`block rounded-lg px-3 py-1.5 text-sm transition-colors ${
                    isActive(path, i.href)
                      ? "bg-accent-weak text-accent"
                      : "text-foreground hover:bg-surface-2"
                  }`}
                >
                  {i.label}
                </Link>
              ))}
            </div>
          ))}
        </div>
      )}
    </div>
  );
}

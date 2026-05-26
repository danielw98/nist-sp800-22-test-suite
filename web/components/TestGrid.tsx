import Link from "next/link";

import { type TestDef, type TestStatus } from "@/lib/tests";

function Badge({ status }: { status: TestStatus }) {
  return status === "implemented" ? (
    <span className="rounded-full bg-ok-weak text-ok text-xs px-2 py-0.5 font-medium">rulabil</span>
  ) : (
    <span className="rounded-full bg-surface-2 text-muted text-xs px-2 py-0.5 font-medium">
      planificat
    </span>
  );
}

/** The 15-test catalog grid, shared by the home page and /tests. */
export function TestGrid({ tests }: { tests: TestDef[] }) {
  return (
    <section className="grid gap-4 sm:grid-cols-2 lg:grid-cols-3">
      {tests.map((t) => (
        <Link
          key={t.id}
          href={`/tests/${t.id}`}
          className="group rounded-2xl border border-border bg-surface p-5 transition-all hover:border-accent hover:shadow-[var(--shadow)] hover:-translate-y-0.5"
        >
          <div className="flex items-center justify-between gap-2">
            <span className="text-xs font-mono text-muted">
              #{t.number} &middot; {t.category}
            </span>
            <Badge status={t.status} />
          </div>
          <div className="font-semibold mt-2 group-hover:text-accent transition-colors">{t.name}</div>
          <div className="text-sm text-muted mt-1.5 leading-relaxed">{t.purpose}</div>
        </Link>
      ))}
    </section>
  );
}

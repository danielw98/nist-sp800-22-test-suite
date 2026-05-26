"use client";

import { useMemo, useState } from "react";

import indexRaw from "@/data-static/source-index.json";

type SrcFile = { path: string; lang: string; lines: number; html: string };
const files = (indexRaw as { files: SrcFile[] }).files;

const GROUP_ORDER = ["src", "tests", "docs", "root"];
const GROUP_LABEL: Record<string, string> = {
  src: "Biblioteca (src)",
  tests: "Teste (tests)",
  docs: "Docs",
  root: "Build",
};

function groupOf(path: string): string {
  const i = path.indexOf("/");
  return i === -1 ? "root" : path.slice(0, i);
}
function baseName(path: string): string {
  return path.split("/").pop() ?? path;
}

export default function SourcePage() {
  const groups = useMemo(() => {
    const map = new Map<string, SrcFile[]>();
    for (const f of files) {
      const g = groupOf(f.path);
      (map.get(g) ?? map.set(g, []).get(g)!).push(f);
    }
    return GROUP_ORDER.filter((g) => map.has(g)).map((g) => ({ group: g, items: map.get(g)! }));
  }, []);

  const [sel, setSel] = useState(
    files.find((f) => f.path.endsWith("spectral_test.cpp"))?.path ?? files[0]?.path,
  );
  const current = files.find((f) => f.path === sel);

  return (
    <div className="space-y-5">
      <div>
        <h1 className="text-2xl font-bold">Cod sursă</h1>
        <p className="text-muted mt-1">
          Biblioteca C++, suita de teste, build-ul și scriptul de figuri - evidențiate
          sintactic.
        </p>
      </div>

      <div className="grid gap-5 md:grid-cols-[15rem_1fr]">
        <aside className="rounded-xl border border-border bg-surface p-2 h-fit max-h-[72vh] overflow-y-auto">
          {groups.map(({ group, items }) => (
            <div key={group} className="mb-2">
              <div className="px-2 pt-2 pb-1 text-[0.7rem] font-semibold uppercase tracking-wide text-muted">
                {GROUP_LABEL[group] ?? group}
              </div>
              <ul>
                {items.map((f) => (
                  <li key={f.path}>
                    <button
                      onClick={() => setSel(f.path)}
                      className={`w-full text-left px-2 py-1.5 rounded-md font-mono text-xs transition-colors ${
                        sel === f.path
                          ? "bg-accent-weak text-accent"
                          : "text-muted hover:text-foreground hover:bg-surface-2"
                      }`}
                    >
                      {baseName(f.path)}
                    </button>
                  </li>
                ))}
              </ul>
            </div>
          ))}
        </aside>

        <div className="min-w-0 rounded-xl border border-border overflow-hidden">
          {current && (
            <>
              <div className="flex items-center justify-between gap-3 border-b border-border bg-surface px-4 py-2.5">
                <span className="font-mono text-sm truncate">{current.path}</span>
                <span className="text-xs text-muted whitespace-nowrap">
                  {current.lines} linii &middot; {current.lang}
                </span>
              </div>
              <div
                className="code-viewer overflow-x-auto max-h-[72vh] overflow-y-auto"
                dangerouslySetInnerHTML={{ __html: current.html }}
              />
            </>
          )}
        </div>
      </div>
    </div>
  );
}

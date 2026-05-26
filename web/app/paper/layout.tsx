import { PaperNav } from "@/components/PaperNav";

export default function PaperLayout({ children }: { children: React.ReactNode }) {
  return (
    <div className="grid gap-8 md:grid-cols-[13rem_1fr] max-w-6xl mx-auto">
      <aside>
        <PaperNav />
      </aside>
      <article className="prose-paper min-w-0">{children}</article>
    </div>
  );
}

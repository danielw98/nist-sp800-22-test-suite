import type { Metadata } from "next";
import { Geist, Geist_Mono } from "next/font/google";
import "./globals.css";
import "katex/dist/katex.min.css";
import { Nav } from "@/components/Nav";

const sans = Geist({
  subsets: ["latin", "latin-ext"],
  variable: "--font-sans-loaded",
  display: "swap",
});
const mono = Geist_Mono({
  subsets: ["latin"],
  variable: "--font-mono-loaded",
  display: "swap",
});

export const metadata: Metadata = {
  title: "NIST SP 800-22 - Suită completă de teste de aleatorism",
  description:
    "Toate cele 15 teste de aleatorism din suita NIST SP 800-22, rulate pe binarul C++ real. Testul spectral (Transformată Fourier Discretă) este tratat în profunzime.",
};

// Set the theme class before paint to avoid a flash of the wrong theme.
const themeScript = `(function(){try{if(localStorage.getItem('theme')==='dark'){document.documentElement.classList.add('dark');}}catch(e){}})();`;

export default function RootLayout({ children }: { children: React.ReactNode }) {
  return (
    <html
      lang="ro"
      suppressHydrationWarning
      className={`${sans.variable} ${mono.variable} h-full antialiased`}
    >
      <body className="min-h-full flex flex-col">
        <script dangerouslySetInnerHTML={{ __html: themeScript }} />
        <Nav />
        <main className="flex-1 w-full max-w-[88rem] mx-auto px-4 sm:px-6 lg:px-10 py-8 sm:py-10">
          {children}
        </main>
        <footer className="border-t border-border text-sm text-muted">
          <div className="max-w-[88rem] mx-auto px-4 sm:px-6 lg:px-10 py-6 flex flex-wrap gap-x-6 gap-y-2 justify-between">
            <span>Testul Spectral DFT - NIST SP 800-22, secțiunea 2.6</span>
            <span className="font-mono">prng-nist-tests.student-dev.ro</span>
          </div>
        </footer>
      </body>
    </html>
  );
}

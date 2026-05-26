import { buttonClasses } from "@/components/ui";

const REFS: { n: number; cite: string; href: string }[] = [
  {
    n: 1,
    cite: "L. E. Bassham et al., A Statistical Test Suite for Random and Pseudorandom Number Generators for Cryptographic Applications, NIST SP 800-22 Rev 1a, 2010.",
    href: "https://doi.org/10.6028/NIST.SP.800-22r1a",
  },
  {
    n: 2,
    cite: "L. C. Noll, T. Gilgan, R. Paccagnella, sts - NIST Statistical Test Suite (versiune refactorizată, GitHub).",
    href: "https://github.com/arcetri/sts",
  },
  {
    n: 3,
    cite: "S.-J. Kim, K. Umeno, A. Hasegawa, Corrections of the NIST Statistical Test Suite for Randomness, 2004.",
    href: "https://arxiv.org/abs/nlin/0401040",
  },
  {
    n: 4,
    cite: "K. Hamano, The Distribution of the Spectrum for the DFT Test Included in SP800-22, IEICE Trans. Fundamentals E88-A(1):67-73, 2005.",
    href: "https://doi.org/10.1093/ietfec/e88-a.1.67",
  },
  {
    n: 5,
    cite: "F. Pareschi, R. Rovatti, G. Setti, On Statistical Tests for Randomness Included in the NIST SP800-22 Test Suite..., IEEE TIFS 7(2):491-505, 2012.",
    href: "https://doi.org/10.1109/TIFS.2012.2185227",
  },
  {
    n: 6,
    cite: "A. Iwasaki, Deriving the Variance of the DFT Test Using Parseval's Theorem, 2018.",
    href: "https://arxiv.org/abs/1806.10357",
  },
  {
    n: 7,
    cite: "A. Iwasaki, K. Umeno, A new randomness test solving problems of the DFT Test, 2017.",
    href: "https://arxiv.org/abs/1708.08218",
  },
  {
    n: 8,
    cite: "S. Zhu, Y. Ma, J. Lin, J. Zhuang, J. Jing, More Powerful and Reliable Second-Level Statistical Randomness Tests for NIST SP 800-22, ASIACRYPT 2016.",
    href: "https://eprint.iacr.org/2016/863",
  },
  {
    n: 9,
    cite: "D. Chen, H. Chen, L. Fan, K. Luo, Error Analysis of NIST SP 800-22 Test Suite, IEEE TIFS vol. 18, 2023.",
    href: "https://doi.org/10.1109/TIFS.2023.3287391",
  },
  {
    n: 10,
    cite: "L. I. Bluestein, A linear filtering approach to the computation of the DFT, IEEE Trans. Audio Electroacoust. 18(4):451-455, 1970.",
    href: "https://doi.org/10.1109/TAU.1970.1162132",
  },
];

const LINKS = [
  { label: "NIST CSRC - software + documentație", href: "https://csrc.nist.gov/projects/random-bit-generation/documentation-and-software" },
  { label: "arcetri/sts pe GitHub", href: "https://github.com/arcetri/sts" },
];

export default function ReferencesPage() {
  return (
    <div className="space-y-6">
      <div>
        <h1 className="text-3xl font-bold tracking-tight">Referințe</h1>
        <p className="text-muted mt-2 text-lg">
          Sursele folosite în referat și în implementare.
        </p>
      </div>

      <div className="flex flex-wrap gap-3">
        {LINKS.map((l) => (
          <a
            key={l.href}
            href={l.href}
            target="_blank"
            rel="noopener noreferrer"
            className={buttonClasses("secondary", "md")}
          >
            {l.label}
          </a>
        ))}
      </div>

      <ol className="space-y-3">
        {REFS.map((r) => (
          <li key={r.n} className="flex gap-3 text-sm">
            <span className="font-mono text-muted">[{r.n}]</span>
            <span>
              {r.cite}{" "}
              <a href={r.href} target="_blank" rel="noopener noreferrer" className="text-accent underline break-all">
                {r.href}
              </a>
            </span>
          </li>
        ))}
      </ol>
    </div>
  );
}

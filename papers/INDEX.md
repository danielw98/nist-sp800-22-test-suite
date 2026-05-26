# papers/ - reference sources for the referat

Open-access copies of the sources cited in the referat bibliography (and a couple
of supporting ones). The bib keys below match the referat's bibliography. Paywalled
sources cannot be redistributed here; they are listed at the bottom with their DOIs.

**Verification (2026-05-23):** all 10 references in the referat bibliography were
cross-checked - author names, exact title, venue, year and DOI - against the
local PDFs (where present) and the authoritative source pages (arXiv, IEEE
Xplore, IEICE, IACR ePrint, NIST/CSRC). Every DOI/URL resolves; no dead links,
no fabricated entries. The 6 open-access PDFs below were title-page verified;
the 4 paywalled entries were metadata-verified (full text behind IEEE/IEICE
paywalls - drop an institutional copy here if you want it locally).

## Downloaded (open access)

| File | Bib key | Citation | Backs |
|------|---------|----------|-------|
| `nist-sp800-22r1a.pdf` | `nist` [1] | NIST SP 800-22 Rev 1a (Bassham et al., 2010) | the standard itself; the sec 2.6.8 worked example we dispute (N1=46, p=0.168669) |
| `kim-umeno-hasegawa-2004-corrections.pdf` | `kim` [3] | Kim, Umeno, Hasegawa, *Corrections of the NIST Statistical Test Suite for Randomness*, arXiv:nlin/0401040 / IACR ePrint 2004/018 | **states the DFT test settings are wrong**; corrects threshold sqrt(3n) -> sqrt(ln(20)n) and the variance |
| `iwasaki-umeno-2017-exact-reference-distribution.pdf` | (support) | Iwasaki, Umeno, *Randomness Evaluation with the DFT Test Based on Exact Analysis of the Reference Distribution*, arXiv:1701.01960 | the reference distribution was never derived exactly |
| `iwasaki-umeno-2017-new-dft-test.pdf` | `iwasakiumeno` [7] | Iwasaki, Umeno, *A new randomness test solving problems of the DFT test*, arXiv:1708.08218 | DFT test is among the weakest; proposes a corrected test |
| `iwasaki-2018-variance-parseval.pdf` | `iwasaki` [6] | A. Iwasaki, *Deriving the Variance of the DFT Test Using Parseval's Theorem*, arXiv:1806.10357 | the n/4 variance is only approximate |
| `zhu-2016-second-level.pdf` | `secondlevel` [8] | Zhu et al., *More Powerful and Reliable Second-Level... Tests for NIST SP 800-22*, IACR ePrint 2016/863 (ASIACRYPT 2016) | the second-level (proportion/uniformity) tests are weak |

## Paywalled (not downloadable; cited by DOI)

- `hamano` [4] - K. Hamano, IEICE Trans. Fundamentals E88-A(1):67-73, 2005.
- `pareschi` [5] - Pareschi, Rovatti, Setti, IEEE TIFS 7(2):491-505, 2012, DOI 10.1109/TIFS.2012.2185227 (the c ~ 3.8 variance constant; Var(d) ~ 4/3.8 ~ 1.05).
- `erroranalysis` [9] - Chen, Chen, Fan, Luo, *Error Analysis of NIST SP 800-22*, IEEE TIFS, 2023, DOI 10.1109/TIFS.2023.3287391.
- `bluestein` [10] - L. I. Bluestein, IEEE Trans. Audio Electroacoust. 18(4):451-455, 1970, DOI 10.1109/TAU.1970.1162132.

`arcetri` [2] is the maintained NIST sts fork (code, not a paper); the clone is in
`reference/sts/` and its reference DFT driver is what we cross-checked against.

# CLAIMS - what each reference actually says, with the exact quote

For every non-trivial claim the referat attributes to a source, this maps:
**claim -> where we say it -> source -> exact quote + location -> how verified.**

Verification levels:
- **verbatim (local)** - quote extracted from a PDF in `papers/` (or `reference/`); re-checkable with `pdftotext <file> - | grep`.
- **corroborated (local)** - the source is paywalled, but an open-access paper we *do* hold cites it for exactly this point; that citing quote is given.
- **abstract** - only the publisher/Semantic-Scholar abstract was read (full text paywalled).

---

## 1. The NIST document prints N1 = 46, p = 0.168669 for the 100-bit example
- **Where:** ch.2 (Exemplu numeric), ch.5 (Controversa 1), home, `/controversy`.
- **Source:** `nist-sp800-22r1a.pdf`, sec. 2.6.8 - **verbatim (local)**.
- **Quote:** "(input) eps = 1100100100001111110110101010001000100001011010001100001000110100110001001100011001100010100010111000 ... n = 100 ... (processing) N1 = 46 ... N0 = 47.5 ... d = -1.376494 ... (output) P-value = 0.168669".

## 2. The reference code / a correct DFT gives N1 = 48 (document contradicts its code)
- **Where:** ch.5 (Controversa 1), home, `/controversy`.
- **Source:** `reference/sts/` (the maintained NIST suite) + our own FFT + numpy - **verbatim (local)**: we run all three and get 48 (see `tests/validate.py`, `dft_tests`). No reasonable component convention reproduces 46 (47-49).

## 3. Kim et al. corrected the threshold from sqrt(3n) to sqrt(ln(20) n)
- **Where:** ch.2 (Pragul), ch.5 (Controversa 2).
- **Source:** `kim-umeno-hasegawa-2004-corrections.pdf`, sec. 3 - **verbatim (local)**.
- **Quotes:** "Compute T = sqrt(3n) = the 95% peak height threshold value." ; "we can get the value ZC = -ln(0.05) = 2.995732274 ... we conclude that T = sqrt(2.995732274 n)." ; "We have found that the deviation of sqrt(3n) from sqrt(2.995732274 n) makes the distribution invalid." (ln 20 = -ln 0.05 = 2.995732274.)

## 4. The reference distribution was estimated numerically, not derived (circular reasoning)
- **Where:** ch.5 (Controversa 2).
- **Source:** `kim...pdf` (abstract: "the test setting of Discrete Fourier Transform test ... are wrong") + `iwasaki-umeno-2017-exact-reference-distribution.pdf` - **verbatim (local)**.
- **Quote (Iwasaki-Umeno):** "Kim et al. numerically estimated the distribution of the test statistic with pseudo-random numbers ... the distribution of the test statistic has still not been derived theoretically but rather numerically estimated."

## 5. Hamano studied/adjusted the spectral distribution but could not derive it
- **Where:** ch.5 (Controversa 2 + 3).
- **Source:** Hamano (paywalled, IEICE) - **corroborated (local)** by two papers we hold:
  - `iwasaki-umeno-2017-exact...pdf`: "In 2005, Hamano theoretically scrutinized the distribution of the Fourier coefficients in the original DFT test. However, he could not derive the theoretical distribution of the test statistic, but he did make the problems in the DFT test clearer."
  - `zhu-2016-second-level.pdf`: "Hamano adjusted the distribution parameters for the Spectral Test and corrected the Overlapping Test."

## 6. Pareschi et al.: variance with c approx 3.8 (not 4) fits better -> Var(d) approx 4/3.8 approx 1.05
- **Where:** ch.4 (Simulare Monte-Carlo), ch.5 (Controversa 3), home.
- **Source:** Pareschi et al. (paywalled, IEEE) - **corroborated (local)** by:
  - `zhu-2016-second-level.pdf`: "For the Spectral Test, Pareschi et al. [10] pointed out that the variance sigma^2 = 0.95 x 0.05 x n/c with c = 3.8, is closer to the ideal distribution than the original value (c = 4) in the NIST SP 800-22 test suite."
- **Note:** our Monte-Carlo Var(d) approx 1.056 ~ 4/3.8 = 1.053; the c=3.8 attribution is exactly what Zhu credits to Pareschi.

## 7. Iwasaki: variance smaller than expected, theoretical value unknown; derived via Parseval; d not exactly N(0,1)
- **Where:** ch.5 (Controversa 3).
- **Source:** `iwasaki-2018-variance-parseval.pdf` - **verbatim (local)**.
- **Quotes:** "the variance of the test statistic is smaller than expected and the theoretical value of the variance is not known ... we ... derive the variance using Parseval's theorem under particular assumptions." ; "The variance of N1 observed numerically is far smaller than expected."

## 8. Zhu et al.: the second-level tests are weak; mean drift/asymmetry remains; propose Q-value
- **Where:** ch.5 (level-2 tests weak; mean drift).
- **Source:** `zhu-2016-second-level.pdf` - **verbatim (local)**.
- **Quotes:** "their second-level tests are flawed due to the inconsistency between the assessed distribution and the assumed one ... we propose Q-value as the metric ... to replace the original P-value." ; "the mean drift (or the asymmetry) still exists after modifying the variance."

## 9. Chen et al.: error analysis of the two-level tests; p-value deviations; false positives at large samples
- **Where:** ch.5 (false-positive / error-analysis angle).
- **Source:** Chen et al. (paywalled, IEEE) - **abstract** (Semantic Scholar).
- **Quote:** "we make a two-stage error analysis of the commonly used two-level randomness tests in the NIST SP 800-22 test suite ... estimates of the p-value deviations of chi-square approximation in the basic tests ... explaining some false positive issues in practical test experiments."
- **Note:** this paper is *not* about the DFT statistic's mean shift; the referat was corrected (2026-05-23) to attribute the mean drift to Zhu (claim 8) and cite Chen only for the false-positive/error analysis.

## 10. Iwasaki-Umeno: a new DFT-style test with a derivable distribution and stronger detection
- **Where:** ch.5 (alte limitari / putere de detectie).
- **Source:** `iwasaki-umeno-2017-new-dft-test.pdf` - **verbatim (local)**.
- **Quote:** "we propose a new test using variance of power spectrum as the test statistic, whose reference distribution can be theoretically derived ... the proposed test has stronger detection power than DFTT."

## 11. Bluestein: arbitrary-length DFT via a linear-filtering (chirp-z) convolution
- **Where:** ch.3 (Transformata rapida).
- **Source:** Bluestein 1970 (paywalled, IEEE) - **abstract/metadata**; the algorithm is standard and implemented in `src/fft.cpp` (cross-checked against the direct DFT in `tests/test_fft.cpp`).
- **Title (verbatim, Crossref):** "A linear filtering approach to the computation of discrete Fourier transform".

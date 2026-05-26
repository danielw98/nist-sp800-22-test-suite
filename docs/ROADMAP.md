# ROADMAP

## Ce este (si ce vrem sa fie) site-ul

`prng-nist-tests.student-dev.ro` are trei roluri:

1. **Prezentare** pentru materie - referatul despre testul spectral (DFT).
2. **Tool interactiv** cu API public utilizabil - rulezi orice test pe orice secventa.
3. **Tool de invatare** - explica testele NIST, cu vizualizari, exemple si cod.

## Stadiu actual

Suita NIST SP 800-22 este **completa: toate cele 15 teste sunt implementate si
validate** (fata de exemplele publicate NIST si codul de referinta `reference/sts`).
Arhitectura: un registru de teste + un runner generic + un API uniform
(`POST /api/run/{test}`) + un catalog web (`/tests`) cu o pagina didactica si
rulabila per test. Testul spectral (DFT) ramane tratat in profunzime (referat,
controverse, simulari, figuri).

Verdictul este dat acum prin **valoarea critica din distributie** - statistica
comparata cu `z_{1-alpha/2}` (teste normale) sau `chi^2_{alpha,df}` (teste chi-patrat),
cum ai citi dintr-un tabel statistic - cu valoarea p pastrata ca informatie secundara.

Cele 15 teste: Frequency (Monobit), Frequency within a Block, Runs, Longest Run,
Binary Matrix Rank, Discrete Fourier Transform, Non-overlapping & Overlapping
Template Matching, Maurer's Universal, Linear Complexity, Serial, Approximate
Entropy, Cumulative Sums, Random Excursions + Variant.

## Cum se adauga un test

Vezi `docs/ADDING-A-TEST.md`. Pe scurt: un singur `src/<slug>_test.cpp` care
implementeaza `RandomnessTest`, o linie in `src/test_registry.cpp` si o linie in
`CMakeLists.txt`; pe web, doar comuta `status: "implemented"` in `web/lib/tests.ts`.
Restul (runner, API, pagini, analiza de nivel 2) se reutilizeaza neschimbat.

# PROJECT - arhitectura si dezvoltare

Implementarea testului spectral NIST SP 800-22 (sectiunea 2.6): o biblioteca C++,
un referat si o aplicatie web. Document de inginerie (separat de referatul
academic, livrat ca PDF in radacina).

## Decizia centrala

Fidel **codului** de referinta NIST, nu exemplului din documentatie: pentru
exemplul de 100 de biti, raspunsul corect este `N1 = 48, p = 0.646355` (verificat
cu driverul de referinta original si cu numpy). Documentatia tipareste 46, o
valoare ce ar necesita un prag mai mic decat cel corect - de aici corectiile
propuse in literatura.

## Structura

```
spectral-dft-test/
  src/            biblioteca C++17 + 2 CLI-uri
  tests/          teste doctest C++ + validate.py + run_all
  data/           fisierele NIST (e, pi, sqrt2, sqrt3, sha1, exemplul de 100 biti)
  docs/           make_figures.py + figuri + docs de inginerie
                  (PROJECT/ROADMAP/TESTING/API/GRAPHS/ADDING-A-TEST)
  web/            aplicatia Next.js (UI + API public)
  papers/         CLAIMS.md/INDEX.md (controversa)
  reference/      driverul de referinta NIST original compilat
  Testul-Spectral-DFT-NIST-SP-800-22.pdf   referatul compilat
  CMakeLists.txt
```

## Harta modulelor C++ (`src/`)

| Fisier | Responsabilitate |
| --- | --- |
| `complex.hpp` | tipul `Complex` (`std::complex<double>`), definit o singura data |
| `fft.{hpp,cpp}` | `DftEngine` (interfata Strategy); `FftEngine` = radix-2 Cooley-Tukey + Bluestein pentru orice n; `DirectDftEngine` = O(n^2) pentru verificare |
| `engine_factory.hpp` | `enum class Method` + `parseMethod` (string -> enum, la granita CLI) + `makeEngine` (total pe enum, nu arunca); comun celor 2 CLI |
| `bit_sequence.{hpp,cpp}` | `BitSequence`: citire (text/binar/stdin), `toBipolar` (+/-1), `split` in fluxuri |
| `test.hpp` | `RandomnessTest` (interfata suitei) + `TestReport` (plic uniform: n, p, statistici, optional spectru) |
| `spectral_test.{hpp,cpp}` | `SpectralTest` (DFT, cei 8 pasi NIST; `analyze` + spectrul); implementeaza `RandomnessTest` |
| `test_registry.{hpp,cpp}` | `id -> test`; `makeTest(id, params)` + `listTests()` (singurul loc de inregistrare) |
| `nist_runner.{hpp,cpp}` | runner generic: `RunRequest` in, plic JSON/text afara |
| `*_test.cpp` (monobit, block_frequency, runs, longest_run, rank, non_overlapping, overlapping, universal, linear_complexity, serial, approx_entropy, cusum, random_excursions, random_excursions_variant) | cele 14 teste NIST de prim nivel in afara de DFT, fiecare un `RandomnessTest` |
| `assessment.{hpp,cpp}` | `Assessment`: proportie + uniformitate (chi-patrat) peste orice `RandomnessTest` |
| `special.{hpp,cpp}` | gamma incompleta regularizata + CDF normala (valori p) + cuantile inverse (`normalQuantile`, `chiSquareCritical`) pentru valorile critice |
| `verdict.{hpp,cpp}` | `criticalVerdict`: statistica vs valoarea critica (`z_{1-alpha/2}` / `chi^2_{alpha,df}`); echivalent cu p < alpha, dar prezentat ca dintr-un tabel |
| `nist_main.cpp` / `nist_assess_main.cpp` | binarele `nist_test` / `nist_assess` (CLI `--test <id>`) |

Fiecare functie/clasa are un comentariu de documentare in header. Pasii 1-8 din
NIST se regasesc unu-la-unu in `SpectralTest::analyze`. Cum se adauga un test nou:
`docs/ADDING-A-TEST.md`.

## Build

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/dft_tests            # teste unitare doctest (FFT vs direct, exemplul NIST = 48)
```

## Fluxul datelor (build-time vs runtime in aplicatie)

- **Build-time** (`web/scripts/prebuild.mjs`, rulat de `pnpm gen` / inainte de
  `next build`): ruleaza binarele pe fisierele NIST si scrie `web/data-static/`
  (`nist-results.json`, `assess-results.json`, `source-index.json`, copii ale
  `figure-data/*.json`), copiaza SVG-urile + PDF-ul referatului
  (`Testul-Spectral-DFT-NIST-SP-800-22.pdf`) in `web/public/`.
  Paginile importa aceste JSON-uri static (zero cost la runtime).
- **Runtime** (rulari pe input dat de utilizator): rutele API
  `web/app/api/{run,assess,sequence}/route.ts` lanseaza binarul C++ (`spawn`),
  calea din `DFT_BIN_DIR` (implicit `../build`, in Docker `/app/bin`); datele din
  `DFT_DATA_DIR`. Validare cu zod, limite de rata, timeout.

## Aplicatia web (`web/`)

- Next.js 15 (App Router) + React 19 + Tailwind v4, `output: 'standalone'`.
- Continut paper ca MDX/HTML, math cu KaTeX, cod cu Shiki, grafice **Plotly**
  (`react-plotly.js`, client-only via import dinamic ssr:false).
- Pagini: `/`, `/runner`, `/sequences`, `/controversy`, `/validation`, `/paper`,
  `/source`, `/figures`, `/api-docs`, `/references`.
- Local (fara Docker): `cd web && pnpm install && pnpm dev` -> http://localhost:3000.

## Teste

Detaliat in `docs/TESTING.md`. Pe scurt, testat in limbajul codului testat:

- C++ (doctest): `tests/test_*.cpp` -> tinta `dft_tests` (`ctest -R unit`) - FFT vs
  direct, mecanica testului, conformitate NIST (N1=48), bit_sequence, special,
  assessment, verdict. 35 de cazuri / 386 de aserțiuni.
- Web (vitest): `web/lib/*.test.ts` - rutele API (`/api/run`, `/api/health`, prin
  binarul real), validarea zod, limitatorul de rata. `cd web && pnpm test`.
- Python: `tests/validate.py` - raspuns cunoscut + verificari statistice + cross-check.
- Tot lantul C++: `tests/run_all.{ps1,sh}`.

## Deploy

prng-nist-tests.student-dev.ro, Docker pe VPS-B, container `rngtests-web` pe reteaua
externa `studlab_net`, in spatele nginx-ului partajat `studlab-nginx`. Deploy din
`web/`: `deploy_rngtests.bat`. Vezi `web/Dockerfile` (context = parinte),
`web/docker-compose.yml`, `web/nginx/prng-nist-tests.conf`. Nicio constructie Docker
locala - imaginea se face pe server.

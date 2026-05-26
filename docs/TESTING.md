# TESTING - how the project is tested

The test suite is split by *what is under test*, and each layer uses the natural
framework for that language. This is deliberate (see "Why not one language / C#"
at the end). Three layers:

| Layer | Tool | Location | Tests what |
|-------|------|----------|------------|
| C++ unit + NIST compliance | doctest | `tests/test_*.cpp` -> `dft_tests` | the library directly (FFT, the test math, edge cases, the SP 800-22 reference values) |
| Web API + helpers | vitest | `web/lib/*.test.ts` | the route handlers (end-to-end via the real binary), input validation, the rate limiter |
| Statistical validation | Python (numpy/scipy) | `tests/validate.py` | known answers, Monte-Carlo behaviour under H0, cross-check vs the original NIST driver |

## 1. C++ unit + compliance tests (doctest)

doctest is a single vendored header (`tests/doctest.h`); no package manager
needed. CMake globs `tests/test_*.cpp` into one `dft_tests` executable and
registers it with CTest as `unit`.

- `test_fft.cpp` - the fast engine (radix-2 + Bluestein) agrees with the naive
  O(n^2) DFT across power-of-two and non-power-of-two lengths; constant-signal DC
  bin; empty / single-element transforms.
- `test_spectral.cpp` - the threshold / N0 formulas; pathological inputs (all
  ones/zeros, periodic, period-2 on the excluded Nyquist) are rejected; FFT and
  direct engines give identical verdicts.
- `test_nist.cpp` - **compliance**: the section 2.6.8 example reproduces the
  reference code (N1 = 48, N0 = 47.5, d = 0.458831, p = 0.646355) and the
  decision rule `passed iff p >= alpha`.
- `test_bit_sequence.cpp` - parsing (only 0/1 kept), the bipolar mapping, stream
  splitting (trailing partial block dropped, `maxStreams` cap).
- `test_special.cpp` - the incomplete gamma `Q(a,x)` against closed forms
  (`Q(1,x)=e^-x`, `Q(1/2,x)=erfc(sqrt(x))`, boundaries).
- `test_assessment.cpp` - second-level proportion/uniformity bookkeeping; good
  random streams pass, a periodic generator is rejected.

Run:

```sh
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build            # or: ./build/dft_tests
```

## 2. Web API + helper tests (vitest)

`web/vitest.config.ts` maps `@/...` to the project root so tests import route
handlers and lib modules exactly as the app does.

- `lib/api-routes.test.ts` - calls `POST /api/run` and `GET /api/health`
  directly. The run handler spawns the compiled `../build/nist_test`, so this is a
  true end-to-end check: posting the NIST example returns N1 = 48 / p = 0.646355;
  bad input gives 400; health reports the binaries present. (Requests carry no
  `X-Forwarded-For`, so the client IP is `unknown`, which is allowlisted - the
  rate limiter never trips during tests.)
- `lib/validation.test.ts` - the zod schema and `sanitizeBits`.
- `lib/ratelimit.test.ts` - per-minute / per-IP-week / global limits and the
  allowlist.

Run (needs the C++ binaries built first, since the API tests spawn them):

```sh
cd web
pnpm test            # vitest run
```

## 3. Statistical validation (Python)

`tests/validate.py` runs the compiled binary (`--json --spectrum`) over a battery
of cases and writes a report + per-case graphs:

- **known answers**: NIST example, all-ones/zeros, periodic, period-2, random;
- **statistical under H0**: false-positive rate ~ alpha, p-value uniformity;
- **cross-check**: against the original NIST reference driver where available.

```sh
python tests/validate.py
```

## Why not one language / C#?

We test in the language of the code under test, on purpose:

- The **C++ library** is tested in C++ (doctest) so tests link against the real
  classes - no FFI, no marshalling, no separate process. A C# test project could
  only reach the C++ through P/Invoke or by shelling out to the binary, i.e.
  black-box - strictly weaker than calling `SpectralTest::analyze` directly.
- The **web API** is TypeScript, so vitest imports the route handlers and runs
  them in-process (and still exercises the real binary through `spawn`). From C#
  we could only hit it over HTTP against a running server.
- The **statistical checks** lean on numpy/scipy, where they are shortest.

C# (xUnit/NUnit) would be the right choice if the production code were C#. It is
not, so adding the .NET toolchain purely for tests would buy only black-box
coverage at the cost of an extra runtime. If we ever port the core to C#, the
tests move with it; until then, polyglot tests-by-layer is the correct shape.

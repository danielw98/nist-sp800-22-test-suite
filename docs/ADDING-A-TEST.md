# Adding a NIST test

The suite is built so a new test is one C++ file + two one-line registrations.
The shared infrastructure (`BitSequence`, `regularizedGammaQ`/`normalCdf`/`erfc`,
the generic runner, the second-level `Assessment`, the web registry-driven API and
pages) is reused as-is.

## C++ (the computation)

1. **One file `src/<slug>_test.cpp`.** Define a file-local class implementing
   `RandomnessTest` (`src/test.hpp`):

   ```cpp
   #include "basic_tests.hpp"   // declares the factory
   #include <cmath>
   #include "special.hpp"        // regularizedGammaQ / normalCdf if needed

   namespace dft {
   namespace {
   class MyTest : public RandomnessTest {
   public:
       std::string id() const override { return "my-test"; }
       std::string name() const override { return "My Test"; }
       std::size_t minLength() const override { return 100; }
       TestReport run(const BitSequence &seq) const override {
           const auto &bits = seq.bits();           // 0/1 access
           // ... compute the statistic and the p-value ...
           TestReport r;
           r.n = bits.size();
           r.pValue = /* ... */;
           r.stats = {{"my_stat", value}};          // shown in UI + JSON
           return r;
       }
   };
   } // namespace
   std::unique_ptr<RandomnessTest> makeMyTest(const TestParams &p) {
       return std::make_unique<MyTest>(/* p.blockSize, ... */);
   }
   } // namespace dft
   ```

   `pValue()` is derived from `run()`, so the second-level `Assessment` works for
   free. Reusable helpers: `BitSequence::split(M)` for block tests,
   `regularizedGammaQ(a, x)` for chi-square -> p, `std::erfc`, `normalCdf`.

2. **Declare the factory** in `src/basic_tests.hpp`:
   `std::unique_ptr<RandomnessTest> makeMyTest(const TestParams &);`

3. **Register it** in `src/test_registry.cpp` (one line in the table):
   `{"my-test", "My Test", 100, makeMyTest},`

4. **Build target**: the file is picked up automatically only if you add
   `src/<slug>_test.cpp` to the `dft` library list in `CMakeLists.txt`.

That is the whole engine change. `nist_test --test my-test` and
`nist_assess --test my-test` now work.

## Web (the page + API)

5. **Flip the registry entry** in `web/lib/tests.ts`: set the test's
   `status: "implemented"` (it is already listed as `"planned"` with its didactic
   copy). Adjust `params`/`viz` if needed. That single change makes
   `POST /api/run/my-test` stop returning `501`, and turns the `/tests/my-test`
   page from a "planned" notice into a live runner. No route, page, or validation
   code to touch - they are all driven by the registry.

## Validation

6. **One doctest** in `tests/test_basic.cpp`: assert the p-value against NIST's
   published example (sec 2.x.8) and/or cross-check against the reference
   implementation in `reference/sts`. Run `ctest --test-dir build`.

That's it: one `*_test.cpp`, two registry lines (C++), one status flip (web), one
assertion. See any of `src/monobit_test.cpp` ... `src/cusum_test.cpp` as templates.

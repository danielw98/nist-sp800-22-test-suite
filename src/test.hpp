// Suite-wide abstraction for an SP 800-22 statistical test.
//
// Every test (DFT, Monobit, Runs, ...) implements RandomnessTest and returns a
// uniform TestReport: a p-value plus a list of named intermediate statistics
// (and, for the DFT test only, an optional spectrum for plotting). This lets one
// generic runner and one second-level Assessment treat every test the same way.
#ifndef DFT_TEST_HPP
#define DFT_TEST_HPP

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "bit_sequence.hpp"

namespace dft {

// One named intermediate quantity a test reports (e.g. {"d", 0.4588}).
struct NamedStat
{
    std::string label;
    double value;
};

// Reference distribution of a test's first-level statistic, used to decide the
// verdict by comparing the statistic against a critical value from the table
// (rather than the p-value against alpha):
//   Normal2Sided    - signed z; reject if |statistic| > z_{1-alpha/2}
//   HalfNormalUpper  - folded/non-negative z; reject if statistic > z_{1-alpha/2}
//   ChiSquareUpper   - reject if statistic > chi^2_{alpha, dof}
//   None             - no tabulated critical value; the verdict stays p-value based
enum class RefDist
{
    None,
    Normal2Sided,
    HalfNormalUpper,
    ChiSquareUpper,
};

// One downsampled spectrum sample (DFT test only).
struct SpectrumPoint
{
    std::size_t k;
    double mag;
    bool above; // |S[k]| >= threshold
};

// Uniform outcome of one test on one sequence.
struct TestReport
{
    std::size_t n = 0;
    double pValue = 0.0;                            // headline p-value (see pValues note)
    // First-level statistic and its reference distribution, for the critical-value
    // verdict. statistic is the value compared to the table critical value; dof is
    // used only for ChiSquareUpper. dist == None -> the verdict falls back to p-value.
    double statistic = 0.0;
    RefDist dist = RefDist::None;
    double dof = 0.0;
    std::vector<NamedStat> stats;                  // test-specific quantities
    // Per-state / per-template p-values for tests that yield several (random
    // excursions, its variant, template matching). Empty for single-value tests.
    // When present, the headline pValue above is the most conservative summary
    // (the minimum), so the registry, runner and second-level Assessment keep a
    // single value to work with.
    std::vector<NamedStat> pValues;
    std::optional<std::vector<SpectrumPoint>> spectrum; // filled only by the DFT test
};

// Common interface. A new test implements id/name/minLength/run; pValue is
// derived from run() so the second-level Assessment needs nothing else.
class RandomnessTest
{
public:
    virtual ~RandomnessTest() = default;

    virtual std::string id() const = 0;        // url-safe slug, e.g. "monobit"
    virtual std::string name() const = 0;      // human-readable name
    virtual std::size_t minLength() const = 0; // NIST recommended minimum n

    virtual TestReport run(const BitSequence &seq) const = 0;

    double pValue(const BitSequence &seq) const
    {
        return run(seq).pValue;
    }
};

} // namespace dft

#endif // DFT_TEST_HPP

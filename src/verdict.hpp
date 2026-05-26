// Turns a test's first-level statistic + reference distribution into a verdict by
// comparing it against the table critical value (z_{1-alpha/2} or chi^2_{alpha,dof}),
// the classical alternative to comparing the p-value against alpha. The two are
// mathematically equivalent; this module is what lets the suite report the verdict
// the way a statistics table would.
#ifndef DFT_VERDICT_HPP
#define DFT_VERDICT_HPP

#include "test.hpp"

namespace dft {

struct CriticalVerdict
{
    bool hasCritical; // false -> no tabulated critical value; the p-value fallback was used
    double critical;  // z_{1-alpha/2} (normal/half-normal) or chi^2_{alpha, dof}
    bool reject;      // true -> reject H0 (non-random) at this alpha
};

// Decide via the critical value when the statistic's distribution is known
// (dist != None), else fall back to p-value vs alpha (e.g. cusum).
CriticalVerdict criticalVerdict(const TestReport &report, double alpha);

// Human-readable name of a reference distribution.
const char *refDistLabel(RefDist dist);

} // namespace dft

#endif // DFT_VERDICT_HPP

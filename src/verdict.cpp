#include "verdict.hpp"

#include <cmath>

#include "special.hpp"

namespace dft {

CriticalVerdict criticalVerdict(const TestReport &report, double alpha)
{
    switch (report.dist)
    {
    case RefDist::Normal2Sided:
    case RefDist::HalfNormalUpper:
    {
        // Both compare |statistic| to the two-sided normal critical value; a folded
        // (half-normal) statistic is already non-negative, so |.| is a no-op there.
        const double zc = normalQuantile(1.0 - alpha / 2.0);
        return {true, zc, std::fabs(report.statistic) > zc};
    }
    case RefDist::ChiSquareUpper:
    {
        const double cc = chiSquareCritical(alpha, report.dof);
        return {true, cc, report.statistic > cc};
    }
    case RefDist::None:
    default:
        return {false, 0.0, report.pValue < alpha};
    }
}

const char *refDistLabel(RefDist dist)
{
    switch (dist)
    {
    case RefDist::Normal2Sided:
        return "normal (two-sided)";
    case RefDist::HalfNormalUpper:
        return "half-normal (upper)";
    case RefDist::ChiSquareUpper:
        return "chi-square (upper)";
    case RefDist::None:
    default:
        return "";
    }
}

} // namespace dft

// Runs Test - NIST SP 800-22 section 2.3.
// Counts the total number of runs (maximal blocks of identical bits) and checks
// it against what a random sequence with the observed proportion would give.
#include "basic_tests.hpp"

#include <cmath>
#include <cstdint>

namespace dft {
namespace {

class RunsTest : public RandomnessTest
{
public:
    std::string id() const override
    {
        return "runs";
    }
    std::string name() const override
    {
        return "Runs";
    }
    std::size_t minLength() const override
    {
        return 100;
    }

    TestReport run(const BitSequence &seq) const override
    {
        const std::vector<std::uint8_t> &bits = seq.bits();
        const std::size_t n = bits.size();

        std::size_t ones = 0;
        for (std::uint8_t bit : bits)
        {
            ones += bit;
        }
        const double pi = static_cast<double>(ones) / static_cast<double>(n);

        TestReport report;
        report.n = n;

        // Prerequisite: the monobit proportion must be near 1/2, else the test
        // does not apply and the p-value is 0.
        if (std::fabs(pi - 0.5) >= 2.0 / std::sqrt(static_cast<double>(n)))
        {
            report.pValue = 0.0;
            report.stats = {{"V_obs", 0.0}, {"pi", pi}};
            return report;
        }

        long vObs = 1; // total runs = transitions + 1
        for (std::size_t i = 0; i + 1 < n; i++)
        {
            if (bits[i] != bits[i + 1])
            {
                vObs++;
            }
        }

        const double expected = 2.0 * static_cast<double>(n) * pi * (1.0 - pi);
        const double denom = 2.0 * std::sqrt(2.0 * static_cast<double>(n)) * pi * (1.0 - pi);
        report.pValue = std::erfc(std::fabs(static_cast<double>(vObs) - expected) / denom);
        // Standard-normal statistic: z = sqrt(2) * (V_obs - E) / denom.
        report.statistic = std::sqrt(2.0) * (static_cast<double>(vObs) - expected) / denom;
        report.dist = RefDist::Normal2Sided;
        report.stats = {{"V_obs", static_cast<double>(vObs)}, {"pi", pi}};
        return report;
    }
};

} // namespace

std::unique_ptr<RandomnessTest> makeRuns(const TestParams &)
{
    return std::make_unique<RunsTest>();
}

} // namespace dft

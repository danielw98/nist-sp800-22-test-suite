// Frequency (Monobit) Test - NIST SP 800-22 section 2.1.
// Tests whether the proportion of ones is close to 1/2 over the whole sequence.
#include "basic_tests.hpp"

#include <cmath>
#include <cstdint>

namespace dft {
namespace {

class MonobitTest : public RandomnessTest
{
public:
    std::string id() const override
    {
        return "monobit";
    }
    std::string name() const override
    {
        return "Frequency (Monobit)";
    }
    std::size_t minLength() const override
    {
        return 100;
    }

    TestReport run(const BitSequence &seq) const override
    {
        const std::vector<std::uint8_t> &bits = seq.bits();
        const std::size_t n = bits.size();

        long sum = 0; // S_n = #1 - #0
        for (std::uint8_t bit : bits)
        {
            sum += bit ? 1 : -1;
        }
        const double sObs = std::fabs(static_cast<double>(sum)) / std::sqrt(static_cast<double>(n));

        TestReport report;
        report.n = n;
        report.pValue = std::erfc(sObs / std::sqrt(2.0));
        report.statistic = sObs; // |S_n|/sqrt(n); reject if it exceeds z_{1-alpha/2}
        report.dist = RefDist::HalfNormalUpper;
        report.stats = {{"S_n", static_cast<double>(sum)}, {"s_obs", sObs}};
        return report;
    }
};

} // namespace

std::unique_ptr<RandomnessTest> makeMonobit(const TestParams &)
{
    return std::make_unique<MonobitTest>();
}

} // namespace dft

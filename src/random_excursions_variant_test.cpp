// Random Excursions Variant Test - NIST SP 800-22 section 2.15.
// On the +-1 cumulative-sum walk, counts the total number of visits to each of
// the eighteen states x in {-9..-1, 1..9} and compares each to the cycle count
// J. Yields one p-value per state (eighteen in all).
#include "basic_tests.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

namespace dft {
namespace {

constexpr int kMaxState = 9;                // states |x| up to 9
constexpr std::size_t kStateSpan = 2 * kMaxState + 1; // indices for -9..9 (0 unused)

std::string stateLabel(int x)
{
    return std::string("x=") + (x > 0 ? "+" : "") + std::to_string(x);
}

class RandomExcursionsVariantTest : public RandomnessTest
{
public:
    std::string id() const override
    {
        return "random-excursions-variant";
    }
    std::string name() const override
    {
        return "Random Excursions Variant";
    }
    std::size_t minLength() const override
    {
        return 1000000;
    }

    TestReport run(const BitSequence &seq) const override
    {
        const std::vector<std::uint8_t> &bits = seq.bits();
        const std::size_t n = bits.size();

        TestReport report;
        report.n = n;

        // One pass: running +-1 walk, counting cycles (returns to zero) and the
        // total visits to each state. The trailing partial cycle is counted in J.
        long sum = 0;
        long cycles = 0;
        std::array<long, kStateSpan> visits{};
        for (std::size_t i = 0; i < n; i++)
        {
            sum += bits[i] ? 1 : -1;
            if (sum == 0)
            {
                cycles++;
            }
            else if (sum >= -kMaxState && sum <= kMaxState)
            {
                visits[static_cast<std::size_t>(sum + kMaxState)]++;
            }
        }
        if (n > 0 && sum != 0)
        {
            cycles++; // trailing partial cycle
        }

        if (cycles < 1)
        {
            report.pValue = 0.0;
            report.stats = {{"J", static_cast<double>(cycles)}};
            return report;
        }

        double minP = 1.0;
        double maxZ = 0.0;
        for (int x = -kMaxState; x <= kMaxState; x++)
        {
            if (x == 0)
            {
                continue;
            }
            const long xi = visits[static_cast<std::size_t>(x + kMaxState)];
            const double denominator = std::sqrt(2.0 * static_cast<double>(cycles) * (4.0 * std::abs(x) - 2.0));
            const double arg = std::fabs(static_cast<double>(xi - cycles)) / denominator;
            const double p = std::erfc(arg);
            report.pValues.push_back({stateLabel(x), p});
            maxZ = std::max(maxZ, std::sqrt(2.0) * arg); // |z| = sqrt(2) * erfc-argument
            minP = std::min(minP, p);
        }

        report.pValue = minP;
        report.statistic = maxZ; // most extreme per-state |z| (corresponds to min p)
        report.dist = RefDist::HalfNormalUpper;
        report.stats = {{"J", static_cast<double>(cycles)}};
        return report;
    }
};

} // namespace

std::unique_ptr<RandomnessTest> makeRandomExcursionsVariant(const TestParams &)
{
    return std::make_unique<RandomExcursionsVariantTest>();
}

} // namespace dft

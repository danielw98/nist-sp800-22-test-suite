// Random Excursions Test - NIST SP 800-22 section 2.14.
// Builds the +-1 cumulative-sum walk, splits it into cycles (returns to zero),
// and for each of the eight states x in {-4..-1, 1..4} chi-square-tests how the
// per-cycle visit counts to x are distributed. Yields one p-value per state.
#include "basic_tests.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

#include "special.hpp"

namespace dft {
namespace {

constexpr int kMaxState = 4;           // states with |x| up to 4
constexpr std::size_t kStates = 8;     // -4, -3, -2, -1, 1, 2, 3, 4
constexpr std::size_t kClasses = 6;    // per-cycle visit counts 0, 1, 2, 3, 4, >=5
constexpr long kMinCycles = 500;       // NIST: too few cycles makes the test unreliable

// Class probabilities pi[k] for a state with |x| = absState (NIST sec 2.14.4).
std::array<double, kClasses> stateProbabilities(int absState)
{
    const double p0 = 1.0 - 1.0 / (2.0 * absState);
    std::array<double, kClasses> pi{};
    pi[0] = p0;
    for (std::size_t k = 1; k + 1 < kClasses; k++)
    {
        pi[k] = 1.0 / (4.0 * absState * absState) * std::pow(p0, static_cast<double>(k) - 1.0);
    }
    pi[kClasses - 1] = 1.0 / (2.0 * absState) * std::pow(p0, 4.0);
    return pi;
}

// Index in [0, 8) for a state value in {-4..-1, 1..4}: -4->0 ... -1->3, 1->4 ... 4->7.
std::size_t stateIndex(long value)
{
    return value < 0 ? static_cast<std::size_t>(value + kMaxState)
                     : static_cast<std::size_t>(value + kMaxState - 1);
}

std::string stateLabel(int x)
{
    return std::string("x=") + (x > 0 ? "+" : "") + std::to_string(x);
}

class RandomExcursionsTest : public RandomnessTest
{
public:
    std::string id() const override
    {
        return "random-excursions";
    }
    std::string name() const override
    {
        return "Random Excursions";
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

        // Step 3: the +-1 cumulative-sum walk.
        std::vector<long> walk(n);
        long sum = 0;
        for (std::size_t i = 0; i < n; i++)
        {
            sum += bits[i] ? 1 : -1;
            walk[i] = sum;
        }

        // Step 4: cycle boundaries are the returns to zero, plus a trailing partial cycle.
        std::vector<std::size_t> cycleEnd;
        for (std::size_t i = 0; i < n; i++)
        {
            if (walk[i] == 0)
            {
                cycleEnd.push_back(i);
            }
        }
        if (n > 0 && walk[n - 1] != 0)
        {
            cycleEnd.push_back(n);
        }
        const long cycles = static_cast<long>(cycleEnd.size());

        static constexpr std::array<int, kStates> states = {-4, -3, -2, -1, 1, 2, 3, 4};
        if (cycles < 1)
        {
            report.pValue = 0.0;
            report.stats = {{"J", static_cast<double>(cycles)}};
            return report;
        }

        // Steps 5/6: v[k][s] = number of cycles in which state s is visited exactly k times.
        std::array<std::array<long, kStates>, kClasses> v{};
        std::size_t cycleStop = 0;
        for (long c = 0; c < cycles; c++)
        {
            const std::size_t cycleStart = cycleStop;
            cycleStop = cycleEnd[static_cast<std::size_t>(c)];
            std::array<long, kStates> counter{};
            for (std::size_t i = cycleStart; i < cycleStop; i++)
            {
                const long value = walk[i];
                if (value != 0 && value >= -kMaxState && value <= kMaxState)
                {
                    counter[stateIndex(value)]++;
                }
            }
            for (std::size_t s = 0; s < kStates; s++)
            {
                const std::size_t k = counter[s] < static_cast<long>(kClasses) - 1
                                          ? static_cast<std::size_t>(counter[s])
                                          : kClasses - 1;
                v[k][s]++;
            }
        }

        // Steps 7/8: per-state chi-square and p-value; the headline is the minimum.
        double minP = 1.0;
        double maxChiSquare = 0.0;
        for (std::size_t s = 0; s < kStates; s++)
        {
            const std::array<double, kClasses> pi = stateProbabilities(std::abs(states[s]));
            double chiSquare = 0.0;
            for (std::size_t k = 0; k < kClasses; k++)
            {
                const double expected = static_cast<double>(cycles) * pi[k];
                const double diff = static_cast<double>(v[k][s]) - expected;
                chiSquare += diff * diff / expected;
            }
            const double p = regularizedGammaQ((kClasses - 1) / 2.0, chiSquare / 2.0);
            report.pValues.push_back({stateLabel(states[s]), p});
            maxChiSquare = std::max(maxChiSquare, chiSquare);
            minP = std::min(minP, p);
        }

        report.pValue = minP;
        report.statistic = maxChiSquare; // most extreme per-state chi-square (corresponds to min p)
        report.dof = static_cast<double>(kClasses - 1); // 5
        report.dist = RefDist::ChiSquareUpper;
        report.stats = {{"J", static_cast<double>(cycles)},
                        {"enough_cycles", cycles >= kMinCycles ? 1.0 : 0.0}};
        return report;
    }
};

} // namespace

std::unique_ptr<RandomnessTest> makeRandomExcursions(const TestParams &)
{
    return std::make_unique<RandomExcursionsTest>();
}

} // namespace dft

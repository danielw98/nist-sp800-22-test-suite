// Overlapping Template Matching Test - NIST SP 800-22 section 2.8.
// Counts, in each M-bit block, how many times an all-ones template of length m
// occurs (the window advances by one bit even after a match) and chi-square-
// tests the distribution of per-block counts against the reference probabilities.
#include "basic_tests.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

#include "special.hpp"

namespace dft {
namespace {

constexpr int kTemplateLength = 9;          // m: NIST fixes the template to nine ones
constexpr std::size_t kBlockLength = 1032;  // M
constexpr std::size_t kClasses = 6;         // occurrence counts 0,1,2,3,4,>=5 (K = 5)

// P(template occurs exactly u times in a block), the compound-Poisson term from
// the original NIST reference (sec 2.8.4). NOTE: the arcetri/sts fork replaces
// these with slightly different recomputed constants; the original formula here
// is what reproduces the published 2.8.8 example (p = 0.110434).
double occurrenceProbability(unsigned u, double eta)
{
    if (u == 0)
    {
        return std::exp(-eta);
    }
    double sum = 0.0;
    for (unsigned l = 1; l <= u; l++)
    {
        sum += std::exp(-eta - u * std::log(2.0) + l * std::log(eta) - std::lgamma(l + 1) +
                        std::lgamma(u) - std::lgamma(l) - std::lgamma(u - l + 1));
    }
    return sum;
}

class OverlappingTemplateTest : public RandomnessTest
{
public:
    std::string id() const override
    {
        return "overlapping";
    }
    std::string name() const override
    {
        return "Overlapping Template Matching";
    }
    std::size_t minLength() const override
    {
        return 1000000;
    }

    TestReport run(const BitSequence &seq) const override
    {
        const std::vector<std::uint8_t> &bits = seq.bits();
        const std::size_t n = bits.size();
        const std::size_t N = n / kBlockLength;

        // Class probabilities for the fixed (m, M); lambda = (M-m+1)/2^m, eta = lambda/2.
        const double lambda = static_cast<double>(kBlockLength - kTemplateLength + 1) /
                              std::pow(2.0, kTemplateLength);
        const double eta = lambda / 2.0;
        std::array<double, kClasses> pi{};
        double piSum = 0.0;
        for (std::size_t i = 0; i + 1 < kClasses; i++)
        {
            pi[i] = occurrenceProbability(static_cast<unsigned>(i), eta);
            piSum += pi[i];
        }
        pi[kClasses - 1] = 1.0 - piSum;

        std::array<long, kClasses> v{};
        for (std::size_t block = 0; block < N; block++)
        {
            const std::size_t base = block * kBlockLength;
            long occurrences = 0;
            int run = 0;
            for (std::size_t i = 0; i < kBlockLength; i++)
            {
                run = bits[base + i] ? run + 1 : 0;
                if (run >= kTemplateLength)
                {
                    occurrences++; // overlapping: a length-(>=m) run yields one match per extra bit
                }
            }
            const std::size_t cls = occurrences < static_cast<long>(kClasses) - 1
                                        ? static_cast<std::size_t>(occurrences)
                                        : kClasses - 1;
            v[cls]++;
        }

        double chiSquare = 0.0;
        for (std::size_t i = 0; i < kClasses; i++)
        {
            const double expected = static_cast<double>(N) * pi[i];
            const double diff = static_cast<double>(v[i]) - expected;
            chiSquare += expected > 0.0 ? diff * diff / expected : 0.0;
        }
        // 6 classes -> K = 5 degrees of freedom -> P = Q(K/2, chi^2/2).
        const double pValue =
            N > 0 ? regularizedGammaQ((kClasses - 1) / 2.0, chiSquare / 2.0) : 0.0;

        TestReport report;
        report.n = n;
        report.pValue = pValue;
        report.statistic = chiSquare;
        report.dof = static_cast<double>(kClasses - 1); // 5
        report.dist = N > 0 ? RefDist::ChiSquareUpper : RefDist::None;
        report.stats = {{"M", static_cast<double>(kBlockLength)},
                        {"N", static_cast<double>(N)},
                        {"m", static_cast<double>(kTemplateLength)},
                        {"chi_sq", chiSquare}};
        return report;
    }
};

} // namespace

std::unique_ptr<RandomnessTest> makeOverlapping(const TestParams &)
{
    return std::make_unique<OverlappingTemplateTest>();
}

} // namespace dft

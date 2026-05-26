// Approximate Entropy Test - NIST SP 800-22 section 2.12.
// Compares the frequency of overlapping m- and (m+1)-bit patterns; a random
// sequence has near-maximal approximate entropy.
#include "basic_tests.hpp"

#include <cmath>
#include <cstdint>
#include <vector>

#include "special.hpp"

namespace dft {
namespace {

// phi^(m): sum of C_i ln C_i over overlapping m-bit patterns with wraparound.
double phi(const std::vector<std::uint8_t> &bits, std::size_t m)
{
    if (m == 0)
    {
        return 0.0;
    }
    const std::size_t n = bits.size();
    std::vector<long> counts(std::size_t(1) << m, 0);
    for (std::size_t i = 0; i < n; i++)
    {
        std::size_t pattern = 0;
        for (std::size_t j = 0; j < m; j++)
        {
            pattern = (pattern << 1) | bits[(i + j) % n];
        }
        counts[pattern]++;
    }
    double sum = 0.0;
    for (long c : counts)
    {
        if (c > 0)
        {
            const double proportion = static_cast<double>(c) / static_cast<double>(n);
            sum += proportion * std::log(proportion);
        }
    }
    return sum;
}

class ApproxEntropyTest : public RandomnessTest
{
public:
    explicit ApproxEntropyTest(std::size_t blockSize) : m_(blockSize > 0 ? blockSize : 2)
    {
    }
    std::string id() const override
    {
        return "approx-entropy";
    }
    std::string name() const override
    {
        return "Approximate Entropy";
    }
    std::size_t minLength() const override
    {
        return 100;
    }

    TestReport run(const BitSequence &seq) const override
    {
        const std::vector<std::uint8_t> &bits = seq.bits();
        const double n = static_cast<double>(bits.size());

        const double apEn = phi(bits, m_) - phi(bits, m_ + 1);
        const double chiSquare = 2.0 * n * (std::log(2.0) - apEn);

        TestReport report;
        report.n = bits.size();
        report.pValue = regularizedGammaQ(std::pow(2.0, static_cast<double>(m_) - 1.0), chiSquare / 2.0);
        report.statistic = chiSquare;
        report.dof = std::pow(2.0, static_cast<double>(m_)); // 2 * (2^(m-1))
        report.dist = RefDist::ChiSquareUpper;
        report.stats = {{"m", static_cast<double>(m_)}, {"ap_en", apEn}, {"chi_sq", chiSquare}};
        return report;
    }

private:
    std::size_t m_;
};

} // namespace

std::unique_ptr<RandomnessTest> makeApproxEntropy(const TestParams &params)
{
    return std::make_unique<ApproxEntropyTest>(params.blockSize);
}

} // namespace dft

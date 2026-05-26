// Non-overlapping Template Matching Test - NIST SP 800-22 section 2.7.
// Splits the sequence into N = 8 blocks and counts non-overlapping occurrences
// of an m-bit aperiodic template in each block (after a match the window slides
// past the template), then chi-square-tests the per-block counts against the
// expected mean. NIST iterates over a whole library of aperiodic templates; here
// we use the canonical first one, 0^(m-1)1 (the template of the 2.7.8 example).
#include "basic_tests.hpp"

#include <cmath>
#include <cstdint>
#include <vector>

#include "special.hpp"

namespace dft {
namespace {

constexpr std::size_t kBlocks = 8; // N

class NonOverlappingTemplateTest : public RandomnessTest
{
public:
    explicit NonOverlappingTemplateTest(std::size_t templateLength) : m_(templateLength > 0 ? templateLength : 9)
    {
    }
    std::string id() const override
    {
        return "non-overlapping";
    }
    std::string name() const override
    {
        return "Non-overlapping Template Matching";
    }
    std::size_t minLength() const override
    {
        return 1000000;
    }

    TestReport run(const BitSequence &seq) const override
    {
        const std::vector<std::uint8_t> &bits = seq.bits();
        const std::size_t n = bits.size();
        const std::size_t m = m_;
        const std::size_t M = n / kBlocks; // block length

        // Canonical aperiodic template: m-1 zeros then a single one.
        std::vector<std::uint8_t> templ(m, 0);
        if (m > 0)
        {
            templ[m - 1] = 1;
        }

        const double twoPowM = std::pow(2.0, static_cast<double>(m));
        const double mu = (static_cast<double>(M) - static_cast<double>(m) + 1.0) / twoPowM;
        const double sigmaSquared =
            static_cast<double>(M) * (1.0 / twoPowM - (2.0 * m - 1.0) / std::pow(2.0, 2.0 * m));

        double chiSquare = 0.0;
        const bool usable = M >= m && sigmaSquared > 0.0;
        if (usable)
        {
            for (std::size_t block = 0; block < kBlocks; block++)
            {
                const std::size_t base = block * M;
                long wObs = 0;
                std::size_t j = 0;
                while (j + m <= M)
                {
                    bool match = true;
                    for (std::size_t k = 0; k < m; k++)
                    {
                        if (templ[k] != bits[base + j + k])
                        {
                            match = false;
                            break;
                        }
                    }
                    if (match)
                    {
                        wObs++;
                        j += m; // non-overlapping: slide past the matched template
                    }
                    else
                    {
                        j++;
                    }
                }
                const double diff = static_cast<double>(wObs) - mu;
                chiSquare += diff * diff / sigmaSquared;
            }
        }
        // N blocks -> P = Q(N/2, chi^2/2).
        const double pValue = usable ? regularizedGammaQ(kBlocks / 2.0, chiSquare / 2.0) : 0.0;

        TestReport report;
        report.n = n;
        report.pValue = pValue;
        report.statistic = chiSquare;
        report.dof = static_cast<double>(kBlocks); // 8
        report.dist = usable ? RefDist::ChiSquareUpper : RefDist::None;
        report.stats = {{"m", static_cast<double>(m)},
                        {"M", static_cast<double>(M)},
                        {"N", static_cast<double>(kBlocks)},
                        {"mu", mu},
                        {"chi_sq", chiSquare}};
        return report;
    }

private:
    std::size_t m_;
};

} // namespace

std::unique_ptr<RandomnessTest> makeNonOverlapping(const TestParams &params)
{
    return std::make_unique<NonOverlappingTemplateTest>(params.blockSize);
}

} // namespace dft

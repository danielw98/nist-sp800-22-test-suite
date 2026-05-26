// Frequency within a Block Test - NIST SP 800-22 section 2.2.
// Tests whether the proportion of ones in each M-bit block is close to 1/2.
#include "basic_tests.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "special.hpp"

namespace dft {
namespace {

class BlockFrequencyTest : public RandomnessTest
{
public:
    explicit BlockFrequencyTest(std::size_t blockSize) : blockSize_(blockSize)
    {
    }
    std::string id() const override
    {
        return "block-frequency";
    }
    std::string name() const override
    {
        return "Frequency within a Block";
    }
    std::size_t minLength() const override
    {
        return 100;
    }

    TestReport run(const BitSequence &seq) const override
    {
        const std::vector<std::uint8_t> &bits = seq.bits();
        const std::size_t n = bits.size();
        const std::size_t M = blockSize_ > 0 ? blockSize_ : std::min<std::size_t>(128, n);
        const std::size_t N = M > 0 ? n / M : 0; // number of whole blocks

        double chiSquare = 0.0;
        for (std::size_t block = 0; block < N; block++)
        {
            std::size_t ones = 0;
            for (std::size_t j = 0; j < M; j++)
            {
                ones += bits[block * M + j];
            }
            const double pi = static_cast<double>(ones) / static_cast<double>(M);
            chiSquare += (pi - 0.5) * (pi - 0.5);
        }
        chiSquare *= 4.0 * static_cast<double>(M);

        TestReport report;
        report.n = n;
        report.pValue = N > 0 ? regularizedGammaQ(static_cast<double>(N) / 2.0, chiSquare / 2.0) : 0.0;
        report.statistic = chiSquare;
        report.dof = static_cast<double>(N);
        report.dist = N > 0 ? RefDist::ChiSquareUpper : RefDist::None;
        report.stats = {{"M", static_cast<double>(M)},
                        {"N", static_cast<double>(N)},
                        {"chi_sq", chiSquare}};
        return report;
    }

private:
    std::size_t blockSize_;
};

} // namespace

std::unique_ptr<RandomnessTest> makeBlockFrequency(const TestParams &params)
{
    return std::make_unique<BlockFrequencyTest>(params.blockSize);
}

} // namespace dft

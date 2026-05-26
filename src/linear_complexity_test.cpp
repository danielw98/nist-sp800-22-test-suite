// Linear Complexity Test - NIST SP 800-22 section 2.10.
// Splits the sequence into N blocks of M bits, finds each block's linear
// complexity (the length of the shortest LFSR that generates it) via the binary
// Berlekamp-Massey algorithm, and chi-square-tests the distribution of the
// deviation T of that length from its expected value.
#include "basic_tests.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

#include "special.hpp"

namespace dft {
namespace {

// Length of the shortest LFSR generating an M-bit block, by binary
// Berlekamp-Massey (NIST sec 2.10.4, sub-step 2).
int linearComplexity(const std::uint8_t *block, std::size_t M)
{
    std::vector<std::uint8_t> b(M, 0); // last c before the last length change
    std::vector<std::uint8_t> c(M, 0); // current connection polynomial
    std::vector<std::uint8_t> t(M, 0); // scratch copy of c
    b[0] = 1;
    c[0] = 1;
    int length = 0; // L
    int lastChange = -1; // m: position of the last length change
    for (std::size_t j = 0; j < M; j++)
    {
        int discrepancy = block[j];
        for (int k = 1; k <= length; k++)
        {
            discrepancy += c[static_cast<std::size_t>(k)] * block[j - static_cast<std::size_t>(k)];
        }
        if ((discrepancy & 1) == 1)
        {
            t = c;
            const std::size_t shift = j - static_cast<std::size_t>(lastChange);
            for (std::size_t k = shift; k < M; k++)
            {
                c[k] ^= b[k - shift];
            }
            if (length <= static_cast<int>(j) / 2)
            {
                length = static_cast<int>(j) + 1 - length;
                lastChange = static_cast<int>(j);
                b = t;
            }
        }
    }
    return length;
}

// Theoretical class probabilities (NIST sec 2.10.3 / the arcetri reference).
// NOTE: pi[0] is the reference-code value 0.01047; the SP 800-22 text prints
// 0.010417, but 0.01047 is what reproduces the published 2.10.8 example.
constexpr std::array<double, 7> kPi = {0.01047, 0.03125, 0.12500, 0.50000, 0.25000, 0.06250, 0.020833};

class LinearComplexityTest : public RandomnessTest
{
public:
    explicit LinearComplexityTest(std::size_t blockSize) : m_(blockSize > 0 ? blockSize : 500)
    {
    }
    std::string id() const override
    {
        return "linear-complexity";
    }
    std::string name() const override
    {
        return "Linear Complexity";
    }
    std::size_t minLength() const override
    {
        return 1000000;
    }

    TestReport run(const BitSequence &seq) const override
    {
        const std::vector<std::uint8_t> &bits = seq.bits();
        const std::size_t n = bits.size();
        const std::size_t M = m_;
        const std::size_t N = M > 0 ? n / M : 0;

        // Expected linear complexity of a random M-bit block (NIST sec 2.10.4),
        // with sign = (-1)^M shared by the mean correction and the T statistic.
        const double sign = (M % 2 == 0) ? 1.0 : -1.0;
        const double mu = M / 2.0 + (9.0 + sign) / 36.0 -
                          (M / 3.0 + 2.0 / 9.0) / std::pow(2.0, static_cast<double>(M));

        std::array<long, 7> v{};
        for (std::size_t i = 0; i < N; i++)
        {
            const int complexity = linearComplexity(&bits[i * M], M);
            const double tValue = sign * (static_cast<double>(complexity) - mu) + 2.0 / 9.0;
            v[classify(tValue)]++;
        }

        double chiSquare = 0.0;
        for (std::size_t i = 0; i < kPi.size(); i++)
        {
            const double expected = static_cast<double>(N) * kPi[i];
            const double diff = static_cast<double>(v[i]) - expected;
            chiSquare += expected > 0.0 ? diff * diff / expected : 0.0;
        }
        // 7 classes -> K = 6 degrees of freedom -> P = Q(K/2, chi^2/2).
        const double pValue = N > 0 ? regularizedGammaQ(3.0, chiSquare / 2.0) : 0.0;

        TestReport report;
        report.n = n;
        report.pValue = pValue;
        report.statistic = chiSquare;
        report.dof = 6.0; // 7 classes -> 6 degrees of freedom
        report.dist = N > 0 ? RefDist::ChiSquareUpper : RefDist::None;
        report.stats = {{"M", static_cast<double>(M)},
                        {"N", static_cast<double>(N)},
                        {"chi_sq", chiSquare}};
        return report;
    }

private:
    // Map the deviation T to one of 7 classes (NIST sec 2.10.4 step 5).
    static std::size_t classify(double tValue)
    {
        constexpr double edge = 2.5; // (K - 1) / 2 with K = 6
        if (tValue <= -edge)
        {
            return 0;
        }
        if (tValue > edge)
        {
            return 6;
        }
        return static_cast<std::size_t>(std::ceil(tValue + edge));
    }

    std::size_t m_;
};

} // namespace

std::unique_ptr<RandomnessTest> makeLinearComplexity(const TestParams &params)
{
    return std::make_unique<LinearComplexityTest>(params.blockSize);
}

} // namespace dft

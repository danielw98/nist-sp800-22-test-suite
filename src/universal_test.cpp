// Maurer's Universal Statistical Test - NIST SP 800-22 section 2.9.
// Measures the average log2 distance between repeated L-bit blocks. A random
// sequence is incompressible, so the statistic f_n concentrates near a known
// mean; a compressible (structured) sequence drives f_n away from it.
#include "basic_tests.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

namespace dft {
namespace {

// expected_value[L] (mu) and variance[L] for L = 6..16, from the Handbook of
// Applied Cryptography sec 5.4.5 (the same table the NIST suite uses). The
// entries for L < 6 are unused (the test requires L >= 6).
constexpr int kMaxL = 16;
constexpr std::array<double, kMaxL + 1> kExpected = {
    0, 0, 0, 0, 0, 0, 5.2177052, 6.1962507, 7.1836656, 8.1764248, 9.1723243,
    10.170032, 11.168765, 12.168070, 13.167693, 14.167488, 15.167379};
constexpr std::array<double, kMaxL + 1> kVariance = {
    0, 0, 0, 0, 0, 0, 2.954, 3.125, 3.238, 3.311, 3.356, 3.384, 3.401, 3.410, 3.416, 3.419, 3.421};

// Largest L in [6, 16] with n >= 1010 * 2^L * L; 0 if the sequence is too short.
int chooseL(std::size_t n)
{
    int chosen = 0;
    for (int L = 6; L <= kMaxL; L++)
    {
        const double need = 1010.0 * static_cast<double>(std::size_t(1) << L) * L;
        if (static_cast<double>(n) < need)
        {
            break;
        }
        chosen = L;
    }
    return chosen;
}

// Decimal value of the L-bit block starting at `start` (most significant first).
std::size_t blockValue(const std::vector<std::uint8_t> &bits, std::size_t start, int L)
{
    std::size_t value = 0;
    for (int j = 0; j < L; j++)
    {
        value = (value << 1) | bits[start + static_cast<std::size_t>(j)];
    }
    return value;
}

class UniversalTest : public RandomnessTest
{
public:
    std::string id() const override
    {
        return "universal";
    }
    std::string name() const override
    {
        return "Maurer's Universal Statistical";
    }
    std::size_t minLength() const override
    {
        return 387840; // smallest n admitting L = 6
    }

    TestReport run(const BitSequence &seq) const override
    {
        const std::vector<std::uint8_t> &bits = seq.bits();
        const std::size_t n = bits.size();
        const int L = chooseL(n);

        TestReport report;
        report.n = n;
        if (L < 6)
        {
            report.pValue = 0.0;
            report.stats = {{"L", static_cast<double>(L)}};
            return report;
        }

        const std::size_t blocks = n / static_cast<std::size_t>(L);
        const std::size_t Q = 10u * (std::size_t(1) << L); // initialization segment
        const std::size_t K = blocks - Q;                  // test segment (all remaining blocks)

        // Step 2: the initialization segment records each block's last position.
        std::vector<std::size_t> lastSeen(std::size_t(1) << L, 0);
        for (std::size_t i = 1; i <= Q; i++)
        {
            lastSeen[blockValue(bits, (i - 1) * static_cast<std::size_t>(L), L)] = i;
        }
        // Step 3: the test segment accumulates log2 gaps to the previous occurrence.
        double sum = 0.0;
        for (std::size_t i = Q + 1; i <= Q + K; i++)
        {
            const std::size_t value = blockValue(bits, (i - 1) * static_cast<std::size_t>(L), L);
            sum += std::log2(static_cast<double>(i - lastSeen[value]));
            lastSeen[value] = i;
        }

        const double fn = sum / static_cast<double>(K);
        // Step 5: normalize the statistic and turn it into a p-value (sec 2.9.4).
        const double c = 0.7 - 0.8 / L + (4.0 + 32.0 / L) * std::pow(static_cast<double>(K), -3.0 / L) / 15.0;
        const double sigma = c * std::sqrt(kVariance[static_cast<std::size_t>(L)] / static_cast<double>(K));
        const double arg = std::fabs(fn - kExpected[static_cast<std::size_t>(L)]) / (std::sqrt(2.0) * sigma);
        const double pValue = std::erfc(arg);

        report.pValue = pValue;
        report.statistic = (fn - kExpected[static_cast<std::size_t>(L)]) / sigma; // standardized f_n
        report.dist = RefDist::Normal2Sided;
        report.stats = {{"L", static_cast<double>(L)},
                        {"Q", static_cast<double>(Q)},
                        {"K", static_cast<double>(K)},
                        {"f_n", fn},
                        {"expected", kExpected[static_cast<std::size_t>(L)]}};
        return report;
    }
};

} // namespace

std::unique_ptr<RandomnessTest> makeUniversal(const TestParams &)
{
    return std::make_unique<UniversalTest>();
}

} // namespace dft

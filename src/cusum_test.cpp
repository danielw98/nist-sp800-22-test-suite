// Cumulative Sums (Cusum) Test - NIST SP 800-22 section 2.13.
// Looks at the maximal excursion of the running sum of +/-1 values. NIST defines
// a forward and a reverse variant; we report the forward p-value and carry the
// reverse one as a stat.
#include "basic_tests.hpp"

#include <cmath>
#include <cstdint>
#include <vector>

#include "special.hpp"

namespace dft {
namespace {

// Maximal |partial sum| of the +/-1 walk, scanning bits in the given direction.
long maxExcursion(const std::vector<std::uint8_t> &bits, bool reverse)
{
    long sum = 0;
    long best = 0;
    const std::size_t n = bits.size();
    for (std::size_t i = 0; i < n; i++)
    {
        const std::uint8_t bit = reverse ? bits[n - 1 - i] : bits[i];
        sum += bit ? 1 : -1;
        const long absSum = sum < 0 ? -sum : sum;
        best = absSum > best ? absSum : best;
    }
    return best;
}

// NIST sec 2.13.4 step 4 p-value from the maximal excursion z over n bits.
double cusumPValue(long z, std::size_t n)
{
    if (z == 0)
    {
        return 1.0;
    }
    const double zz = static_cast<double>(z);
    const double sqrtN = std::sqrt(static_cast<double>(n));
    const double upper = (static_cast<double>(n) / zz - 1.0) / 4.0;

    double sum1 = 0.0;
    for (long k = static_cast<long>(std::floor((-static_cast<double>(n) / zz + 1.0) / 4.0));
         k <= static_cast<long>(std::floor(upper)); k++)
    {
        sum1 += normalCdf(((4.0 * k + 1.0) * zz) / sqrtN) - normalCdf(((4.0 * k - 1.0) * zz) / sqrtN);
    }
    double sum2 = 0.0;
    for (long k = static_cast<long>(std::floor((-static_cast<double>(n) / zz - 3.0) / 4.0));
         k <= static_cast<long>(std::floor(upper)); k++)
    {
        sum2 += normalCdf(((4.0 * k + 3.0) * zz) / sqrtN) - normalCdf(((4.0 * k + 1.0) * zz) / sqrtN);
    }
    return 1.0 - sum1 + sum2;
}

class CusumTest : public RandomnessTest
{
public:
    std::string id() const override
    {
        return "cusum";
    }
    std::string name() const override
    {
        return "Cumulative Sums (Cusum)";
    }
    std::size_t minLength() const override
    {
        return 100;
    }

    TestReport run(const BitSequence &seq) const override
    {
        const std::vector<std::uint8_t> &bits = seq.bits();
        const std::size_t n = bits.size();

        const long zForward = maxExcursion(bits, false);
        const long zReverse = maxExcursion(bits, true);
        const double pForward = cusumPValue(zForward, n);
        const double pReverse = cusumPValue(zReverse, n);

        TestReport report;
        report.n = n;
        report.pValue = pForward;
        report.stats = {{"z_forward", static_cast<double>(zForward)},
                        {"z_reverse", static_cast<double>(zReverse)},
                        {"p_reverse", pReverse}};
        return report;
    }
};

} // namespace

std::unique_ptr<RandomnessTest> makeCusum(const TestParams &)
{
    return std::make_unique<CusumTest>();
}

} // namespace dft

// Serial Test - NIST SP 800-22 section 2.11.
// Looks at the frequency of every overlapping m-bit pattern; under randomness
// all 2^m patterns are equally likely. Reports two p-values (we keep the first
// as the headline and the second as a stat).
#include "basic_tests.hpp"

#include <cmath>
#include <cstdint>
#include <vector>

#include "special.hpp"

namespace dft {
namespace {

// psi^2_k over overlapping k-bit patterns with wraparound (NIST sec 2.11.4).
double psiSquared(const std::vector<std::uint8_t> &bits, std::size_t k)
{
    if (k == 0)
    {
        return 0.0;
    }
    const std::size_t n = bits.size();
    std::vector<long> counts(std::size_t(1) << k, 0);
    for (std::size_t i = 0; i < n; i++)
    {
        std::size_t pattern = 0;
        for (std::size_t j = 0; j < k; j++)
        {
            pattern = (pattern << 1) | bits[(i + j) % n];
        }
        counts[pattern]++;
    }
    double sumSquares = 0.0;
    for (long c : counts)
    {
        sumSquares += static_cast<double>(c) * static_cast<double>(c);
    }
    return (static_cast<double>(std::size_t(1) << k) / static_cast<double>(n)) * sumSquares -
           static_cast<double>(n);
}

class SerialTest : public RandomnessTest
{
public:
    explicit SerialTest(std::size_t blockSize) : m_(blockSize > 0 ? blockSize : 2)
    {
    }
    std::string id() const override
    {
        return "serial";
    }
    std::string name() const override
    {
        return "Serial";
    }
    std::size_t minLength() const override
    {
        return 100;
    }

    TestReport run(const BitSequence &seq) const override
    {
        const std::vector<std::uint8_t> &bits = seq.bits();

        const double psiM = psiSquared(bits, m_);
        const double psiM1 = psiSquared(bits, m_ >= 1 ? m_ - 1 : 0);
        const double psiM2 = psiSquared(bits, m_ >= 2 ? m_ - 2 : 0);
        const double delPsi = psiM - psiM1;             // first difference
        const double del2Psi = psiM - 2.0 * psiM1 + psiM2; // second difference

        const double p1 = regularizedGammaQ(std::pow(2.0, static_cast<double>(m_) - 2.0), delPsi / 2.0);
        const double p2 = regularizedGammaQ(std::pow(2.0, static_cast<double>(m_) - 3.0), del2Psi / 2.0);

        TestReport report;
        report.n = bits.size();
        report.pValue = p1;
        // The headline p1 uses Q(2^(m-2), del_psi/2), i.e. chi-square with 2^(m-1) dof.
        report.statistic = delPsi;
        report.dof = std::pow(2.0, static_cast<double>(m_) - 1.0);
        report.dist = RefDist::ChiSquareUpper;
        report.stats = {{"m", static_cast<double>(m_)},
                        {"psi_sq_m", psiM},
                        {"del_psi", delPsi},
                        {"del2_psi", del2Psi},
                        {"p_value2", p2}};
        return report;
    }

private:
    std::size_t m_;
};

} // namespace

std::unique_ptr<RandomnessTest> makeSerial(const TestParams &params)
{
    return std::make_unique<SerialTest>(params.blockSize);
}

} // namespace dft

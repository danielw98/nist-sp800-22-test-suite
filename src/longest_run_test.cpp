// Longest Run of Ones in a Block Test - NIST SP 800-22 section 2.4.
// Tallies, per M-bit block, the longest run of consecutive ones into K+1
// classes and chi-square-tests the histogram against the reference probabilities.
#include "basic_tests.hpp"

#include <cmath>
#include <cstdint>
#include <vector>

#include "special.hpp"

namespace dft {
namespace {

class LongestRunTest : public RandomnessTest
{
public:
    std::string id() const override
    {
        return "longest-run";
    }
    std::string name() const override
    {
        return "Longest Run of Ones in a Block";
    }
    std::size_t minLength() const override
    {
        return 128;
    }

    TestReport run(const BitSequence &seq) const override
    {
        const std::vector<std::uint8_t> &bits = seq.bits();
        const std::size_t n = bits.size();

        // Block size M and reference probabilities pi[] per NIST Table (sec 2.4).
        std::size_t M;
        std::vector<double> pi;
        if (n < 6272)
        {
            M = 8;
            pi = {0.2148, 0.3672, 0.2305, 0.1875}; // classes: <=1, 2, 3, >=4
        }
        else if (n < 750000)
        {
            M = 128;
            pi = {0.1174, 0.2430, 0.2493, 0.1752, 0.1027, 0.1124}; // <=4,5,6,7,8,>=9
        }
        else
        {
            M = 10000;
            pi = {0.0882, 0.2092, 0.2483, 0.1933, 0.1208, 0.0675, 0.0727}; // <=10..15,>=16
        }
        const std::size_t classes = pi.size(); // K + 1
        const std::size_t N = n / M;            // number of whole blocks

        std::vector<long> v(classes, 0);
        for (std::size_t block = 0; block < N; block++)
        {
            std::size_t longest = 0;
            std::size_t current = 0;
            for (std::size_t j = 0; j < M; j++)
            {
                if (bits[block * M + j])
                {
                    current++;
                    longest = current > longest ? current : longest;
                }
                else
                {
                    current = 0;
                }
            }
            v[classify(M, longest, classes)]++;
        }

        double chiSquare = 0.0;
        for (std::size_t i = 0; i < classes; i++)
        {
            const double expected = static_cast<double>(N) * pi[i];
            const double diff = static_cast<double>(v[i]) - expected;
            chiSquare += diff * diff / expected;
        }
        const double degreesOfFreedom = static_cast<double>(classes - 1); // K

        TestReport report;
        report.n = n;
        report.pValue = N > 0 ? regularizedGammaQ(degreesOfFreedom / 2.0, chiSquare / 2.0) : 0.0;
        report.statistic = chiSquare;
        report.dof = degreesOfFreedom;
        report.dist = N > 0 ? RefDist::ChiSquareUpper : RefDist::None;
        report.stats = {{"M", static_cast<double>(M)},
                        {"N", static_cast<double>(N)},
                        {"chi_sq", chiSquare}};
        return report;
    }

private:
    // Map a block's longest run to its class index [0, classes-1].
    static std::size_t classify(std::size_t M, std::size_t longest, std::size_t classes)
    {
        std::size_t lo; // first class is "longest <= lo"
        if (M == 8)
        {
            lo = 1;
        }
        else if (M == 128)
        {
            lo = 4;
        }
        else
        {
            lo = 10;
        }
        if (longest <= lo)
        {
            return 0;
        }
        if (longest >= lo + classes - 1)
        {
            return classes - 1;
        }
        return longest - lo;
    }
};

} // namespace

std::unique_ptr<RandomnessTest> makeLongestRun(const TestParams &)
{
    return std::make_unique<LongestRunTest>();
}

} // namespace dft

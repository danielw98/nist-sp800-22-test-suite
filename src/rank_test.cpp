// Binary Matrix Rank Test - NIST SP 800-22 section 2.5.
// Splits the sequence into disjoint 32x32 bit matrices, computes each matrix's
// rank over GF(2), and chi-square-tests the distribution of full-rank /
// one-below-full / lower-rank matrices against the reference probabilities.
#include "basic_tests.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <utility>
#include <vector>

namespace dft {
namespace {

constexpr int kRows = 32; // M: rows of one matrix
constexpr int kCols = 32; // Q: columns of one matrix

// GF(2) rank of a matrix whose rows are packed into uint32_t bitmasks (column j
// is bit j). Gauss-Jordan elimination with XOR; the rank is invariant to the
// column packing order, so the bit-position convention does not matter.
int gf2Rank(std::array<std::uint32_t, kRows> rows)
{
    int rank = 0;
    for (int col = 0; col < kCols && rank < kRows; col++)
    {
        const std::uint32_t mask = std::uint32_t(1) << col;
        int pivot = rank;
        while (pivot < kRows && (rows[static_cast<std::size_t>(pivot)] & mask) == 0)
        {
            pivot++;
        }
        if (pivot == kRows)
        {
            continue; // no pivot in this column
        }
        std::swap(rows[static_cast<std::size_t>(rank)], rows[static_cast<std::size_t>(pivot)]);
        for (int r = 0; r < kRows; r++)
        {
            if (r != rank && (rows[static_cast<std::size_t>(r)] & mask) != 0)
            {
                rows[static_cast<std::size_t>(r)] ^= rows[static_cast<std::size_t>(rank)];
            }
        }
        rank++;
    }
    return rank;
}

// P(rank = r) for an M x Q binary matrix over GF(2) (NIST sec 2.5.3).
double rankProbability(int r, int rows, int cols)
{
    double product = 1.0;
    for (int i = 0; i <= r - 1; i++)
    {
        product *= ((1.0 - std::pow(2.0, i - rows)) * (1.0 - std::pow(2.0, i - cols))) /
                   (1.0 - std::pow(2.0, i - r));
    }
    return std::pow(2.0, static_cast<double>(r) * (rows + cols - r) - rows * cols) * product;
}

double chiTerm(long observed, double expected)
{
    const double diff = static_cast<double>(observed) - expected;
    return expected > 0.0 ? diff * diff / expected : 0.0;
}

class RankTest : public RandomnessTest
{
public:
    std::string id() const override
    {
        return "rank";
    }
    std::string name() const override
    {
        return "Binary Matrix Rank";
    }
    std::size_t minLength() const override
    {
        return 38 * kRows * kCols; // NIST: at least 38 matrices (38912 bits)
    }

    TestReport run(const BitSequence &seq) const override
    {
        const std::vector<std::uint8_t> &bits = seq.bits();
        const std::size_t n = bits.size();
        const std::size_t bitsPerMatrix = static_cast<std::size_t>(kRows) * kCols;
        const std::size_t matrixCount = n / bitsPerMatrix;

        const double pFull = rankProbability(kRows, kRows, kCols);         // rank 32
        const double pOneBelow = rankProbability(kRows - 1, kRows, kCols); // rank 31
        const double pRemaining = 1.0 - pFull - pOneBelow;                 // rank <= 30

        long fFull = 0;
        long fOneBelow = 0;
        for (std::size_t m = 0; m < matrixCount; m++)
        {
            std::array<std::uint32_t, kRows> rows{};
            const std::size_t base = m * bitsPerMatrix;
            for (int i = 0; i < kRows; i++)
            {
                std::uint32_t row = 0;
                for (int j = 0; j < kCols; j++)
                {
                    if (bits[base + static_cast<std::size_t>(i) * kCols + static_cast<std::size_t>(j)])
                    {
                        row |= std::uint32_t(1) << j;
                    }
                }
                rows[static_cast<std::size_t>(i)] = row;
            }
            const int rank = gf2Rank(rows);
            if (rank == kRows)
            {
                fFull++;
            }
            else if (rank == kRows - 1)
            {
                fOneBelow++;
            }
        }
        const long fRemaining = static_cast<long>(matrixCount) - fFull - fOneBelow;

        const double count = static_cast<double>(matrixCount);
        const double chiSquare = chiTerm(fFull, count * pFull) + chiTerm(fOneBelow, count * pOneBelow) +
                                 chiTerm(fRemaining, count * pRemaining);
        // Three rank classes -> two degrees of freedom -> P = Q(1, chi^2/2) = exp(-chi^2/2).
        const double pValue = matrixCount > 0 ? std::exp(-chiSquare / 2.0) : 0.0;

        TestReport report;
        report.n = n;
        report.pValue = pValue;
        report.statistic = chiSquare;
        report.dof = 2.0; // three rank classes -> 2 degrees of freedom
        report.dist = matrixCount > 0 ? RefDist::ChiSquareUpper : RefDist::None;
        report.stats = {{"matrices", count},
                        {"F_32", static_cast<double>(fFull)},
                        {"F_31", static_cast<double>(fOneBelow)},
                        {"F_rem", static_cast<double>(fRemaining)},
                        {"chi_sq", chiSquare}};
        return report;
    }
};

} // namespace

std::unique_ptr<RandomnessTest> makeRank(const TestParams &)
{
    return std::make_unique<RankTest>();
}

} // namespace dft

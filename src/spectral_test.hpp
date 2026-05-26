// Discrete Fourier Transform (Spectral) Test, NIST SP 800-22 section 2.6.
// One member of the suite (id "dft"); it keeps a richer `analyze()` + the
// spectrum, which the referat and the interactive runner use.
#ifndef DFT_SPECTRAL_TEST_HPP
#define DFT_SPECTRAL_TEST_HPP

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <vector>

#include "bit_sequence.hpp"
#include "fft.hpp"
#include "test.hpp"

namespace dft {

// Detailed outcome of one run, mirroring the quantities the NIST reference
// reports (N_1, N_0, d, p_value).
struct SpectralResult
{
    std::size_t n = 0;      // bit stream length
    double threshold = 0.0; // T = sqrt(ln(20) * n)
    double n0 = 0.0;        // expected peaks below T = 0.95 * n / 2
    long n1 = 0;            // observed peaks below T
    double d = 0.0;         // (N1 - N0) / sqrt(n / 4 * 0.95 * 0.05)
    double pValue = 0.0;    // erfc(|d| / sqrt(2))
    bool passed = false;    // pValue >= alpha
};

// Write the computed quantities (n, T, N0, N1, d, p-value) as a human-readable
// block. The verdict is left to the caller, which knows the chosen alpha.
std::ostream &operator<<(std::ostream &os, const SpectralResult &r);

// Faithful to the reference: the peak heights are the moduli of S[0..n/2-1]
// (the DC term is included, the Nyquist term is excluded), compared against the
// 95 % peak-height threshold. No fudge factor.
class SpectralTest : public RandomnessTest
{
public:
    explicit SpectralTest(std::shared_ptr<const DftEngine> engine = std::make_shared<FftEngine>(),
                          double alpha = 0.01, bool wantSpectrum = false,
                          std::size_t spectrumPoints = 4000);

    std::string id() const override
    {
        return "dft";
    }
    std::string name() const override
    {
        return "Discrete Fourier Transform (Spectral)";
    }
    std::size_t minLength() const override
    {
        return 1000;
    }
    TestReport run(const BitSequence &seq) const override;

    // Full computation, exposing every intermediate statistic. If
    // outMagnitudes is non-null, it is filled with the first n/2 peak heights
    // |S[0..n/2-1]| (the same ones the test counts), computed in the same pass.
    SpectralResult analyze(const BitSequence &seq, std::vector<double> *outMagnitudes = nullptr) const;

    double alpha() const
    {
        return alpha_;
    }

private:
    std::shared_ptr<const DftEngine> engine_;
    double alpha_;
    bool wantSpectrum_;
    std::size_t spectrumPoints_;
};

} // namespace dft

#endif // DFT_SPECTRAL_TEST_HPP

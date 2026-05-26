#include "spectral_test.hpp"

#include <cmath>
#include <iomanip>
#include <ostream>
#include <stdexcept>

namespace dft {

namespace {
// NIST SP 800-22 sec 2.6 constants.
constexpr double kThresholdLog = 20.0; // T = sqrt(ln(20) * n)
constexpr double kPeakFraction = 0.95; // expected fraction of peaks below T
} // namespace

SpectralTest::SpectralTest(std::shared_ptr<const DftEngine> engine, double alpha, bool wantSpectrum,
                           std::size_t spectrumPoints)
    : engine_(std::move(engine)), alpha_(alpha), wantSpectrum_(wantSpectrum),
      spectrumPoints_(spectrumPoints)
{
    if (engine_ == nullptr)
    {
        throw std::invalid_argument("SpectralTest requires a non-null DFT engine");
    }
}

namespace {
// Downsample the peak heights to at most maxPoints by keeping the tallest peak
// in each bucket (so visible peaks are preserved), as {k, mag, above}.
std::vector<SpectrumPoint> downsampleSpectrum(const std::vector<double> &mags, double threshold,
                                              std::size_t maxPoints)
{
    std::vector<SpectrumPoint> out;
    const std::size_t half = mags.size();
    if (half == 0)
    {
        return out;
    }
    const bool downsample = half > maxPoints;
    const std::size_t points = downsample ? maxPoints : half;
    out.reserve(points);
    for (std::size_t p = 0; p < points; p++)
    {
        const std::size_t lo = downsample ? (p * half) / points : p;
        const std::size_t hi = downsample ? ((p + 1) * half) / points : p + 1;
        std::size_t bestK = lo;
        double bestMag = mags[lo];
        for (std::size_t i = lo + 1; i < hi; i++)
        {
            if (mags[i] > bestMag)
            {
                bestMag = mags[i];
                bestK = i;
            }
        }
        out.push_back({bestK, bestMag, bestMag >= threshold});
    }
    return out;
}
} // namespace

TestReport SpectralTest::run(const BitSequence &seq) const
{
    std::vector<double> mags;
    const SpectralResult r = analyze(seq, wantSpectrum_ ? &mags : nullptr);

    TestReport report;
    report.n = r.n;
    report.pValue = r.pValue;
    report.statistic = r.d; // d = (N1 - N0)/sigma; reject if |d| > z_{1-alpha/2}
    report.dist = RefDist::Normal2Sided;
    report.stats = {{"threshold", r.threshold},
                    {"N0", r.n0},
                    {"N1", static_cast<double>(r.n1)},
                    {"d", r.d}};
    if (wantSpectrum_)
    {
        report.spectrum = downsampleSpectrum(mags, r.threshold, spectrumPoints_);
    }
    return report;
}

SpectralResult SpectralTest::analyze(const BitSequence &seq, std::vector<double> *outMagnitudes) const
{
    const std::size_t n = seq.size();
    if (n < 2)
    {
        throw std::invalid_argument("spectral test requires at least 2 bits");
    }
    const double length = static_cast<double>(n);

    // Steps 1-2: map bits to +/-1 and take the DFT.
    const std::vector<Complex> spectrum = engine_->transform(seq.toBipolar());

    // Step 3: peak heights are the moduli of the first n/2 components, i.e. the
    // DC term S[0] up to (but not including) the Nyquist term S[n/2].
    const std::size_t half = n / 2;

    // Step 4: 95 % peak-height threshold T = sqrt(ln(20) * n).
    const double threshold = std::sqrt(std::log(kThresholdLog) * length);

    if (outMagnitudes != nullptr)
    {
        outMagnitudes->resize(half);
    }

    // Step 6: count the peaks that fall below the threshold.
    long peaksBelow = 0;
    for (std::size_t i = 0; i < half; i++)
    {
        const double magnitude = std::abs(spectrum[i]);
        if (outMagnitudes != nullptr)
        {
            (*outMagnitudes)[i] = magnitude;
        }
        if (magnitude < threshold)
        {
            peaksBelow++;
        }
    }

    SpectralResult result;
    result.n = n;
    result.threshold = threshold;
    result.n0 = kPeakFraction * length / 2.0; // Step 5
    result.n1 = peaksBelow;

    // Step 7: normalise the deviation. The reference variance is n/4 * 0.95 * 0.05.
    const double deviationSd = std::sqrt(length / 4.0 * kPeakFraction * (1.0 - kPeakFraction));
    result.d = (static_cast<double>(peaksBelow) - result.n0) / deviationSd;
    result.pValue = std::erfc(std::fabs(result.d) / std::sqrt(2.0)); // Step 8
    result.passed = result.pValue >= alpha_;
    return result;
}

std::ostream &operator<<(std::ostream &os, const SpectralResult &result)
{
    const std::ios::fmtflags flags = os.flags();
    const std::streamsize precision = os.precision();
    os << std::fixed << std::setprecision(6) << "n         = " << result.n << "\n"
       << "T         = " << result.threshold << "\n"
       << "N0        = " << result.n0 << "\n"
       << "N1        = " << result.n1 << "\n"
       << "d         = " << result.d << "\n"
       << "p-value   = " << result.pValue << "\n";
    os.flags(flags);
    os.precision(precision);
    return os;
}

} // namespace dft

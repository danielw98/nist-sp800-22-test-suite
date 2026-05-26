#include "assessment.hpp"

#include <cmath>

#include "special.hpp"

namespace dft {

Assessment::Assessment(const RandomnessTest &test, double alpha, int uniformityBins,
                       double uniformityLevel)
    : test_(test), alpha_(alpha), bins_(uniformityBins), uniformityLevel_(uniformityLevel)
{
}

AssessmentResult Assessment::evaluate(const std::vector<BitSequence> &streams) const
{
    AssessmentResult result;
    result.bins.assign(static_cast<std::size_t>(bins_), 0);
    result.pValues.reserve(streams.size());

    for (const BitSequence &stream : streams)
    {
        const double pValue = test_.pValue(stream);
        result.pValues.push_back(pValue);
        result.sampleCount++;
        if (pValue >= alpha_)
        {
            result.passCount++;
        }

        int binIndex;
        if (pValue >= 1.0)
        {
            binIndex = bins_ - 1;
        }
        else if (pValue <= 0.0)
        {
            binIndex = 0;
        }
        else
        {
            binIndex = static_cast<int>(std::floor(pValue * bins_));
        }
        result.bins[static_cast<std::size_t>(binIndex)]++;
    }

    if (result.sampleCount > 0)
    {
        const double samples = static_cast<double>(result.sampleCount);
        const double binCount = static_cast<double>(bins_);
        result.proportion = static_cast<double>(result.passCount) / samples;

        // NIST proportion acceptance interval: p_hat +/- 3 * sqrt(p_hat * alpha / s).
        const double pHat = 1.0 - alpha_;
        const double margin = 3.0 * std::sqrt(pHat * alpha_ / samples);
        result.proportionMin = pHat - margin;
        result.proportionMax = pHat + margin;
        result.proportionPassed =
            result.proportion >= result.proportionMin && result.proportion <= result.proportionMax;

        // Uniformity: chi-square over the p-value histogram, turned into a
        // p-value by the regularized upper incomplete gamma function.
        const double expected = samples / binCount;
        double chiSquare = 0.0;
        for (long count : result.bins)
        {
            const double diff = static_cast<double>(count) - expected;
            chiSquare += diff * diff / expected;
        }
        result.uniformity = regularizedGammaQ((bins_ - 1) / 2.0, chiSquare / 2.0);
        result.uniformityPassed = result.uniformity >= uniformityLevel_;
    }
    return result;
}

} // namespace dft

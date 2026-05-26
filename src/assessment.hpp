// Second-level analysis over many bitstreams: the proportion of streams that
// pass and the uniformity of their p-values, exactly as NIST's
// finalAnalysisReport.txt reports them. Works on any RandomnessTest.
#ifndef DFT_ASSESSMENT_HPP
#define DFT_ASSESSMENT_HPP

#include <vector>

#include "bit_sequence.hpp"
#include "test.hpp"

namespace dft {

struct AssessmentResult
{
    long sampleCount = 0;
    long passCount = 0;
    double proportion = 0.0;    // passCount / sampleCount
    double proportionMin = 0.0; // NIST acceptance interval (lower bound)
    double proportionMax = 0.0; // NIST acceptance interval (upper bound)
    bool proportionPassed = false;
    double uniformity = 0.0; // chi-square p-value of the p-value histogram
    bool uniformityPassed = false;
    std::vector<long> bins;      // p-value histogram (one bucket per interval)
    std::vector<double> pValues; // one p-value per stream
};

class Assessment
{
public:
    explicit Assessment(const RandomnessTest &test, double alpha = 0.01, int uniformityBins = 10,
                        double uniformityLevel = 0.0001);

    AssessmentResult evaluate(const std::vector<BitSequence> &streams) const;

private:
    const RandomnessTest &test_;
    double alpha_;
    int bins_;
    double uniformityLevel_;
};

} // namespace dft

#endif // DFT_ASSESSMENT_HPP

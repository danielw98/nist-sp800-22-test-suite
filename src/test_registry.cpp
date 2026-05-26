#include "test_registry.hpp"

#include <functional>

#include "basic_tests.hpp"
#include "engine_factory.hpp"
#include "spectral_test.hpp"

namespace dft {
namespace {

struct Entry
{
    std::string id;
    std::string name;
    std::size_t minLength;
    std::function<std::unique_ptr<RandomnessTest>(const TestParams &)> factory;
};

std::unique_ptr<RandomnessTest> makeDft(const TestParams &params)
{
    return std::make_unique<SpectralTest>(makeEngine(params.method), params.alpha, params.spectrum,
                                          params.spectrumPoints);
}

const std::vector<Entry> &entries()
{
    static const std::vector<Entry> table = {
        {"monobit", "Frequency (Monobit)", 100, makeMonobit},
        {"block-frequency", "Frequency within a Block", 100, makeBlockFrequency},
        {"runs", "Runs", 100, makeRuns},
        {"longest-run", "Longest Run of Ones in a Block", 128, makeLongestRun},
        {"rank", "Binary Matrix Rank", 38912, makeRank},
        {"dft", "Discrete Fourier Transform (Spectral)", 1000, makeDft},
        {"non-overlapping", "Non-overlapping Template Matching", 1000000, makeNonOverlapping},
        {"overlapping", "Overlapping Template Matching", 1000000, makeOverlapping},
        {"universal", "Maurer's Universal Statistical", 387840, makeUniversal},
        {"linear-complexity", "Linear Complexity", 1000000, makeLinearComplexity},
        {"serial", "Serial", 100, makeSerial},
        {"approx-entropy", "Approximate Entropy", 100, makeApproxEntropy},
        {"cusum", "Cumulative Sums (Cusum)", 100, makeCusum},
        {"random-excursions", "Random Excursions", 1000000, makeRandomExcursions},
        {"random-excursions-variant", "Random Excursions Variant", 1000000, makeRandomExcursionsVariant},
    };
    return table;
}

} // namespace

std::vector<TestInfo> listTests()
{
    std::vector<TestInfo> out;
    out.reserve(entries().size());
    for (const Entry &e : entries())
    {
        out.push_back({e.id, e.name, e.minLength});
    }
    return out;
}

std::unique_ptr<RandomnessTest> makeTest(const std::string &id, const TestParams &params)
{
    for (const Entry &e : entries())
    {
        if (e.id == id)
        {
            return e.factory(params);
        }
    }
    return nullptr;
}

} // namespace dft

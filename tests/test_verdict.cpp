// The critical-value verdict (statistic vs table critical value) must agree with
// the p-value verdict (p < alpha) for every test and every alpha - they are two
// framings of the same decision. This pins that every test sets its statistic,
// reference distribution and dof correctly.
#include "doctest.h"

#include <memory>
#include <string>

#include "bit_sequence.hpp"
#include "test_params.hpp"
#include "test_registry.hpp"
#include "verdict.hpp"

using namespace dft;

namespace {
const char *kExample100 =
    "11001001000011111101101010100010001000010110100011"
    "00001000110100110001001100011001100010100010111000";

void checkConsistent(const std::string &id, const BitSequence &seq, std::size_t blockSize = 0)
{
    TestParams params;
    params.blockSize = blockSize;
    const std::unique_ptr<RandomnessTest> test = makeTest(id, params);
    REQUIRE(test != nullptr);
    const TestReport report = test->run(seq);
    for (double alpha : {0.01, 0.05, 0.1})
    {
        CAPTURE(id);
        CAPTURE(alpha);
        const CriticalVerdict v = criticalVerdict(report, alpha);
        if (v.hasCritical)
        {
            // statistic-vs-critical-value must match p-value-vs-alpha exactly
            CHECK(v.reject == (report.pValue < alpha));
        }
    }
}
} // namespace

TEST_CASE("critical-value verdict agrees with p-value verdict (small-n tests)")
{
    const BitSequence ex = BitSequence::fromText(kExample100);
    checkConsistent("monobit", ex);
    checkConsistent("runs", ex);
    checkConsistent("block-frequency", ex, 10);
    checkConsistent("serial", ex, 2);
    checkConsistent("approx-entropy", ex, 2);
    checkConsistent("cusum", ex); // dist == None -> p-value fallback; no critical assertion
    checkConsistent("dft", ex);
}

#ifdef DFT_DATA_DIR
TEST_CASE("critical-value verdict agrees with p-value verdict (data-file tests)")
{
    const BitSequence e = BitSequence::fromFileAuto(std::string(DFT_DATA_DIR) + "/data.e");
    REQUIRE(e.size() >= 1000000);
    const BitSequence m1 = e.split(1000000, 1).at(0);
    checkConsistent("rank", m1);
    checkConsistent("universal", m1);
    checkConsistent("overlapping", m1);
    checkConsistent("non-overlapping", m1);
    checkConsistent("linear-complexity", m1, 1000);
    checkConsistent("random-excursions", m1);
    checkConsistent("random-excursions-variant", m1);
}
#endif

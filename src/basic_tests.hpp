// Factories for the time-domain SP 800-22 tests in the first batch. Each test
// lives in its own .cpp (the class is file-local); the registry builds them
// through these factories. Adding a test = one .cpp + one line here + one line
// in test_registry.cpp.
#ifndef DFT_BASIC_TESTS_HPP
#define DFT_BASIC_TESTS_HPP

#include <memory>

#include "test.hpp"
#include "test_params.hpp"

namespace dft {

std::unique_ptr<RandomnessTest> makeMonobit(const TestParams &params);
std::unique_ptr<RandomnessTest> makeBlockFrequency(const TestParams &params);
std::unique_ptr<RandomnessTest> makeRuns(const TestParams &params);
std::unique_ptr<RandomnessTest> makeLongestRun(const TestParams &params);
std::unique_ptr<RandomnessTest> makeRank(const TestParams &params);
std::unique_ptr<RandomnessTest> makeNonOverlapping(const TestParams &params);
std::unique_ptr<RandomnessTest> makeOverlapping(const TestParams &params);
std::unique_ptr<RandomnessTest> makeUniversal(const TestParams &params);
std::unique_ptr<RandomnessTest> makeLinearComplexity(const TestParams &params);
std::unique_ptr<RandomnessTest> makeRandomExcursions(const TestParams &params);
std::unique_ptr<RandomnessTest> makeRandomExcursionsVariant(const TestParams &params);
std::unique_ptr<RandomnessTest> makeCusum(const TestParams &params);
std::unique_ptr<RandomnessTest> makeSerial(const TestParams &params);
std::unique_ptr<RandomnessTest> makeApproxEntropy(const TestParams &params);

} // namespace dft

#endif // DFT_BASIC_TESTS_HPP

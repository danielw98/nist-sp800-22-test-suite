// Second-level analysis (proportion + uniformity) over many bitstreams.
#include "doctest.h"

#include <cstdint>
#include <random>
#include <string>
#include <vector>

#include "assessment.hpp"
#include "bit_sequence.hpp"
#include "spectral_test.hpp"

using namespace dft;

namespace {
std::string randomBits(std::size_t n, std::uint32_t seed)
{
    std::mt19937 rng(seed);
    std::string s(n, '0');
    for (char &ch : s)
    {
        ch = (rng() & 1U) ? '1' : '0';
    }
    return s;
}
} // namespace

TEST_CASE("good random streams pass and the bookkeeping is consistent")
{
    const BitSequence seq = BitSequence::fromText(randomBits(500000, 2024));
    const std::vector<BitSequence> streams = seq.split(10000);
    REQUIRE(streams.size() == 50);

    const SpectralTest test;
    const Assessment assessment(test);
    const AssessmentResult result = assessment.evaluate(streams);

    CHECK(result.sampleCount == 50);
    CHECK(result.pValues.size() == 50u);
    long binSum = 0;
    for (long count : result.bins)
    {
        binSum += count;
    }
    CHECK(binSum == result.sampleCount); // every stream lands in exactly one bin
    CHECK(result.proportion > 0.9);      // a sound generator clears the NIST bar
    CHECK(result.proportionPassed);
}

TEST_CASE("a periodic generator is rejected by the proportion test")
{
    std::string periodic;
    for (int i = 0; i < 50000; i++)
    {
        periodic += "0001"; // 200000 bits, period 4
    }
    const std::vector<BitSequence> streams = BitSequence::fromText(periodic).split(10000);
    const SpectralTest test;
    const Assessment assessment(test);
    const AssessmentResult result = assessment.evaluate(streams);

    CHECK(result.passCount == 0);
    CHECK_FALSE(result.proportionPassed);
}

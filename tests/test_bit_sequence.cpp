// BitSequence: parsing, the bipolar mapping, and stream splitting.
#include "doctest.h"

#include <string>
#include <vector>

#include "bit_sequence.hpp"

using namespace dft;

TEST_CASE("fromText keeps only 0/1 and ignores everything else")
{
    const BitSequence seq = BitSequence::fromText("01 0\n1\t1xx0");
    CHECK(seq.size() == 6);
    CHECK_FALSE(seq.empty());
}

TEST_CASE("an empty or all-noise string yields no bits")
{
    CHECK(BitSequence::fromText("").empty());
    CHECK(BitSequence::fromText("hello world").empty());
}

TEST_CASE("toBipolar maps 1 -> +1 and 0 -> -1 on the real axis")
{
    const std::vector<Complex> x = BitSequence::fromText("10").toBipolar();
    REQUIRE(x.size() == 2);
    CHECK(x[0].real() == doctest::Approx(1.0));
    CHECK(x[1].real() == doctest::Approx(-1.0));
    CHECK(x[0].imag() == doctest::Approx(0.0));
}

TEST_CASE("split drops the trailing partial block and honours maxStreams")
{
    const BitSequence seq = BitSequence::fromText(std::string(25, '1'));
    SUBCASE("whole blocks only")
    {
        const std::vector<BitSequence> streams = seq.split(10);
        CHECK(streams.size() == 2); // 25 / 10 = 2, the last 5 bits are dropped
        CHECK(streams[0].size() == 10);
    }
    SUBCASE("maxStreams caps the count")
    {
        CHECK(seq.split(10, 1).size() == 1);
    }
    SUBCASE("length 0 yields nothing")
    {
        CHECK(seq.split(0).empty());
    }
}

TEST_CASE("fromArg treats a non-path token as a literal bitstring")
{
    CHECK(BitSequence::fromArg("0101").size() == 4);
}

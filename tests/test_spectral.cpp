// DFT spectral-test mechanics: the formulas, the engines agreeing, and the
// behaviour on pathological inputs (edge cases).
#include "doctest.h"

#include <cmath>
#include <memory>
#include <random>
#include <string>

#include "bit_sequence.hpp"
#include "fft.hpp"
#include "spectral_test.hpp"

using namespace dft;

namespace {
BitSequence bits(const std::string &s)
{
    return BitSequence::fromText(s);
}

std::string randomBits(std::size_t n, std::uint32_t seed)
{
    std::mt19937 rng(seed);
    std::string s(n, '0');
    for (auto &ch : s)
    {
        ch = (rng() & 1U) ? '1' : '0';
    }
    return s;
}
} // namespace

TEST_CASE("threshold and N0 follow the NIST formulas")
{
    const std::size_t n = 4096;
    const SpectralResult r = SpectralTest().analyze(bits(std::string(n, '1')));
    CHECK(r.n == n);
    CHECK(r.threshold == doctest::Approx(std::sqrt(std::log(20.0) * n)));
    CHECK(r.n0 == doctest::Approx(0.95 * n / 2.0));
}

TEST_CASE("pathological sequences are rejected")
{
    const SpectralTest test;
    SUBCASE("all ones (DC dominates)")
    {
        CHECK_FALSE(test.analyze(bits(std::string(4096, '1'))).passed);
    }
    SUBCASE("all zeros")
    {
        CHECK_FALSE(test.analyze(bits(std::string(4096, '0'))).passed);
    }
    SUBCASE("periodic 0001 (in-band peak)")
    {
        std::string s;
        for (int i = 0; i < 1024; i++)
        {
            s += "0001";
        }
        CHECK_FALSE(test.analyze(bits(s)).passed);
    }
    SUBCASE("alternating period-2 (peak on excluded Nyquist)")
    {
        std::string s;
        for (int i = 0; i < 2048; i++)
        {
            s += "01";
        }
        const SpectralResult r = test.analyze(bits(s));
        CHECK_FALSE(r.passed);   // rejected indirectly
        CHECK(r.n1 == 2048);     // all counted bins fall below T
    }
}

TEST_CASE("a good random sequence is not rejected")
{
    CHECK(SpectralTest().analyze(bits(randomBits(8192, 12345))).passed);
}

TEST_CASE("the FFT and direct engines give identical verdicts")
{
    const std::string s = randomBits(1024, 777);
    const SpectralResult fast = SpectralTest(std::make_shared<FftEngine>()).analyze(bits(s));
    const SpectralResult slow = SpectralTest(std::make_shared<DirectDftEngine>()).analyze(bits(s));
    CHECK(fast.n1 == slow.n1);
    CHECK(fast.pValue == doctest::Approx(slow.pValue));
}

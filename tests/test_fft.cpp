// FFT engine correctness: the fast engine (radix-2 + Bluestein) must agree with
// the naive O(n^2) DFT, including for non-power-of-two lengths (Bluestein path).
#include "doctest.h"

#include <complex>
#include <random>
#include <vector>

#include "fft.hpp"

using dft::Complex;

namespace {
std::vector<Complex> randomComplex(std::size_t n, std::uint32_t seed)
{
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    std::vector<Complex> v(n);
    for (auto &z : v)
    {
        z = Complex(dist(rng), dist(rng));
    }
    return v;
}

double maxError(const std::vector<Complex> &a, const std::vector<Complex> &b)
{
    double e = 0.0;
    for (std::size_t i = 0; i < a.size(); i++)
    {
        e = std::max(e, std::abs(a[i] - b[i]));
    }
    return e;
}
} // namespace

TEST_CASE("FftEngine matches DirectDftEngine across lengths")
{
    const dft::FftEngine fast;
    const dft::DirectDftEngine slow;
    for (std::size_t n : {2u, 8u, 10u, 64u, 100u, 257u, 1000u})
    {
        CAPTURE(n);
        const auto in = randomComplex(n, static_cast<std::uint32_t>(0x9E37 ^ n));
        CHECK(maxError(fast.transform(in), slow.transform(in)) < 1e-9);
    }
}

TEST_CASE("FFT of a constant signal has all energy in the DC bin")
{
    const dft::FftEngine fft;
    std::vector<Complex> ones(64, Complex(1.0, 0.0));
    const auto s = fft.transform(ones);
    CHECK(std::abs(s[0]) == doctest::Approx(64.0));
    for (std::size_t k = 1; k < s.size(); k++)
    {
        CHECK(std::abs(s[k]) < 1e-9);
    }
}

TEST_CASE("empty and single-element transforms are well defined")
{
    const dft::FftEngine fft;
    CHECK(fft.transform({}).empty());
    const auto one = fft.transform({Complex(3.0, -2.0)});
    REQUIRE(one.size() == 1);
    CHECK(std::abs(one[0] - Complex(3.0, -2.0)) < 1e-12);
}

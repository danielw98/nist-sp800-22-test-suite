// NIST SP 800-22 compliance: the implementation must reproduce the spec's
// reference values and decision rule for the section 2.6.8 example.
#include "doctest.h"

#include <memory>

#include "bit_sequence.hpp"
#include "fft.hpp"
#include "spectral_test.hpp"

using namespace dft;

// The 100-bit example sequence from NIST SP 800-22 section 2.6.8.
static const char *kNistExample =
    "11001001000011111101101010100010001000010110100011"
    "00001000110100110001001100011001100010100010111000";

TEST_CASE("sec 2.6.8 example matches the NIST reference code")
{
    const SpectralResult r = SpectralTest().analyze(BitSequence::fromText(kNistExample));
    CHECK(r.n == 100);
    // The reference code (and a correct DFT) give 48; the document misprints 46.
    CHECK(r.n1 == 48);
    CHECK(r.n0 == doctest::Approx(47.5));
    CHECK(r.d == doctest::Approx(0.458831).epsilon(1e-4));
    CHECK(r.pValue == doctest::Approx(0.646355).epsilon(1e-5));
    CHECK(r.passed); // p >= 0.01 -> accept as random
}

TEST_CASE("decision rule: passed iff p-value >= alpha")
{
    const BitSequence seq = BitSequence::fromText(kNistExample); // p approx 0.6464
    CHECK(SpectralTest(std::make_shared<FftEngine>(), 0.01).analyze(seq).passed);
    CHECK_FALSE(SpectralTest(std::make_shared<FftEngine>(), 0.70).analyze(seq).passed);
}

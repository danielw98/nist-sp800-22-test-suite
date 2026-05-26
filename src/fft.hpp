// Discrete Fourier transform engines.
//
// A small strategy hierarchy so callers can swap transform back ends without
// touching the test logic (mirroring the NIST suite's LEGACY_FFT / FFTW switch):
//
//   DftEngine        - abstract forward-DFT interface
//   FftEngine        - O(n log n): radix-2 Cooley-Tukey, generalised to any n
//                      by Bluestein's chirp-z algorithm
//   DirectDftEngine  - naive O(n^2), used to cross-check the fast engine
//
// Sign convention (matches the NIST reference):
//   X[k] = sum_j x[j] * exp(-2*pi*i*j*k/n).
#ifndef DFT_FFT_HPP
#define DFT_FFT_HPP

#include <vector>

#include "complex.hpp"

namespace dft {

class DftEngine
{
public:
    virtual ~DftEngine() = default;

    // Forward DFT of a complex sequence; returns the full spectrum.
    virtual std::vector<Complex> transform(const std::vector<Complex> &in) const = 0;
};

class FftEngine : public DftEngine
{
public:
    std::vector<Complex> transform(const std::vector<Complex> &in) const override;

private:
    // In-place iterative radix-2 FFT; a.size() must be a power of two.
    static void radix2(std::vector<Complex> &a, bool invert);

    // Arbitrary-length DFT via Bluestein's chirp-z algorithm.
    static std::vector<Complex> bluestein(const std::vector<Complex> &in);
};

class DirectDftEngine : public DftEngine
{
public:
    std::vector<Complex> transform(const std::vector<Complex> &in) const override;
};

} // namespace dft

#endif // DFT_FFT_HPP

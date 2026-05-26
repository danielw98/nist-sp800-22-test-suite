#include "fft.hpp"

#include <cmath>

namespace dft {

namespace {
constexpr double kPi = 3.14159265358979323846;
} // namespace

void FftEngine::radix2(std::vector<Complex> &a, bool invert)
{
    const std::size_t n = a.size();
    if (n <= 1)
    {
        return;
    }

    // Bit-reversal permutation.
    for (std::size_t i = 1, j = 0; i < n; i++)
    {
        std::size_t bit = n >> 1;
        for (; j & bit; bit >>= 1)
        {
            j ^= bit;
        }
        j ^= bit;
        if (i < j)
        {
            std::swap(a[i], a[j]);
        }
    }

    // Cooley-Tukey butterflies.
    for (std::size_t len = 2; len <= n; len <<= 1)
    {
        const double ang = 2.0 * kPi / static_cast<double>(len) * (invert ? 1.0 : -1.0);
        const Complex wlen(std::cos(ang), std::sin(ang));
        for (std::size_t i = 0; i < n; i += len)
        {
            Complex w(1.0, 0.0);
            for (std::size_t k = 0; k < len / 2; k++)
            {
                const Complex u = a[i + k];
                const Complex v = a[i + k + len / 2] * w;
                a[i + k] = u + v;
                a[i + k + len / 2] = u - v;
                w *= wlen;
            }
        }
    }

    if (invert)
    {
        const double inv_n = 1.0 / static_cast<double>(n);
        for (Complex &x : a)
        {
            x *= inv_n;
        }
    }
}

std::vector<Complex> FftEngine::bluestein(const std::vector<Complex> &in)
{
    const std::size_t n = in.size();

    // chirp[j] = exp(-i * pi * j^2 / n). The argument j*j is reduced modulo 2n
    // before scaling so the angle stays accurate even for n near a million.
    std::vector<Complex> chirp(n);
    std::vector<Complex> a(n);
    for (std::size_t j = 0; j < n; j++)
    {
        const unsigned long long jj = static_cast<unsigned long long>(j) * j;
        const double ang = -kPi * static_cast<double>(jj % (2ULL * n)) / static_cast<double>(n);
        chirp[j] = Complex(std::cos(ang), std::sin(ang));
        a[j] = in[j] * chirp[j];
    }

    // Pad to a power of two >= 2n - 1 so the cyclic convolution is linear.
    std::size_t m = 1;
    while (m < 2 * n - 1)
    {
        m <<= 1;
    }

    std::vector<Complex> fa(m, Complex(0.0, 0.0));
    std::vector<Complex> fb(m, Complex(0.0, 0.0));
    for (std::size_t j = 0; j < n; j++)
    {
        fa[j] = a[j];
    }

    // b is the symmetric kernel conj(chirp); mirror copies cover negative lags.
    fb[0] = std::conj(chirp[0]);
    for (std::size_t j = 1; j < n; j++)
    {
        const Complex v = std::conj(chirp[j]);
        fb[j] = v;
        fb[m - j] = v;
    }

    radix2(fa, false);
    radix2(fb, false);
    for (std::size_t i = 0; i < m; i++)
    {
        fa[i] *= fb[i];
    }
    radix2(fa, true);

    std::vector<Complex> out(n);
    for (std::size_t k = 0; k < n; k++)
    {
        out[k] = fa[k] * chirp[k];
    }
    return out;
}

std::vector<Complex> FftEngine::transform(const std::vector<Complex> &in) const
{
    const std::size_t n = in.size();
    if (n == 0)
    {
        return {};
    }
    if (n == 1)
    {
        return {in[0]};
    }

    // Power-of-two fast path; otherwise fall back to Bluestein.
    if ((n & (n - 1)) == 0)
    {
        std::vector<Complex> a = in;
        radix2(a, false);
        return a;
    }
    return bluestein(in);
}

std::vector<Complex> DirectDftEngine::transform(const std::vector<Complex> &in) const
{
    const std::size_t n = in.size();
    const double dn = static_cast<double>(n);
    std::vector<Complex> out(n);
    for (std::size_t k = 0; k < n; k++)
    {
        Complex sum(0.0, 0.0);
        for (std::size_t j = 0; j < n; j++)
        {
            const double angle = -2.0 * kPi * static_cast<double>(j) * static_cast<double>(k) / dn;
            sum += in[j] * Complex(std::cos(angle), std::sin(angle));
        }
        out[k] = sum;
    }
    return out;
}

} // namespace dft

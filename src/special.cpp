#include "special.hpp"

#include <cmath>

namespace dft {

namespace {
constexpr double kBig = 4.503599627370496e15;
constexpr double kBigInv = 2.22044604925031308085e-16;
constexpr double kMachep = 1.11022302462515654042e-16;
constexpr double kMaxLog = 7.09782712893383996732e2;

// Regularized lower incomplete gamma P(a, x) via its power series.
double lowerSeries(double a, double x)
{
    if (x <= 0.0 || a <= 0.0)
    {
        return 0.0;
    }

    double ax = a * std::log(x) - x - std::lgamma(a);
    if (ax < -kMaxLog)
    {
        return 0.0;
    }
    ax = std::exp(ax);

    double r = a;
    double c = 1.0;
    double ans = 1.0;
    do
    {
        r += 1.0;
        c *= x / r;
        ans += c;
    } while (c / ans > kMachep);

    return ans * ax / a;
}

// Regularized upper incomplete gamma Q(a, x) via a continued fraction.
double upperFraction(double a, double x)
{
    double ax = a * std::log(x) - x - std::lgamma(a);
    if (ax < -kMaxLog)
    {
        return 0.0;
    }
    ax = std::exp(ax);

    double y = 1.0 - a;
    double z = x + y + 1.0;
    double c = 0.0;
    double pkm2 = 1.0;
    double qkm2 = x;
    double pkm1 = x + 1.0;
    double qkm1 = z * x;
    double ans = pkm1 / qkm1;
    double t;
    do
    {
        c += 1.0;
        y += 1.0;
        z += 2.0;
        const double yc = y * c;
        const double pk = pkm1 * z - pkm2 * yc;
        const double qk = qkm1 * z - qkm2 * yc;
        if (qk != 0.0)
        {
            const double r = pk / qk;
            t = std::fabs((ans - r) / r);
            ans = r;
        }
        else
        {
            t = 1.0;
        }
        pkm2 = pkm1;
        pkm1 = pk;
        qkm2 = qkm1;
        qkm1 = qk;
        if (std::fabs(pk) > kBig)
        {
            pkm2 *= kBigInv;
            pkm1 *= kBigInv;
            qkm2 *= kBigInv;
            qkm1 *= kBigInv;
        }
    } while (t > kMachep);

    return ans * ax;
}
} // namespace

double regularizedGammaQ(double a, double x)
{
    if (x <= 0.0 || a <= 0.0)
    {
        return 1.0;
    }
    if (x < 1.0 || x < a)
    {
        return 1.0 - lowerSeries(a, x);
    }
    return upperFraction(a, x);
}

double normalCdf(double z)
{
    return 0.5 * std::erfc(-z / std::sqrt(2.0));
}

double normalQuantile(double p)
{
    // Clamp the open interval (the callers only ever pass p in [0.75, 1)).
    if (p <= 0.0)
    {
        p = 1e-300;
    }
    if (p >= 1.0)
    {
        p = 1.0 - 1e-16;
    }

    // Acklam's rational approximation (|abs error| < 1.15e-9 before refinement).
    static const double a[] = {-3.969683028665376e+01, 2.209460984245205e+02, -2.759285104469687e+02,
                               1.383577518672690e+02,  -3.066479806614716e+01, 2.506628277459239e+00};
    static const double b[] = {-5.447609879822406e+01, 1.615858368580409e+02, -1.556989798598866e+02,
                               6.680131188771972e+01,  -1.328068155288572e+01};
    static const double c[] = {-7.784894002430293e-03, -3.223964580411365e-01, -2.400758277161838e+00,
                               -2.549732539343734e+00, 4.374664141464968e+00,  2.938163982698783e+00};
    static const double d[] = {7.784695709041462e-03, 3.224671290700398e-01, 2.445134137142996e+00,
                               3.754408661907416e+00};
    constexpr double pLow = 0.02425;
    const double pHigh = 1.0 - pLow;

    double z;
    if (p < pLow)
    {
        const double q = std::sqrt(-2.0 * std::log(p));
        z = (((((c[0] * q + c[1]) * q + c[2]) * q + c[3]) * q + c[4]) * q + c[5]) /
            ((((d[0] * q + d[1]) * q + d[2]) * q + d[3]) * q + 1.0);
    }
    else if (p <= pHigh)
    {
        const double q = p - 0.5;
        const double r = q * q;
        z = (((((a[0] * r + a[1]) * r + a[2]) * r + a[3]) * r + a[4]) * r + a[5]) * q /
            (((((b[0] * r + b[1]) * r + b[2]) * r + b[3]) * r + b[4]) * r + 1.0);
    }
    else
    {
        const double q = std::sqrt(-2.0 * std::log(1.0 - p));
        z = -(((((c[0] * q + c[1]) * q + c[2]) * q + c[3]) * q + c[4]) * q + c[5]) /
            ((((d[0] * q + d[1]) * q + d[2]) * q + d[3]) * q + 1.0);
    }

    // One Halley step against the true CDF for full double accuracy.
    constexpr double sqrtTwoPi = 2.50662827463100050242;
    const double err = normalCdf(z) - p;
    const double u = err * sqrtTwoPi * std::exp(z * z / 2.0);
    z -= u / (1.0 + z * u / 2.0);
    return z;
}

double chiSquareCritical(double alpha, double dof)
{
    if (dof <= 0.0)
    {
        return 0.0;
    }
    if (alpha <= 0.0)
    {
        return HUGE_VAL;
    }
    if (alpha >= 1.0)
    {
        return 0.0;
    }

    const double a = dof / 2.0;
    // Wilson-Hilferty initial guess: chi^2 ~ dof * (1 - 2/(9 dof) + z * sqrt(2/(9 dof)))^3.
    const double z = normalQuantile(1.0 - alpha);
    const double h = 2.0 / (9.0 * dof);
    double x = dof * std::pow(1.0 - h + z * std::sqrt(h), 3.0);
    if (!(x > 0.0))
    {
        x = dof; // guard against a bad cube for tiny dof
    }

    // Newton refinement on f(x) = regularizedGammaQ(a, x/2) - alpha. With
    // f'(x) = -(1/2) (x/2)^(a-1) e^{-x/2} / Gamma(a) (the chi-square density).
    for (int i = 0; i < 20; i++)
    {
        const double f = regularizedGammaQ(a, x / 2.0) - alpha;
        const double logDensity = (a - 1.0) * std::log(x / 2.0) - x / 2.0 - std::lgamma(a);
        const double deriv = -0.5 * std::exp(logDensity);
        if (deriv == 0.0)
        {
            break;
        }
        const double step = f / deriv;
        x -= step;
        if (x <= 0.0)
        {
            x = 1e-12;
        }
        if (std::fabs(step) < 1e-12 * (x + 1e-12))
        {
            break;
        }
    }
    return x;
}

} // namespace dft

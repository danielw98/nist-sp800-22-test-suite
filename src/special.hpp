// Special functions needed by the second-level analysis.
#ifndef DFT_SPECIAL_HPP
#define DFT_SPECIAL_HPP

namespace dft {

// Regularized upper incomplete gamma function Q(a, x) = Gamma(a, x) / Gamma(a).
// Port of the Cephes routine used by the NIST suite to turn a chi-square
// statistic into the uniformity p-value.
double regularizedGammaQ(double a, double x);

// Standard normal cumulative distribution function, Phi(z) = P(Z <= z).
// Used by the Cumulative Sums test.
double normalCdf(double z);

// Inverse standard normal CDF (probit): the z with Phi(z) = p, for 0 < p < 1.
// Gives the critical value for the normal-statistic tests, e.g.
// normalQuantile(1 - alpha/2) = 1.959964 (alpha=0.05), 2.575829 (alpha=0.01).
double normalQuantile(double p);

// Upper-tail chi-square critical value: the x with P(chi^2_dof > x) = alpha,
// i.e. regularizedGammaQ(dof/2, x/2) = alpha. The critical value against which a
// chi-square test statistic is compared (reject if statistic > x).
double chiSquareCritical(double alpha, double dof);

} // namespace dft

#endif // DFT_SPECIAL_HPP

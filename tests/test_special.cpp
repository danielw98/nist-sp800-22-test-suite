// Regularized upper incomplete gamma Q(a, x), checked against closed forms.
#include "doctest.h"

#include <cmath>
#include <initializer_list>

#include "special.hpp"

using namespace dft;

TEST_CASE("boundary values")
{
    CHECK(regularizedGammaQ(1.0, 0.0) == doctest::Approx(1.0)); // Q(a, 0) = 1
    CHECK(regularizedGammaQ(2.5, 0.0) == doctest::Approx(1.0));
    CHECK(regularizedGammaQ(1.0, 50.0) < 1e-15);                // Q(a, large) -> 0
}

TEST_CASE("Q(1, x) = exp(-x)")
{
    for (double x : {0.5, 1.0, 2.0, 5.0})
    {
        CAPTURE(x);
        CHECK(regularizedGammaQ(1.0, x) == doctest::Approx(std::exp(-x)).epsilon(1e-9));
    }
}

TEST_CASE("Q(1/2, x) = erfc(sqrt(x))")
{
    for (double x : {0.25, 1.0, 3.0})
    {
        CAPTURE(x);
        CHECK(regularizedGammaQ(0.5, x) == doctest::Approx(std::erfc(std::sqrt(x))).epsilon(1e-9));
    }
}

TEST_CASE("normalQuantile matches the standard normal table")
{
    CHECK(normalQuantile(0.95) == doctest::Approx(1.6448536).epsilon(1e-6));
    CHECK(normalQuantile(0.975) == doctest::Approx(1.9599640).epsilon(1e-6)); // z_{0.025}
    CHECK(normalQuantile(0.995) == doctest::Approx(2.5758293).epsilon(1e-6)); // z_{0.005}, alpha=0.01
    CHECK(normalQuantile(0.5) == doctest::Approx(0.0));
    CHECK(normalQuantile(0.025) == doctest::Approx(-1.9599640).epsilon(1e-6)); // symmetry
}

TEST_CASE("chiSquareCritical matches the chi-square table and round-trips")
{
    CHECK(chiSquareCritical(0.05, 1.0) == doctest::Approx(3.8414588).epsilon(1e-5));
    CHECK(chiSquareCritical(0.01, 2.0) == doctest::Approx(9.2103404).epsilon(1e-5)); // rank dof=2
    CHECK(chiSquareCritical(0.01, 5.0) == doctest::Approx(15.0862724).epsilon(1e-5)); // overlapping dof=5
    CHECK(chiSquareCritical(0.01, 8.0) == doctest::Approx(20.0902350).epsilon(1e-5)); // non-overlapping dof=8
    // Round-trip: Q(dof/2, crit/2) == alpha for an assortment of (alpha, dof).
    for (double dof : {1.0, 2.0, 6.0, 64.0, 512.0})
    {
        for (double alpha : {0.01, 0.05, 0.1})
        {
            CAPTURE(dof);
            CAPTURE(alpha);
            const double crit = chiSquareCritical(alpha, dof);
            CHECK(regularizedGammaQ(dof / 2.0, crit / 2.0) == doctest::Approx(alpha).epsilon(1e-6));
        }
    }
}

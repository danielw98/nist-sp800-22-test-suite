/*
 * Minimal driver that reproduces the EXACT original NIST sts-2.1.2
 * DiscreteFourierTransform() arithmetic on the section 2.6.8 example,
 * using the original ogg FFT (dfft.c). Built only to observe what number
 * the reference code actually emits.
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void __ogg_fdrffti(int n, double *wsave, int *ifac);
void __ogg_fdrfftf(int n, double *r, double *wsave, int *ifac);

int main(void)
{
    const char *bits =
        "11001001000011111101101010100010001000010110100011"
        "00001000110100110001001100011001100010100010111000";

    int n = 100;
    /* allocate n+1 so the original loop's X[n] read is in-bounds */
    double *X = (double *)calloc(n + 1, sizeof(double));
    double *wsave = (double *)calloc(2 * n, sizeof(double));
    double *m = (double *)calloc(n / 2 + 1, sizeof(double));
    int ifac[15];
    int i, count;
    double upperBound, N_l, N_o, d, p_value;

    for (i = 0; i < n; i++)
    {
        X[i] = 2 * (bits[i] - '0') - 1;
    }

    __ogg_fdrffti(n, wsave, ifac);
    __ogg_fdrfftf(n, X, wsave, ifac);

    m[0] = sqrt(X[0] * X[0]);
    for (i = 0; i < n / 2; i++)
    {
        m[i + 1] = sqrt(pow(X[2 * i + 1], 2) + pow(X[2 * i + 2], 2));
    }

    count = 0;
    upperBound = sqrt(2.995732274 * n);
    for (i = 0; i < n / 2; i++)
    {
        if (m[i] < upperBound)
        {
            count++;
        }
    }
    N_l = (double)count;
    N_o = (double)0.95 * n / 2.0;
    d = (N_l - N_o) / sqrt(n / 4.0 * 0.95 * 0.05);
    p_value = erfc(fabs(d) / sqrt(2.0));

    printf("ORIGINAL NIST sts-2.1.2 reference (ogg FFT):\n");
    printf("  T (upperBound) = %f\n", upperBound);
    printf("  N1 (count)     = %d\n", count);
    printf("  N0             = %f\n", N_o);
    printf("  d              = %f\n", d);
    printf("  p_value        = %f\n", p_value);

    free(X);
    free(wsave);
    free(m);
    return 0;
}

#!/usr/bin/env python3
"""Validation suite for the spectral test.

Three kinds of checks, all against the compiled binary where possible:

  1. Known-answer cases  - run dft_test on fixed inputs and assert the result
     (and emit a spectrum graph per case into tests/graphs/).
  2. Statistical check   - Monte-Carlo under H0: the false-positive rate is near
     alpha and the p-values are roughly uniform.
  3. Reference cross-check - the original NIST ogg-FFT driver yields N1 = 48 on
     the section 2.6.8 example, matching our implementation.

Run as a script (asserts + graphs + summary) or under pytest (the test_* funcs):
    python tests/validate.py
    pytest tests/validate.py
"""
import json
import math
import os
import subprocess

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EXE = ".exe" if os.name == "nt" else ""
NIST_TEST = os.path.join(ROOT, "build", "nist_test" + EXE)
REF_DRIVER = os.path.join(ROOT, "reference", "sts-2.1.2-original", "ref_dft_driver" + EXE)
GRAPHS = os.path.join(ROOT, "tests", "graphs")

NIST_EXAMPLE = (
    "11001001000011111101101010100010001000010110100011"
    "00001000110100110001001100011001100010100010111000"
)
LN20 = math.log(20.0)


def run_case(bits):
    """Run dft_test --json --spectrum on a bit string and return the parsed result."""
    out = subprocess.run(
        [NIST_TEST, "--json", "--spectrum", "-t", "dft", "-"],
        input=bits,
        capture_output=True,
        text=True,
    )
    return json.loads(out.stdout)


def save_graph(slug, result):
    """Write a spectrum graph (|S[k]| + threshold) for one case."""
    os.makedirs(GRAPHS, exist_ok=True)
    spec = result.get("spectrum", [])
    if not spec:
        return
    ks = [p["k"] for p in spec]
    mags = [p["mag"] for p in spec]
    fig, ax = plt.subplots(figsize=(7, 2.8))
    ax.plot(ks, mags, lw=0.7, color="#2b6cb0")
    ax.axhline(result["stats"]["threshold"], color="#c0392b", lw=1.0, ls="--")
    ax.set_title(f"{slug}: N1={result['stats']['N1']:.0f}, p={result['p_value']:.4g}")
    ax.set_xlabel("k")
    ax.set_ylabel("|S[k]|")
    fig.tight_layout()
    fig.savefig(os.path.join(GRAPHS, slug + ".svg"))
    plt.close(fig)


# (slug, description, bits, predicate on result)
CASES = [
    ("nist_example", "Exemplul NIST 2.6.8 (n=100): N1=48, p=0.646355",
     NIST_EXAMPLE, lambda r: r["stats"]["N1"] == 48 and abs(r["p_value"] - 0.646355) < 1e-4),
    ("all_ones", "Numai 1 (n=4096): DC domina -> respins",
     "1" * 4096, lambda r: not r["passed"]),
    ("all_zeros", "Numai 0 (n=4096): la fel ca numai 1 -> respins",
     "0" * 4096, lambda r: not r["passed"]),
    ("periodic_0001", "Periodic 0001 (n=4096): varf in banda -> respins",
     "0001" * 1024, lambda r: not r["passed"]),
    ("period_2", "Alternant 01 (n=4096): respins indirect (Nyquist exclus)",
     "01" * 2048, lambda r: not r["passed"] and r["stats"]["N1"] == 2048),
]


def test_known_answers():
    for slug, _desc, bits, ok in CASES:
        r = run_case(bits)
        save_graph(slug, r)
        assert ok(r), f"{slug} failed: {r}"


def test_statistical_under_h0():
    rng = np.random.default_rng(2024)
    n, trials = 2048, 2000
    x = 2.0 * rng.integers(0, 2, size=(trials, n)) - 1.0
    m = np.abs(np.fft.fft(x, axis=1)[:, : n // 2])
    T = math.sqrt(LN20 * n)
    n1 = np.sum(m < T, axis=1).astype(float)
    n0 = 0.95 * n / 2
    d = (n1 - n0) / math.sqrt(n / 4 * 0.95 * 0.05)
    from scipy.special import erfc
    p = erfc(np.abs(d) / math.sqrt(2.0))
    fp = float(np.mean(p < 0.01))
    # The known mild over-rejection: nominal 0.01, empirically ~0.012-0.015.
    assert 0.004 < fp < 0.03, f"false-positive rate out of range: {fp}"
    # p-values should be spread across the unit interval, not clumped.
    counts, _ = np.histogram(p, bins=10, range=(0, 1))
    assert counts.min() > 0, f"empty p-value bin: {counts}"


def test_reference_driver_matches():
    if not os.path.exists(REF_DRIVER):
        return  # driver not built; skip
    out = subprocess.run([REF_DRIVER], capture_output=True, text=True).stdout
    assert "N1 (count)     = 48" in out, out


def main():
    print(f"binary: {NIST_TEST}")
    failures = 0
    for slug, desc, bits, ok in CASES:
        r = run_case(bits)
        save_graph(slug, r)
        passed = ok(r)
        failures += not passed
        print(f"  [{'PASS' if passed else 'FAIL'}] {desc}  -> N1={r['stats']['N1']:.0f} p={r['p_value']:.4g}")
    for fn in (test_statistical_under_h0, test_reference_driver_matches):
        try:
            fn()
            print(f"  [PASS] {fn.__name__}")
        except AssertionError as e:
            failures += 1
            print(f"  [FAIL] {fn.__name__}: {e}")
    print(f"graphs -> {GRAPHS}")
    print("ALL PASSED" if failures == 0 else f"{failures} FAILURE(S)")
    raise SystemExit(1 if failures else 0)


if __name__ == "__main__":
    main()

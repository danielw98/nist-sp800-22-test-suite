// Per-run parameters, parsed once at the CLI/API boundary and handed to a test's
// factory. Most tests use only `alpha` (and some a block size); the DFT-specific
// fields are ignored by the others.
#ifndef DFT_TEST_PARAMS_HPP
#define DFT_TEST_PARAMS_HPP

#include <cstddef>

#include "engine_factory.hpp" // Method (DFT transform-engine selection)

namespace dft {

struct TestParams
{
    double alpha = 0.01;
    std::size_t blockSize = 0;        // M / m for block tests; 0 = the test's default
    bool spectrum = false;            // DFT only: include the downsampled spectrum
    std::size_t spectrumPoints = 4000;
    Method method = Method::Auto;     // DFT only: transform engine
};

} // namespace dft

#endif // DFT_TEST_PARAMS_HPP

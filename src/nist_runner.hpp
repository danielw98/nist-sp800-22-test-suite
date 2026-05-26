// Generic facade: pick a test by id, run it, write a uniform report. Replaces the
// DFT-only spectral_runner so every suite test shares one output contract.
#ifndef DFT_NIST_RUNNER_HPP
#define DFT_NIST_RUNNER_HPP

#include <iosfwd>
#include <string>

#include "bit_sequence.hpp"
#include "test_params.hpp"

namespace dft {

struct RunRequest
{
    std::string test = "dft";
    TestParams params;
    bool json = false;
};

// Run req.test on seq and write the report (JSON envelope or human-readable) to
// os. Throws std::invalid_argument on an unknown test id or too-short input.
void runTest(std::ostream &os, const BitSequence &seq, const RunRequest &req);

} // namespace dft

#endif // DFT_NIST_RUNNER_HPP

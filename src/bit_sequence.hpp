// A validated container of bits, with the input plumbing shared by every entry
// point (literal string, text file, binary file, stdin) in one place.
#ifndef DFT_BIT_SEQUENCE_HPP
#define DFT_BIT_SEQUENCE_HPP

#include <cstdint>
#include <istream>
#include <string>
#include <vector>

#include "complex.hpp"

namespace dft {

class BitSequence
{
public:
    BitSequence() = default;

    explicit BitSequence(std::vector<std::uint8_t> bits) : bits_(std::move(bits))
    {
    }

    // Keep only '0'/'1' characters from arbitrary text (whitespace etc. ignored).
    static BitSequence fromText(const std::string &text);

    // Read text bits from a stream or a file.
    static BitSequence fromStream(std::istream &in);
    static BitSequence fromTextFile(const std::string &path);

    // Read a binary file, unpacking each byte into 8 bits, most-significant first.
    static BitSequence fromBinaryFile(const std::string &path);

    // Auto-detect: treat as ASCII bits if the file is only '0'/'1' + whitespace,
    // otherwise unpack it as binary.
    static BitSequence fromFileAuto(const std::string &path);

    // Resolve a CLI token that is either a path or a literal bitstring.
    static BitSequence fromArg(const std::string &arg);

    std::size_t size() const
    {
        return bits_.size();
    }

    bool empty() const
    {
        return bits_.empty();
    }

    // Raw 0/1 bits, for the time-domain tests (counting, runs, blocks).
    const std::vector<std::uint8_t> &bits() const
    {
        return bits_;
    }

    // Map to the +1/-1 complex sequence consumed by the DFT (x_i = 2*e_i - 1).
    std::vector<Complex> toBipolar() const;

    // Split into consecutive blocks of `length` bits. A trailing partial block
    // is dropped (NIST processes whole bitstreams only). maxStreams == 0 -> all.
    std::vector<BitSequence> split(std::size_t length, std::size_t maxStreams = 0) const;

private:
    std::vector<std::uint8_t> bits_;
};

} // namespace dft

#endif // DFT_BIT_SEQUENCE_HPP

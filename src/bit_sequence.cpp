#include "bit_sequence.hpp"

#include <fstream>
#include <iterator>
#include <sstream>
#include <stdexcept>

namespace dft {

namespace {
bool isAsciiBitChar(unsigned char byte)
{
    return byte == '0' || byte == '1' || byte == ' ' || byte == '\n' || byte == '\r' ||
           byte == '\t' || byte == '\f' || byte == '\v';
}

void unpackByte(unsigned char byte, std::vector<std::uint8_t> &out)
{
    for (int k = 7; k >= 0; k--)
    {
        out.push_back(static_cast<std::uint8_t>((byte >> k) & 1U));
    }
}
} // namespace

BitSequence BitSequence::fromText(const std::string &text)
{
    std::vector<std::uint8_t> bits;
    bits.reserve(text.size());
    for (char c : text)
    {
        if (c == '0')
        {
            bits.push_back(0);
        }
        else if (c == '1')
        {
            bits.push_back(1);
        }
    }
    return BitSequence(std::move(bits));
}

BitSequence BitSequence::fromStream(std::istream &in)
{
    std::ostringstream ss;
    ss << in.rdbuf();
    return fromText(ss.str());
}

BitSequence BitSequence::fromTextFile(const std::string &path)
{
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs)
    {
        throw std::runtime_error("cannot open file: " + path);
    }
    return fromStream(ifs);
}

BitSequence BitSequence::fromBinaryFile(const std::string &path)
{
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs)
    {
        throw std::runtime_error("cannot open file: " + path);
    }
    std::vector<std::uint8_t> bits;
    char ch;
    while (ifs.get(ch))
    {
        unpackByte(static_cast<unsigned char>(ch), bits);
    }
    return BitSequence(std::move(bits));
}

BitSequence BitSequence::fromFileAuto(const std::string &path)
{
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs)
    {
        throw std::runtime_error("cannot open file: " + path);
    }
    const std::vector<unsigned char> raw((std::istreambuf_iterator<char>(ifs)),
                                         std::istreambuf_iterator<char>());

    bool isText = true;
    for (unsigned char byte : raw)
    {
        if (!isAsciiBitChar(byte))
        {
            isText = false;
            break;
        }
    }

    std::vector<std::uint8_t> bits;
    if (isText)
    {
        bits.reserve(raw.size());
        for (unsigned char byte : raw)
        {
            if (byte == '0')
            {
                bits.push_back(0);
            }
            else if (byte == '1')
            {
                bits.push_back(1);
            }
        }
    }
    else
    {
        bits.reserve(raw.size() * 8);
        for (unsigned char byte : raw)
        {
            unpackByte(byte, bits);
        }
    }
    return BitSequence(std::move(bits));
}

BitSequence BitSequence::fromArg(const std::string &arg)
{
    std::ifstream probe(arg, std::ios::binary);
    if (probe)
    {
        return fromFileAuto(arg);
    }
    return fromText(arg);
}

std::vector<Complex> BitSequence::toBipolar() const
{
    std::vector<Complex> x(bits_.size());
    for (std::size_t i = 0; i < bits_.size(); i++)
    {
        x[i] = Complex(bits_[i] != 0 ? 1.0 : -1.0, 0.0);
    }
    return x;
}

std::vector<BitSequence> BitSequence::split(std::size_t length, std::size_t maxStreams) const
{
    std::vector<BitSequence> streams;
    if (length == 0)
    {
        return streams;
    }

    std::size_t count = bits_.size() / length;
    if (maxStreams != 0 && count > maxStreams)
    {
        count = maxStreams;
    }

    streams.reserve(count);
    for (std::size_t streamIndex = 0; streamIndex < count; streamIndex++)
    {
        const std::size_t start = streamIndex * length;
        std::vector<std::uint8_t> chunk(bits_.begin() + static_cast<std::ptrdiff_t>(start),
                                        bits_.begin() + static_cast<std::ptrdiff_t>(start + length));
        streams.emplace_back(std::move(chunk));
    }
    return streams;
}

} // namespace dft

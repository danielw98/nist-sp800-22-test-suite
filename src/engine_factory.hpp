// Selecting a DFT engine by name. Used by the DFT test (the nist_test/nist_assess CLIs).
//
// The user-facing string is parsed once, at the CLI boundary, into a Method
// enum; everything downstream takes the enum, so building an engine cannot fail
// on a bad name (the switch is exhaustive and never throws).
#ifndef DFT_ENGINE_FACTORY_HPP
#define DFT_ENGINE_FACTORY_HPP

#include <memory>
#include <optional>
#include <string_view>

#include "fft.hpp"

namespace dft {

// Which transform engine to use. Auto currently resolves to the FFT engine.
enum class Method { Auto, Fft, Direct };

// Parse a method name ("auto" | "fft" | "direct"); returns nullopt if unknown,
// so the caller decides how to report the error (no exception thrown here).
inline std::optional<Method> parseMethod(std::string_view name)
{
    if (name == "auto")
    {
        return Method::Auto;
    }
    if (name == "fft")
    {
        return Method::Fft;
    }
    if (name == "direct")
    {
        return Method::Direct;
    }
    return std::nullopt;
}

// Build the engine for a method. Total over the enum, so it never throws.
inline std::shared_ptr<const DftEngine> makeEngine(Method method)
{
    switch (method)
    {
    case Method::Direct:
        return std::make_shared<DirectDftEngine>();
    case Method::Fft:
    case Method::Auto:
        break;
    }
    return std::make_shared<FftEngine>();
}

} // namespace dft

#endif // DFT_ENGINE_FACTORY_HPP

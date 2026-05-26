// The complex scalar type used throughout the DFT code. Defined once here so the
// FFT engines and the bit-sequence mapping share a single name.
#ifndef DFT_COMPLEX_HPP
#define DFT_COMPLEX_HPP

#include <complex>

namespace dft {

using Complex = std::complex<double>;

} // namespace dft

#endif // DFT_COMPLEX_HPP

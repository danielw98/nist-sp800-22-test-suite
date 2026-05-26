cmake_minimum_required(VERSION 3.16)
project(spectral_dft_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-O2 -Wall -Wextra)
endif()

# Reusable core library: FFT engines, bit I/O, special functions, the test
# registry + generic runner, and all the suite tests.
add_library(dft
    src/fft.cpp
    src/special.cpp
    src/verdict.cpp
    src/bit_sequence.cpp
    src/assessment.cpp
    src/spectral_test.cpp
    src/test_registry.cpp
    src/nist_runner.cpp
    src/monobit_test.cpp
    src/block_frequency_test.cpp
    src/runs_test.cpp
    src/longest_run_test.cpp
    src/rank_test.cpp
    src/non_overlapping_test.cpp
    src/overlapping_test.cpp
    src/universal_test.cpp
    src/linear_complexity_test.cpp
    src/cusum_test.cpp
    src/serial_test.cpp
    src/approx_entropy_test.cpp
    src/random_excursions_test.cpp
    src/random_excursions_variant_test.cpp)
target_include_directories(dft PUBLIC src)

# Single-sequence command-line tool: nist_test --test <id>.
add_executable(nist_test src/nist_main.cpp)
target_link_libraries(nist_test PRIVATE dft)

# Many-bitstream assessment tool (NIST proportion + uniformity), any test.
add_executable(nist_assess src/nist_assess_main.cpp)
target_link_libraries(nist_assess PRIVATE dft)

# Unit tests (doctest, header-only and vendored in tests/). Grouped by area:
# fft (engine correctness), spectral (test mechanics + edge cases), nist
# (SP 800-22 compliance), bit_sequence / special / assessment (supporting code).
enable_testing()
file(GLOB DFT_TEST_SOURCES CONFIGURE_DEPENDS tests/test_*.cpp)
add_executable(dft_tests ${DFT_TEST_SOURCES})
target_include_directories(dft_tests PRIVATE tests)
target_link_libraries(dft_tests PRIVATE dft)
# Lets the compliance tests reproduce the NIST examples that use the data files
# (e.g. the first 100000 bits of e). Tests skip these checks if the dir is unset.
target_compile_definitions(dft_tests PRIVATE DFT_DATA_DIR="${CMAKE_SOURCE_DIR}/data")
add_test(NAME unit COMMAND dft_tests)

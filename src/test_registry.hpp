// The one place every suite test is registered. listTests() powers catalogs and
// help text; makeTest(id, params) builds a test by its slug. Adding a test means
// adding one line to the table in test_registry.cpp.
#ifndef DFT_TEST_REGISTRY_HPP
#define DFT_TEST_REGISTRY_HPP

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "test.hpp"
#include "test_params.hpp"

namespace dft {

struct TestInfo
{
    std::string id;
    std::string name;
    std::size_t minLength;
};

// Every implemented test, in NIST section order.
std::vector<TestInfo> listTests();

// Build a test by id; returns nullptr if the id is unknown.
std::unique_ptr<RandomnessTest> makeTest(const std::string &id, const TestParams &params);

} // namespace dft

#endif // DFT_TEST_REGISTRY_HPP

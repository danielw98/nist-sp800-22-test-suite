#include "nist_runner.hpp"

#include <iomanip>
#include <ostream>
#include <stdexcept>

#include "test.hpp"
#include "test_registry.hpp"
#include "verdict.hpp"

namespace dft {
namespace {

void writeJson(std::ostream &os, const std::string &id, const TestReport &r, double alpha)
{
    os << std::fixed << std::setprecision(6);
    os << "{\n";
    os << "  \"test\": \"" << id << "\",\n";
    os << "  \"n\": " << r.n << ",\n";
    const CriticalVerdict v = criticalVerdict(r, alpha);
    os << "  \"p_value\": " << r.pValue << ",\n";
    os << "  \"passed\": " << (!v.reject ? "true" : "false") << ",\n";
    if (v.hasCritical)
    {
        os << "  \"statistic\": " << r.statistic << ",\n";
        os << "  \"critical\": " << v.critical << ",\n";
        os << "  \"distribution\": \"" << refDistLabel(r.dist) << "\",\n";
        if (r.dist == RefDist::ChiSquareUpper)
        {
            os << "  \"dof\": " << r.dof << ",\n";
        }
    }
    os << "  \"stats\": {";
    for (std::size_t i = 0; i < r.stats.size(); i++)
    {
        os << (i == 0 ? "\n" : ",\n") << "    \"" << r.stats[i].label << "\": " << r.stats[i].value;
    }
    os << (r.stats.empty() ? "}" : "\n  }");
    if (!r.pValues.empty())
    {
        os << ",\n  \"p_values\": [";
        for (std::size_t i = 0; i < r.pValues.size(); i++)
        {
            os << (i == 0 ? "\n" : ",\n") << "    {\"label\": \"" << r.pValues[i].label
               << "\", \"p\": " << r.pValues[i].value << "}";
        }
        os << "\n  ]";
    }
    if (r.spectrum)
    {
        const std::vector<SpectrumPoint> &sp = *r.spectrum;
        os << ",\n  \"spectrum\": [";
        for (std::size_t i = 0; i < sp.size(); i++)
        {
            os << (i == 0 ? "\n" : ",\n") << "    {\"k\": " << sp[i].k << ", \"mag\": " << sp[i].mag
               << ", \"above\": " << (sp[i].above ? "true" : "false") << "}";
        }
        os << (sp.empty() ? "]" : "\n  ]");
    }
    os << "\n}\n";
}

void writeHuman(std::ostream &os, const std::string &name, const TestReport &r, double alpha)
{
    os << std::fixed << std::setprecision(6);
    os << name << "\n";
    os << "  n        = " << r.n << "\n";
    for (const NamedStat &s : r.stats)
    {
        os << "  " << s.label << " = " << s.value << "\n";
    }
    for (const NamedStat &p : r.pValues)
    {
        os << "  p[" << p.label << "] = " << p.value << "\n";
    }
    const CriticalVerdict v = criticalVerdict(r, alpha);
    if (v.hasCritical)
    {
        os << "  statistic = " << r.statistic << "  (" << refDistLabel(r.dist) << ")\n";
        if (r.dist == RefDist::ChiSquareUpper)
        {
            os << "  critical  = " << v.critical << "  (chi^2 upper, dof = " << r.dof
               << ", alpha = " << alpha << ")\n";
        }
        else
        {
            os << "  critical  = " << v.critical << "  (z at alpha/2 = " << alpha / 2.0 << ")\n";
        }
    }
    os << "  p-value  = " << r.pValue << "\n";
    os << "  result   = " << (!v.reject ? "random (do not reject)" : "non-random (reject)")
       << " at alpha = " << alpha << "\n";
}

} // namespace

void runTest(std::ostream &os, const BitSequence &seq, const RunRequest &req)
{
    std::unique_ptr<RandomnessTest> test = makeTest(req.test, req.params);
    if (test == nullptr)
    {
        throw std::invalid_argument("unknown test: " + req.test);
    }
    const TestReport report = test->run(seq);
    if (req.json)
    {
        writeJson(os, test->id(), report, req.params.alpha);
    }
    else
    {
        writeHuman(os, test->name(), report, req.params.alpha);
    }
}

} // namespace dft

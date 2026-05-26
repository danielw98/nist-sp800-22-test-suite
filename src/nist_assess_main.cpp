// nist_assess - run one NIST test over many bitstreams of one or more files and
// report the SP 800-22 proportion + uniformity metrics for each.
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "assessment.hpp"
#include "bit_sequence.hpp"
#include "engine_factory.hpp"
#include "test.hpp"
#include "test_params.hpp"
#include "test_registry.hpp"

namespace {

void printUsage(const char *prog)
{
    std::cerr << "nist_assess - proportion + uniformity over many bitstreams\n\n"
              << "Usage:\n"
              << "  " << prog << " [options] <file> [<file> ...]\n\n"
              << "Each file is split into consecutive bitstreams; every stream is run through\n"
              << "the chosen test, then the streams are aggregated (proportion + uniformity).\n\n"
              << "Options:\n"
              << "  -t, --test <id>         test to run (default dft)\n"
              << "  -n, --length <bits>     bits per stream (default 10000)\n"
              << "  -s, --streams <count>   cap on the number of streams (default all)\n"
              << "  -a, --alpha <value>     significance level (default 0.01)\n"
              << "  -b, --block <N>         block size M/m for block tests\n"
              << "  -m, --method <auto|fft|direct>  DFT transform engine (dft only)\n"
              << "      --json              emit machine-readable JSON\n"
              << "  -h, --help              show this help\n";
}

void reportFile(const dft::RandomnessTest &test, double alpha, const std::string &path,
                std::size_t length, std::size_t maxStreams)
{
    const dft::BitSequence seq = dft::BitSequence::fromFileAuto(path);
    const std::vector<dft::BitSequence> streams = seq.split(length, maxStreams);

    std::cout << "File: " << path << "\n";
    std::cout << "  bits available : " << seq.size() << "\n";
    std::cout << "  stream length  : " << length << "\n";
    std::cout << "  streams used   : " << streams.size() << "\n";

    if (streams.empty())
    {
        std::cout << "  (not enough bits for one stream)\n\n";
        return;
    }

    const dft::Assessment assessment(test, alpha);
    const dft::AssessmentResult result = assessment.evaluate(streams);

    std::cout << "  p-value bins   :";
    for (long count : result.bins)
    {
        std::cout << ' ' << count;
    }
    std::cout << "\n";

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "  uniformity p   : " << result.uniformity << "  ("
              << (result.uniformityPassed ? "PASS" : "FAIL") << ")\n";
    std::cout << "  proportion     : " << result.passCount << "/" << result.sampleCount << " = "
              << result.proportion << "  accept [" << result.proportionMin << ", "
              << result.proportionMax << "]  (" << (result.proportionPassed ? "PASS" : "FAIL")
              << ")\n";
    std::cout << "  verdict        : "
              << ((result.proportionPassed && result.uniformityPassed) ? "PASS" : "FAIL") << "\n\n";
    std::cout.unsetf(std::ios::floatfield);
}

// Escape a string for inclusion as a JSON string value (backslashes, quotes).
std::string jsonEscape(const std::string &text)
{
    std::string escaped;
    escaped.reserve(text.size() + 8);
    for (char ch : text)
    {
        if (ch == '\\' || ch == '"')
        {
            escaped += '\\';
        }
        escaped += ch;
    }
    return escaped;
}

void reportFileJson(std::ostream &os, const dft::RandomnessTest &test, double alpha,
                    const std::string &path, std::size_t length, std::size_t maxStreams)
{
    const dft::BitSequence seq = dft::BitSequence::fromFileAuto(path);
    const std::vector<dft::BitSequence> streams = seq.split(length, maxStreams);

    os << std::fixed << std::setprecision(6);
    os << "    {\n";
    os << "      \"file\": \"" << jsonEscape(path) << "\",\n";
    os << "      \"bitsAvailable\": " << seq.size() << ",\n";
    os << "      \"streamLength\": " << length << ",\n";
    os << "      \"streamsUsed\": " << streams.size();

    if (streams.empty())
    {
        os << "\n    }";
        return;
    }

    const dft::Assessment assessment(test, alpha);
    const dft::AssessmentResult result = assessment.evaluate(streams);

    os << ",\n      \"bins\": [";
    for (std::size_t i = 0; i < result.bins.size(); i++)
    {
        os << (i == 0 ? "" : ", ") << result.bins[i];
    }
    os << "],\n";
    os << "      \"uniformity\": " << result.uniformity << ",\n";
    os << "      \"uniformityPassed\": " << (result.uniformityPassed ? "true" : "false") << ",\n";
    os << "      \"passCount\": " << result.passCount << ",\n";
    os << "      \"sampleCount\": " << result.sampleCount << ",\n";
    os << "      \"proportion\": " << result.proportion << ",\n";
    os << "      \"proportionMin\": " << result.proportionMin << ",\n";
    os << "      \"proportionMax\": " << result.proportionMax << ",\n";
    os << "      \"proportionPassed\": " << (result.proportionPassed ? "true" : "false") << ",\n";
    os << "      \"verdict\": \""
       << ((result.proportionPassed && result.uniformityPassed) ? "PASS" : "FAIL") << "\"\n";
    os << "    }";
    os.unsetf(std::ios::floatfield);
}

} // namespace

int main(int argc, char **argv)
{
    std::string testId = "dft";
    std::size_t length = 10000;
    std::size_t maxStreams = 0;
    dft::TestParams params;
    bool json = false;
    std::vector<std::string> files;

    for (int i = 1; i < argc; i++)
    {
        const std::string arg = argv[i];
        if (arg == "-h" || arg == "--help")
        {
            printUsage(argv[0]);
            return EXIT_SUCCESS;
        }
        else if (arg == "--json")
        {
            json = true;
        }
        else if ((arg == "-t" || arg == "--test") && i + 1 < argc)
        {
            testId = argv[++i];
        }
        else if ((arg == "-n" || arg == "--length") && i + 1 < argc)
        {
            length = static_cast<std::size_t>(std::strtoull(argv[++i], nullptr, 10));
        }
        else if ((arg == "-s" || arg == "--streams") && i + 1 < argc)
        {
            maxStreams = static_cast<std::size_t>(std::strtoull(argv[++i], nullptr, 10));
        }
        else if ((arg == "-a" || arg == "--alpha") && i + 1 < argc)
        {
            params.alpha = std::atof(argv[++i]);
        }
        else if ((arg == "-b" || arg == "--block") && i + 1 < argc)
        {
            params.blockSize = static_cast<std::size_t>(std::strtoull(argv[++i], nullptr, 10));
        }
        else if ((arg == "-m" || arg == "--method") && i + 1 < argc)
        {
            const auto parsedMethod = dft::parseMethod(argv[++i]);
            if (!parsedMethod)
            {
                std::cerr << "unknown method: " << argv[i] << "\n\n";
                printUsage(argv[0]);
                return EXIT_FAILURE;
            }
            params.method = *parsedMethod;
        }
        else if (arg.size() > 1 && arg[0] == '-')
        {
            std::cerr << "unknown option: " << arg << "\n\n";
            printUsage(argv[0]);
            return EXIT_FAILURE;
        }
        else
        {
            files.push_back(arg);
        }
    }

    if (files.empty())
    {
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    try
    {
        const std::unique_ptr<dft::RandomnessTest> test = dft::makeTest(testId, params);
        if (test == nullptr)
        {
            std::cerr << "unknown test: " << testId << "\n";
            return EXIT_FAILURE;
        }
        if (json)
        {
            std::cout << "{\n  \"test\": \"" << testId << "\",\n  \"files\": [\n";
            for (std::size_t i = 0; i < files.size(); i++)
            {
                reportFileJson(std::cout, *test, params.alpha, files[i], length, maxStreams);
                std::cout << (i + 1 < files.size() ? ",\n" : "\n");
            }
            std::cout << "  ]\n}\n";
        }
        else
        {
            for (const std::string &path : files)
            {
                reportFile(*test, params.alpha, path, length, maxStreams);
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

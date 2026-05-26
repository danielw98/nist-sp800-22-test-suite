// nist_test - run one NIST SP 800-22 statistical test on a single bit sequence.
#include <cstdlib>
#include <exception>
#include <iostream>
#include <optional>
#include <string>

#include "bit_sequence.hpp"
#include "engine_factory.hpp"
#include "nist_runner.hpp"
#include "test_registry.hpp"

namespace {

void printUsage(const char *prog)
{
    std::cerr
        << "nist_test - NIST SP 800-22 statistical tests\n\n"
        << "Usage:\n"
        << "  " << prog << " [options] [<bitstring | file>]\n\n"
        << "Input: a literal 0/1 string, a file path, or stdin (no argument or '-').\n"
        << "Files may be ASCII (0/1) or binary (bytes unpacked MSB-first); auto-detected.\n\n"
        << "Options:\n"
        << "  -t, --test <id>     test to run (default dft; see list below)\n"
        << "  -a, --alpha <value> significance level (default 0.01)\n"
        << "  -b, --block <N>     block size M/m for block tests (test default if omitted)\n"
        << "  -m, --method <auto|fft|direct>  DFT transform engine (dft only)\n"
        << "      --json          emit the machine-readable JSON envelope\n"
        << "      --spectrum      include a downsampled spectrum (dft, JSON only)\n"
        << "  -h, --help          show this help\n\n"
        << "Tests:\n";
    for (const dft::TestInfo &test : dft::listTests())
    {
        std::cerr << "  " << test.id << "\n";
    }
}

} // namespace

int main(int argc, char **argv)
{
    dft::RunRequest req;
    std::string input;
    bool haveInput = false;

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
            req.json = true;
        }
        else if (arg == "--spectrum")
        {
            req.params.spectrum = true;
        }
        else if ((arg == "-t" || arg == "--test") && i + 1 < argc)
        {
            req.test = argv[++i];
        }
        else if ((arg == "-a" || arg == "--alpha") && i + 1 < argc)
        {
            req.params.alpha = std::atof(argv[++i]);
        }
        else if ((arg == "-b" || arg == "--block") && i + 1 < argc)
        {
            req.params.blockSize = static_cast<std::size_t>(std::strtoull(argv[++i], nullptr, 10));
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
            req.params.method = *parsedMethod;
        }
        else if (arg == "-")
        {
            haveInput = false;
        }
        else if (arg.size() > 1 && arg[0] == '-')
        {
            std::cerr << "unknown option: " << arg << "\n\n";
            printUsage(argv[0]);
            return EXIT_FAILURE;
        }
        else
        {
            input = arg;
            haveInput = true;
        }
    }

    try
    {
        const dft::BitSequence seq =
            haveInput ? dft::BitSequence::fromArg(input) : dft::BitSequence::fromStream(std::cin);
        if (seq.empty())
        {
            std::cerr << "no bits provided\n";
            return EXIT_FAILURE;
        }
        dft::runTest(std::cout, seq, req);
    }
    catch (const std::exception &e)
    {
        std::cerr << "error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

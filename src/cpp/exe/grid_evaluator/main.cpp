
#include "antares-xpansion/benders/factories/LoggerFactories.h"
#include "antares-xpansion/benders/output/JsonWriter.h"
#include "antares-xpansion/grid_evaluator/GridCollection.h"
#include "antares-xpansion/grid_evaluator/GridEvaluator.h"

struct ParsedArgs
{
    std::filesystem::path path_to_data;
    ProblemsFormat pb_format = ProblemsFormat::MPS_FILE;
    int num_threads = 1;
};

void printUsage(char** argv)
{
    std::cerr << "Usage: " << argv[0]
              << " <path_to_data> [--pb_format <format>] [--threads <num_threads>]" << std::endl;
}

std::optional<ParsedArgs> parseArguments(int argc, char** argv)
{
    if (argc < 2)
    {
        printUsage(argv);
        return std::nullopt;
    }

    ParsedArgs result;
    result.path_to_data = argv[1];

    for (int i = 2; i < argc; ++i)
    {
        std::string arg = argv[i];

        auto nextArg = [&]() -> std::optional<std::string>
        { return (i + 1 < argc) ? std::optional<std::string>(argv[++i]) : std::nullopt; };

        if (arg == "--pb_format")
        {
            if (auto formatStr = nextArg())
            {
                result.pb_format = problemsFormatFromString(formatStr.value());
            }
            else
            {
                std::cerr << "Error: --pb_format needs a value" << std::endl;
                return std::nullopt;
            }
        }
        else if (arg == "--threads")
        {
            if (auto threadStr = nextArg())
            {
                try
                {
                    result.num_threads = std::stoi(threadStr.value());
                    if (result.num_threads <= 0)
                    {
                        throw std::invalid_argument("Number of threads must be positive");
                    }
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Erreur: --threads needs a positive integer argument." << e.what()
                              << std::endl;
                    return std::nullopt;
                }
            }
            else
            {
                std::cerr << "Erreur: --threads needs a value" << std::endl;
                return std::nullopt;
            }
        }
    }

    return result;
}

int main(int argc, char** argv)
{
    auto parsed = parseArguments(argc, argv);
    if (!parsed)
    {
        printUsage(argv);
        throw std::runtime_error("Error parsing arguments");
    }

    const auto& [path_to_data, pb_format, num_threads] = parsed.value();

    auto report_path = path_to_data / "report.txt";
    auto logger_factory = FileAndStdoutLoggerFactory(report_path, false);
    Logger logger = logger_factory.get_logger();

    auto gridCollection = std::make_shared<GridCollection>(path_to_data / "grid.csv");

    auto writer = std::make_shared<Output::JsonWriter>(std::make_shared<Clock>(),
                                                       path_to_data / "output.json");

    Output::VariationDeNiveauxDeStockData res;
    for (auto& grid: gridCollection->gridDefinitions)
    {
        auto evaluator = GridEvaluator(logger,
                                       writer,
                                       path_to_data,
                                       grid,
                                       ProblemsFormat::MPS_FILE,
                                       num_threads);
        res[grid.gridID] = evaluator.ComputeRewards();
    }

    writer->write_VariationDeNiveauxDeStock(res);
    writer->dump();

    return 0;
}

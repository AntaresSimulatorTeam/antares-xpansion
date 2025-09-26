
#include "antares-xpansion/benders/factories/LoggerFactories.h"
#include "antares-xpansion/benders/output/JsonWriter.h"
#include "antares-xpansion/grid_evaluator/GridEvaluator.h"

std::optional<ProblemsFormat> parseFormat(const std::string& format)
{
    if (format == "MPS")
    {
        return ProblemsFormat::MPS_FILE;
    }
    else if (format == "SAVED")
    {
        return ProblemsFormat::MPS_FILE;
    }
    return std::nullopt;
}

int main(int argc, char** argv)
{
    // Vérifier si le nombre d'arguments est suffisant pour le chemin obligatoire
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <path_to_data> [--pb_format <format>]" << std::endl;
        return 1;
    }

    // Récupérer le chemin à partir du premier argument
    std::filesystem::path path_to_data = argv[1];
    std::optional<ProblemsFormat> pb_format;
    int num_threads = 1;

    // Parcourir les arguments
    for (int i = 2; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--pb_format")
        {
            if (i + 1 < argc)
            {
                pb_format = parseFormat(argv[i + 1]);
                if (!pb_format)
                {
                    std::cerr << "Erreur: Format non reconnu. Utilisez MPS ou SAVED." << std::endl;
                    return 1;
                }
                ++i; // Sauter l'argument suivant
            }
            else
            {
                std::cerr << "Erreur: --pb_format nécessite un argument." << std::endl;
                return 1;
            }
        }
        else if (arg == "--threads")
        {
            if (i + 1 < argc)
            {
                try
                {
                    num_threads = std::stoi(argv[i + 1]);
                    if (num_threads <= 0)
                    {
                        throw std::invalid_argument("Number of threads must be positive.");
                    }
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Erreur: --threads nécessite un nombre valide. " << e.what()
                              << std::endl;
                    return 1;
                }
                ++i; // Sauter l'argument suivant
            }
            else
            {
                std::cerr << "Erreur: --threads nécessite un argument." << std::endl;
                return 1;
            }
        }
    }

    std::ofstream loggerFile("report.txt");
    loggerFile.close();
    auto logger_factory = FileAndStdoutLoggerFactory(path_to_data / "report.txt", false);
    Logger logger = logger_factory.get_logger();
    auto json_file_name = path_to_data / "output.json";
    std::shared_ptr<Output::JsonWriter> writer = std::make_shared<Output::JsonWriter>(
      std::make_shared<Clock>(),
      json_file_name);

    auto valeurs_usage = GridEvaluator(logger,
                                       writer,
                                       path_to_data,
                                       pb_format.value_or(ProblemsFormat::MPS_FILE));

    valeurs_usage.setThreads(num_threads);
    valeurs_usage.launch();
}

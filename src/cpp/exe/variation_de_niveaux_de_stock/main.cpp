
#include "antares-xpansion/benders/factories/LoggerFactories.h"
#include "antares-xpansion/benders/output/JsonWriter.h"
#include "antares-xpansion/variation_de_niveaux_de_stock/VariationDeNiveauxDeStock.h"

int main(int argc, char** argv)
{
    auto path_to_data = std::filesystem::current_path();
    if (argc > 1)
    {
        path_to_data = std::filesystem::path(argv[1]);
    }

    std::ofstream loggerFile("report.txt");
    loggerFile.close();
    auto logger_factory = FileAndStdoutLoggerFactory(path_to_data / "report.txt", false);
    Logger logger = logger_factory.get_logger();
    auto json_file_name = path_to_data / "output.json";
    std::shared_ptr<Output::JsonWriter> writer = std::make_shared<Output::JsonWriter>(
      std::make_shared<Clock>(),
      json_file_name);

    auto valeurs_usage = ValeursUsage(logger, writer, path_to_data);
    valeurs_usage.launch();
}

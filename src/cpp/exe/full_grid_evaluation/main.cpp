
#include <iostream>

#include "antares-xpansion/benders/factories/LoggerFactories.h"
#include "antares-xpansion/grid_evaluator/GridCollection.h"
#include "antares-xpansion/grid_evaluator/GridEvaluator.h"
#include "antares-xpansion/grid_evaluator/ReservoirManagement.h"
#include "antares-xpansion/lpnamer/main/ProblemGenerationExeOptions.h"
#include "antares-xpansion/lpnamer/main/ProblemGenerationForWaterValueCalculation.h"
#include "antares-xpansion/lpnamer/problem_modifier/XpansionProblemsFromAntaresProvider.h"
#include "malloc.h"

std::vector<double> interpolateVector(std::vector<double> values, int size)
{
    std::vector<double> interpolatedValues(size);
    return interpolatedValues;
}

void saveBellmanValues(const std::filesystem::path& path,
                       const std::vector<std::vector<double>> bellmanValues)
{
    std::ofstream file(path);
    for (const auto& weekValues: bellmanValues)
    {
        // interpolate the bellman values of a week from n to 101
        // using linear interpolation
        auto interpolatedValues = interpolateVector(weekValues, 101);
        for (const auto& value: interpolatedValues)
        {
            file << value << " ";
        }
        file << "\n";
    }
}

int main(int argc, char** argv)
{
    try
    {
        auto options_parser = ProblemGenerationExeOptions();
        options_parser.Parse(argc, argv);
        auto path_to_data = options_parser.StudyPath();

        auto gridCollection = std::make_shared<GridCollection>(path_to_data / "grid.csv");

        Reservoir reservoir(path_to_data, "area");
        ReservoirManagement reservoir_management(reservoir, true);

        ConfigurationManager configuration_manager(options_parser);
        auto report_path = path_to_data / "report.txt";
        auto logger_factory = FileAndStdoutLoggerFactory(report_path, false);
        Logger logger = logger_factory.get_logger();
        auto writer = std::make_shared<Output::JsonWriter>(std::make_shared<Clock>(),
                                                           path_to_data / "output.json");

        std::cout << "Generating problems" << std::endl;
        ProblemGenerationForWaterValueCalculation pbg(configuration_manager.Directories());
        std::cout << "Problems generated" << std::endl;

        Output::VariationDeNiveauxDeStockData variationDeNiveauxDeStockData;
        for (auto& grid: gridCollection->gridDefinitions)
        {
            auto mps_path = pbg.updateProblems(grid);

            auto evaluator = GridEvaluator(logger,
                                           writer,
                                           mps_path,
                                           grid,
                                           reservoir_management,
                                           ProblemsFormat::MPS_FILE,
                                           8);
            variationDeNiveauxDeStockData[grid.gridID] = evaluator.ComputeRewards();

            auto bellmanValues = evaluator.ComputeBellmanValues();
            saveBellmanValues(path_to_data / "bellman_values.csv", bellmanValues);
        }

        writer->write_VariationDeNiveauxDeStock(variationDeNiveauxDeStockData);
        writer->dump();

        return 0;
    }
    catch (std::exception& e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Exception of unknown type!" << std::endl;
    }

    return 0;
}


#include <iostream>

#include "antares-xpansion/bellman_values/BellmanValues.h"
#include "antares-xpansion/bellman_values/BellmanValuesExeOptions.h"
#include "antares-xpansion/bellman_values/ReservoirManagement.h"
#include "antares-xpansion/benders/factories/LoggerFactories.h"
#include "antares-xpansion/grid_evaluator/GridEvaluator.h"
#include "antares-xpansion/lpnamer/main/ProblemGenerationForWaterValueCalculation.h"
#include "antares-xpansion/lpnamer/problem_modifier/XpansionProblemsFromAntaresProvider.h"
#include "malloc.h"

std::vector<double> interpolateVector(const std::vector<double>& originalValues, int targetSize)
{
    std::vector<double> result(targetSize);
    int originalSize = static_cast<int>(originalValues.size());

    if (originalSize == 0 || targetSize == 0)
    {
        return result;
    }

    // Cas particulier : un seul point
    if (originalSize == 1)
    {
        std::fill(result.begin(), result.end(), originalValues[0]);
        return result;
    }

    for (int i = 0; i < targetSize; ++i)
    {
        // Position correspondante dans le vecteur d'origine
        double positionInOriginal = static_cast<double>(i) * (originalSize - 1) / (targetSize - 1);

        // Indices entourant cette position
        int lowerIndex = static_cast<int>(std::floor(positionInOriginal));
        int upperIndex = std::min(lowerIndex + 1, originalSize - 1);

        double fraction = positionInOriginal - lowerIndex;

        // Interpolation linéaire
        double interpolated = (1.0 - fraction) * originalValues[lowerIndex]
                              + fraction * originalValues[upperIndex];

        result[i] = interpolated;
    }

    return result;
}

void saveBellmanValues(const std::filesystem::path& path,
                       const std::vector<std::vector<double>>& bellmanValues,
                       bool usingAntaresFormat = false)
{
    std::ofstream file(path);
    if (!file)
    {
        std::cerr << "Failed to open file: " << path << std::endl;
        return;
    }

    for (const auto& weekValues: bellmanValues)
    {
        std::vector<double> values = weekValues;
        if (usingAntaresFormat)
        {
            values = interpolateVector(weekValues, 101);
        }
        for (const auto& value: values)
        {
            file << value << " ";
        }
        file << '\n';
    }
}

int main(int argc, char** argv)
{
    try
    {
        auto optionsParser = BellmanValuesExeOptions();
        optionsParser.Parse(argc, argv);
        auto studyPath = optionsParser.StudyPath();
        auto solverName = optionsParser.SolverName();
        int nbThreads = optionsParser.NbThreads();

        auto gridCollection = std::make_shared<GridCollection>(studyPath / "grid.csv");

        Reservoir reservoir(studyPath, "area");
        ReservoirManagement reservoirManagement(reservoir, true);

        ConfigurationManager::ConfigDirectories directories{
            study_dir: studyPath,
            simulation_dir: ConfigurationManager::generateOutputName(studyPath),
        };

        auto loggerFactory = FileAndStdoutLoggerFactory(directories.simulation_dir / "log.txt",
                                                        false);
        Logger logger = loggerFactory.get_logger();
        auto writer = std::make_shared<Output::JsonWriter>(std::make_shared<Clock>(),
                                                           studyPath / "output.json");

        std::cout << "Generating problems" << std::endl;
        ProblemGenerationForWaterValueCalculation pbg(directories);
        std::cout << "Problems generated" << std::endl;

        Output::VariationDeNiveauxDeStockData variationDeNiveauxDeStockData;
        for (auto& grid: gridCollection->gridDefinitions)
        {
            auto mpsPath = pbg.updateProblems(grid);

            auto evaluator = GridEvaluator(logger,
                                           writer,
                                           mpsPath,
                                           grid,
                                           ProblemsFormat::MPS_FILE,
                                           solverName,
                                           nbThreads);

            auto bellmanValues = BellmanValues(evaluator, reservoirManagement).compute();
            std::string fileName = std::to_string(grid.gridID) + "_bellman_values.csv";
            saveBellmanValues(directories.simulation_dir / fileName, bellmanValues, true);
        }

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


#include <iostream>

#include "antares-xpansion/bellman_values/BellmanValues.h"
#include "antares-xpansion/bellman_values/BellmanValuesExeOptions.h"
#include "antares-xpansion/bellman_values/PenaltiesConfigReader.h"
#include "antares-xpansion/benders/factories/LoggerFactories.h"
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

std::vector<std::vector<double>> interpolateWeekVector(
  const std::vector<std::vector<double>>& originalValues,
  int targetSize)
{
    std::vector<std::vector<double>> interpolatedValues;
    for (const auto& weekValues: originalValues)
    {
        interpolatedValues.push_back(interpolateVector(weekValues, targetSize));
    }
    return interpolatedValues;
}

void saveValues(const std::filesystem::path& path,
                const std::vector<std::vector<double>>& values,
                bool usingAntaresFormat = false)
{
    std::ofstream file(path);
    if (!file)
    {
        std::cerr << "Failed to open file: " << path << std::endl;
        return;
    }

    if (usingAntaresFormat)
    {
        // padding with a line of 0 for each level
        for (int levelIdx = 0; levelIdx < values[0].size(); ++levelIdx)
        {
            file << "0\t";
        }
        file << '\n';
    }
    for (const auto& weekValues: values)
    {
        std::vector<double> values = weekValues;
        if (usingAntaresFormat)
        {
            for (size_t i = 0; i < 7; i++)
            {
                for (const auto& value: values)
                {
                    file << value << '\t';
                }
                if (i != 6)
                {
                    file << '\n';
                }
            }
        }
        else
        {
            for (const auto& value: values)
            {
                file << value << " ";
            }
        }
        file << '\n';
    }
}

std::vector<std::vector<double>> computeWaterValues(
  const std::vector<std::vector<double>>& bellmanValues,
  const std::vector<double>& levels)
{
    if (bellmanValues.empty())
    {
        return {};
    }

    size_t numLevels = levels.size();
    size_t numWeeks = bellmanValues.size() - 1;

    for (const auto& weekVals: bellmanValues)
    {
        if (weekVals.size() != numLevels)
        {
            throw std::invalid_argument("Inconsistent level size in bellmanValues");
        }
    }

    std::vector<std::vector<double>> derivatives(numWeeks, std::vector<double>(numLevels - 1));

    for (size_t week = 1; week <= numWeeks; ++week)
    {
        const auto& values = bellmanValues[week];
        for (size_t i = 0; i < numLevels - 1; ++i)
        {
            // Take the opposite of the derivative to have positive water values
            derivatives[week - 1][i] = -(values[i + 1] - values[i]) / (levels[i + 1] - levels[i]);
        }
    }

    return derivatives;
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
        int startWeek = optionsParser.StartWeek();
        int endWeek = optionsParser.EndWeek();
        int nbLevels = optionsParser.NbLevels();
        bool antaresFormat = optionsParser.AntaresFormat();
        bool writePbFiles = optionsParser.WritePbFiles();
        const std::string problemFormat = optionsParser.ProblemFormat();

        auto gridCollection = std::make_shared<GridCollection>(studyPath / "grid.csv");

        const std::filesystem::path penaltiesConfigFilePath(studyPath / "penalties.yaml");

        // PenaltiesConfigReader will check whether the file exists and return default values if
        // needed
        PenaltiesConfigReader pcr(penaltiesConfigFilePath);

        ReservoirManagement reservoirManagement(gridCollection->reservoirs.begin()->second,
                                                pcr.getPenaltyBottomRuleCurve(),
                                                pcr.getPenaltyUpperRuleCurve(),
                                                pcr.getPenaltyFinalLevel(),
                                                pcr.getForceFinalLevel(),
                                                pcr.getFinalLevel(),
                                                pcr.getOverflow());

        ConfigurationManager::ConfigDirectories directories{
          .study_dir = studyPath,
          .simulation_dir = ConfigurationManager::generateOutputName(studyPath),
        };

        auto loggerFactory = FileAndStdoutLoggerFactory(directories.simulation_dir / "log.txt",
                                                        false);
        Logger logger = loggerFactory.get_logger();

        std::cout << "Generating problems" << std::endl;
        ProblemGenerationForWaterValueCalculation pbg(directories,
                                                      reservoirManagement,
                                                      solverName,
                                                      startWeek,
                                                      endWeek,
                                                      writePbFiles,
                                                      problemFormat);
        std::cout << "Problems generated" << std::endl;

        Output::VariationDeNiveauxDeStockData variationDeNiveauxDeStockData;
        for (auto& grid: gridCollection->gridDefinitions)
        {
            auto problems = pbg.updateProblems(grid);

            auto evaluator = GridEvaluator(logger, problems, grid, solverName, nbThreads);
            auto bellmanValuesEvaluator = BellmanValues(evaluator, reservoirManagement);
            auto bellmanValues = bellmanValuesEvaluator.compute(nbLevels);
            auto levels = bellmanValuesEvaluator.getLevels();
            if (antaresFormat)
            {
                bellmanValues = interpolateWeekVector(bellmanValues, 101);
                levels = interpolateVector(levels, 101);
            }
            auto waterValues = computeWaterValues(bellmanValues, levels);
            std::string fileName = std::to_string(grid.gridID) + "_water_values.csv";
            saveValues(directories.simulation_dir / fileName, waterValues, antaresFormat);
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

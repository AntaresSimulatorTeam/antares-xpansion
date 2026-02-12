
#include "antares-xpansion/grid_evaluator/GridCollection.h"

#include <fstream>
#include <ranges>
#include <regex>

/// @brief Checks if a gridElement is valid
/// - 0.0 <= min <= 1.0 AND 0.0 <= max <= 1.0 AND min <= max
/// - min == max (only one point) OR 0.0 < step <= 1.0
/// @param min min relative value of the gridElement
/// @param max max relative value of the gridElement
/// @param step step used to go from min to max
/// @return true if the gridElement is valid, false otherwise
bool validateGridElement(double min, double max, double step)
{
    return (min >= 0.0 && min <= 1.0) && (max >= 0.0 && max <= 1.0) && (min <= max)
           && (min == max || (step > 0.0 && step <= 1.0));
}

/// @brief Add a gridElement for the current gridCollection
/// @param pbName ex : "problem-1-1--optim-nb-1", "all"
/// @param type "constraint" : this field is unused at the moment
/// @param cstName name of the constraint
/// @param areaName name of the area
/// @param min minimum relative value (min ∈ [0,1])
/// @param max maximum relative value (max ∈ [0,1])
/// @param step step used to go from min to max
void GridDefinition::addGridElement(const std::string& pbName,
                                    const std::string& type,
                                    const std::string& cstName,
                                    const std::string& areaName,
                                    double min,
                                    double max,
                                    double step)
{
    if (!validateGridElement(min, max, step))
    {
        throw std::invalid_argument("Invalid GridElement: "
                                    "min ∈ [0,1], max ∈ [0,1] & > min, step ∈ (0,1]");
    }

    gridElements.push_back({pbName, type, cstName, areaName, min, max, step});
}

/// @brief Build a GridCollection from a file
/// @param filePath
GridCollection::GridCollection(const std::filesystem::path& filePath,
                               Logger logger,
                               const std::optional<std::filesystem::path>& generalDataFilePath):
    logger(logger)
{
    // first of all: Read the general data file
    loadGeneralDataIni(generalDataFilePath.value_or(
      filePath.parent_path().parent_path().parent_path() / "settings/generaldata.ini"));

    // Read the grid.csv file
    std::ifstream grid_csv(filePath);
    if (!grid_csv.is_open())
    {
        throw std::runtime_error("Could not open grid.csv file");
    }

    std::string line;
    std::getline(grid_csv, line); // Skip header
    while (std::getline(grid_csv, line))
    {
        if (line.empty())
        {
            continue;
        }

        std::stringstream ss(line);
        std::string token;

        std::string tokens[8];
        int i = 0;

        while (std::getline(ss, token, ',') && i < 8)
        {
            tokens[i++] = token;
        }

        int gridID = std::stoi(tokens[0]);
        std::string pbName = tokens[1];
        std::string type = tokens[2];
        std::string cstName = tokens[3];
        std::string areaName = tokens[4];

        double min = std::stod(tokens[5]);
        double max = std::stod(tokens[6]);
        double step = std::stod(tokens[7]);

        if (!gridDefinitions.contains(gridID))
        {
            GridDefinition gridDef{gridID, {}, {}, {}};
            gridDefinitions.emplace(gridID, gridDef);
        }
        gridDefinitions.at(gridID).addGridElement(pbName, type, cstName, areaName, min, max, step);

        if (!reservoirs.contains(areaName))
        {
            loadReservoirManagement(filePath.parent_path().parent_path().parent_path(), areaName);
        }
    }
    // it is still required to set default reservoirs, which will generate grid values
    for (auto& gridDefinition: gridDefinitions | std::views::values)
    {
        gridDefinition.setReservoirs(reservoirs);
    }

    checkGridValidity();
}

/// @brief Load MC Years and active Years from the generaldata file
/// @param inputPath study path
void GridCollection::loadGeneralDataIni(const std::filesystem::path& inputPath)
{
    // TODO: modify GeneralDataIniReader() to take any ILoggerXpansion-derived logger
    // this will crash if attempting to log in the case of an error
    auto generalDataReader = GeneralDataIniReader(inputPath, nullptr);
    activeYears = generalDataReader.GetActiveYears();
    mcYears = generalDataReader.GetNbYears();
}

/// @brief Load a ReservoirManagement from a study path and an area
/// @param studyPath path of the input file
/// @param area name of the area
void GridCollection::loadReservoirManagement(const std::filesystem::path& studyPath,
                                             const std::string& area)
{
    Reservoir reservoir(studyPath, area);
    // this reservoir contains raw data read from files; it must be modified to take into account
    // the number of MC years
    if (reservoir.inflow[0].size() == 0)
    {
        // initialize values to 0 for all MC years
        logger->display_message("No inflow data was read, values of zero are assumed.",
                                LogUtils::LOGLEVEL::INFO,
                                logger->CONTEXT);
        reservoir.inflow.assign(Reservoir::weeks_in_year, std::vector<double>(mcYears, 0.0));
    }
    else if (reservoir.inflow[0].size() == 1)
    {
        // copy this time series to all possible MC years
        logger->display_message(
          "Inflow values were found for a single year; these values will be used for all MC years.",
          LogUtils::LOGLEVEL::INFO,
          logger->CONTEXT);
        std::vector<double> inflowToCopy = reservoir.inflow[0];
        reservoir.inflow.assign(Reservoir::weeks_in_year, inflowToCopy);
    }
    else if (reservoir.inflow[0].size() < mcYears)
    {
        // error
        throw std::domain_error("ERROR: mismatch between inflow data ("
                                + std::to_string(reservoir.inflow[0].size())
                                + ") and number of MC years (" + std::to_string(mcYears) + ")");
    }

    reservoirs.emplace(area, reservoir);
}

void GridCollection::checkGridValidity() const
{
    for (auto& grid: gridDefinitions | std::views::values)
    {
        if (grid.gridElements.size() > 1)
        {
            throw std::domain_error(
              "Water values can currently only be computed for one gridElement per gridId.");
        }
    }
}

/// @brief Generate Grid values for all gridElements
void GridDefinition::generateGridValues()
{
    // weekAreaConstraints.clear();
    for (auto& gridElement: gridElements)
    {
        adjustBoundaryValues(gridElement);
        processGridElementWeeks(gridElement);
    }
}

std::optional<int> GridDefinition::parseWeekFromProblem(const std::string& problemName) const
{
    std::regex pattern(R"(problem-(\d+)-(\d+)(?:--.*)?)");
    std::smatch matches;
    if (std::regex_match(problemName, matches, pattern))
    {
        return std::stoi(matches[2]); // Return week
    }
    return std::nullopt;
}

double GridDefinition::interpolate(double min, double max, double normalized) const
{
    return min + (max - min) * normalized;
}

std::vector<double> GridDefinition::generateRhsValues(const GridElement& gridElement,
                                                      double minConstraint,
                                                      double maxConstraint) const
{
    std::vector<double> values;
    bool isFixedValue = gridElement.min == gridElement.max;

    if (isFixedValue)
    {
        values.push_back(interpolate(minConstraint, maxConstraint, gridElement.min));
    }
    else
    {
        int steps = static_cast<int>((gridElement.max - gridElement.min) / gridElement.step);
        for (int i = 0; i <= steps; ++i)
        {
            double normalizedValue = gridElement.min + i * gridElement.step;
            values.push_back(interpolate(minConstraint, maxConstraint, normalizedValue));
        }
    }

    return values;
}

void GridDefinition::processWeek(GridElement& gridElement, size_t week)
{
    const auto& reservoir = reservoirs.at(gridElement.area);
    double minConstraint = -reservoir.max_pumping[week - 1] * reservoir.efficiency;
    double maxConstraint = reservoir.max_generating[week - 1];

    gridElement.rhsValues[week - 1] = generateRhsValues(gridElement, minConstraint, maxConstraint);

    weekAreaConstraints[week][gridElement.area].emplace(gridElement.name,
                                                        gridElement.rhsValues[week - 1]);
}

void GridDefinition::processAllWeeks(GridElement& gridElement)
{
    for (size_t week = 1; week <= Reservoir::weeks_in_year; week++)
    {
        processWeek(gridElement, week);
    }
}

void GridDefinition::processGridElementWeeks(GridElement& gridElement)
{
    if (gridElement.problemName == "all")
    {
        processAllWeeks(gridElement);
    }
    else if (auto week = parseWeekFromProblem(gridElement.problemName))
    {
        processWeek(gridElement, *week);
    }
}

void GridDefinition::adjustBoundaryValues(GridElement& gridElement)
{
    constexpr double epsilon = 0;

    if (gridElement.min == 0.0)
    {
        gridElement.min += epsilon;
    }
    if (gridElement.max == 1.0)
    {
        gridElement.max -= epsilon;
    }
}

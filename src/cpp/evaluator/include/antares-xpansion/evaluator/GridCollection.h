#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "antares-xpansion/evaluator/ReservoirManagement.h"
#include "antares-xpansion/lpnamer/input_reader/GeneralDataReader.h"
#include "antares-xpansion/xpansion_interfaces/ILogger.h"

/// @brief key area name, value constraint map
static enum WEEK
{
    ALLWEEKS = -1
};

using Week = int;
using AreaName = std::string;
using ConstraintName = std::string;

/// @brief key constraint name, value vector of rhs values
using ConstraintMap = std::map<ConstraintName, std::vector<double>&>;
/// @brief key area name, value constraint map
using AreaConstraintMaps = std::map<AreaName, ConstraintMap>;

/// @brief Element of a grid
struct GridElement
{
    std::string problemName;           // name of the problem
    [[maybe_unused]] std::string type; // field unused at the moment as we only treat constraints
    std::string name;                  // name of the constraint
    std::string area;                  // name of the area
    double min;                        // min relative value
    double max;                        // max relative value
    double step;                       // step used to go from min to max

    std::vector<std::vector<double>> rhsValues = std::vector<std::vector<double>>(
      Reservoir::weeks_in_year,
      std::vector<double>{}); // RHS values computed from the gridElement attributes
};

/// @brief Contains multiple gridElements
struct GridDefinition
{
    int gridID;
    std::string area;                            // name of the area
    std::map<std::string, Reservoir> reservoirs; // in the case of multistock, each gridDefinition
                                                 // needs its own copy of the reservoirs that will
                                                 // be modified as the computation goes
    std::map<Week, GridElement> gridElements;
    std::map<Week, AreaConstraintMaps>
      weekAreaConstraints; // key week, value map (key area name, value vector of rhs values)

    void generateGridValues();

    void setReservoirs(const std::map<std::string, Reservoir>& reservoirs)
    {
        this->reservoirs = reservoirs;
        generateGridValues();
    }

    std::vector<std::vector<double>> getRhsValuesForWeek(size_t week) const;

    void addGridElement(const std::string& pbName,
                        const std::string& type,
                        const std::string& cstName,
                        const std::string& areaName,
                        double min,
                        double max,
                        double step);

private:
    std::optional<int> parseWeekFromProblem(const std::string& problemName) const;
    double interpolate(double min, double max, double normalized) const;
    std::vector<double> generateRhsValues(const GridElement& gridElement,
                                          double minConstraint,
                                          double maxConstraint) const;
    void processWeek(GridElement& gridElement, size_t week);
    void processAllWeeks(GridElement& gridElement);
    void processGridElementWeeks(GridElement& gridElement);
    Week gridDefinitionKeyForProblem(std::string pbName) const;
    void adjustBoundaryValues(GridElement& gridElement);
};

class GridCollection
{
public:
    GridCollection(const std::filesystem::path& filePath,
                   Logger logger,
                   const std::optional<std::filesystem::path>& generalDataFilePath = std::nullopt);

    std::map<int, GridDefinition> gridDefinitions;
    std::map<AreaName, Reservoir> reservoirs;
    int mcYears;
    std::vector<int> activeYears;

private:
    void loadReservoirManagement(const std::filesystem::path& studyPath, const std::string& area);
    void loadGeneralDataIni(const std::filesystem::path& inputPath);
    void checkGridValidity() const;

    Logger logger;
};

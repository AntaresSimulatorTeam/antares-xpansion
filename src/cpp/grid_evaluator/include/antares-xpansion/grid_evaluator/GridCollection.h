#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "antares-xpansion/grid_evaluator/ReservoirManagement.h"

/// @brief key area name, value constraint map
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
    const std::map<std::string, Reservoir>& reservoirs;
    std::vector<GridElement> gridElements;
    std::map<Week, AreaConstraintMaps>
      weekAreaConstraints; // key week, value map (key area name, value vector of rhs values)

    void generateGridValues();
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
    void adjustBoundaryValues(GridElement& gridElement);
};

class GridCollection
{
    void loadReservoirManagement(const std::filesystem::path& studyPath, const std::string& area);

public:
    GridCollection(const std::filesystem::path& filePath);

    std::map<int, GridDefinition> gridDefinitions;
    std::map<AreaName, Reservoir> reservoirs;
};

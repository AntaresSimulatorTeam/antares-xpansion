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

struct GridElement
{
    std::string problemName;
    std::string type;
    std::string name;
    std::string area;
    double min;
    double max;
    double step;

    std::vector<std::vector<double>> rhsValues = std::vector<std::vector<double>>(
      Reservoir::weeks_in_year,
      std::vector<double>{});
};

struct GridDefinition
{
    int gridID;
    const std::map<std::string, Reservoir>& reservoirs;
    std::vector<GridElement> gridElements;
    std::map<Week, AreaConstraintMaps>
      weekAreaConstraints; // key week, value map (key area name, value vector of rhs values)

    bool isSubproblemUsed(const std::string& subPbName) const;
    void generateGridValues();
};

class GridCollection
{
    void loadReservoirManagement(const std::filesystem::path& studyPath, const std::string& area);

public:
    GridCollection(const std::filesystem::path& filePath);

    std::vector<GridDefinition> gridDefinitions;
    std::map<AreaName, Reservoir> reservoirs;
};

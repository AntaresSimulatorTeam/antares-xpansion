
#include "antares-xpansion/grid_evaluator/GridCollection.h"

#include <fstream>
#include <iostream>

GridCollection::GridCollection(const std::filesystem::path& filePath)
{
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

        if (gridID <= gridDefinitions.size())
        {
            gridDefinitions.push_back({gridID, {}, {}, {}});
        }
        gridDefinitions[gridID].gridElements.push_back(
          {pbName, type, cstName, areaName, min, max, step});

        if (!reservoirs.contains(areaName))
        {
            loadReservoirManagement(filePath.parent_path(), areaName);
        }
    }
    // it is still required to set default reservoirs and generate grid values
    for (auto& gridDefinition: gridDefinitions)
    {
        gridDefinition.setReservoirs(reservoirs);
    }
}

void GridCollection::loadReservoirManagement(const std::filesystem::path& studyPath,
                                             const std::string& area)
{
    reservoirs.emplace(area, Reservoir(studyPath, area));
}

void GridDefinition::generateGridValues()
{
    for (auto& gridElement: gridElements)
    {
        // constexpr double epsilon = 1e-6;
        constexpr double epsilon = 0;

        if (gridElement.min == 0.0)
        {
            gridElement.min += epsilon;
        }
        if (gridElement.max == 1.0)
        {
            gridElement.max -= epsilon;
        }

        for (size_t week = 1; week <= Reservoir::weeks_in_year; week++)
        {
            double min_cst = -reservoirs.at(gridElement.area).max_pumping[week - 1]
                             * reservoirs.at(gridElement.area).efficiency;
            double max_cst = reservoirs.at(gridElement.area).max_generating[week - 1];

            int steps = static_cast<int>((gridElement.max - gridElement.min) / gridElement.step);

            for (int i = 0; i <= steps; ++i)
            {
                double normalized = gridElement.min + i * gridElement.step;
                double value = min_cst + (max_cst - min_cst) * normalized;
                gridElement.rhsValues[week - 1].push_back(value);
            }
            weekAreaConstraints[week][gridElement.area].emplace(gridElement.name,
                                                                gridElement.rhsValues[week - 1]);
        }
    }
}

bool GridDefinition::isSubproblemUsed(const std::string& subPbName) const
{
    for (const auto& gridElement: gridElements)
    {
        if (gridElement.problemName == subPbName || gridElement.problemName == "all")
        {
            return true;
        }
    }

    return false;
}

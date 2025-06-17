
#include "antares-xpansion/grid_evaluator/GridCollection.h"

#include <fstream>

GridCollection::GridCollection(const std::filesystem::path& path_to_file)
{
    // Read the grid.csv file
    std::ifstream grid_csv(path_to_file);
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

        std::string tokens[11]; // assuming at least 11 fields
        int i = 0;

        while (std::getline(ss, token, ',') && i < 11)
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

        std::string minCst = tokens[8];
        std::string maxCst = tokens[9];
        double minEfficiency = std::stod(tokens[10]);

        if (gridID <= gridDefinitions.size())
        {
            gridDefinitions.push_back({gridID, {}});
        }
        gridDefinitions[gridID].gridElements.push_back(
          {pbName, type, cstName, areaName, min, max, step, minCst, maxCst, minEfficiency});
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

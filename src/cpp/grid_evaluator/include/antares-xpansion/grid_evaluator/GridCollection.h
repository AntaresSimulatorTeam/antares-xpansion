#pragma once

#include <filesystem>
#include <string>
#include <vector>

struct GridElement
{
    std::string problemName;
    std::string type;
    std::string name;
    std::string area;
    double min;
    double max;
    double step;
    std::string min_cst;
    std::string max_cst;
    double min_efficiency;
};

struct GridDefinition
{
    int gridID;
    std::vector<GridElement> gridElements;
    bool isSubproblemUsed(const std::string& subPbName) const;
};

class GridCollection
{
public:
    GridCollection(const std::filesystem::path& path_to_file);

    std::vector<GridDefinition> gridDefinitions;
};

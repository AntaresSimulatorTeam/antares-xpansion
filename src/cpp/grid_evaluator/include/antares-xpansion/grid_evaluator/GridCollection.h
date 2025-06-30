#pragma once

#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <vector>

struct ScenarioAndWeek
{
    int scenario;
    int week;

    bool operator<(const ScenarioAndWeek& other) const
    {
        return std::tie(scenario, week) < std::tie(other.scenario, other.week);
    }
};

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

    std::map<ScenarioAndWeek, std::set<double>> values;
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


#include "antares-xpansion/grid_evaluator/ReservoirManagement.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "antares-xpansion/grid_evaluator/Interpolator.h"

/// @brief Load a reservoir from path of the study and the name of the area
/// @param path_to_input path of the study
/// @param areaName name of the area
Reservoir::Reservoir(const std::filesystem::path& path_to_input, const std::string& areaName):
    area(areaName),
    capacity(0.0),
    efficiency(0.0)
{
    loadHydroIni(path_to_input / "input/hydro/hydro.ini");
    loadRuleCurves(path_to_input);
    loadInflow(path_to_input);
    loadMaxPower(path_to_input);
}

/// @brief Parse a double from a string and return it, if the field is invalid it will throw an
/// exception
/// @param str string to parse
/// @param field_name name of the field to parse (used only for debugging purposes)
/// @return the parsed double value
double parse_double(const std::string& str, const std::string& field_name)
{
    try
    {
        return std::stod(str);
    }
    catch (const std::exception&)
    {
        throw std::runtime_error("Invalid " + field_name + " value: '" + str
                                 + "' (expected a double)");
    }
}

/// @brief Parse a bool from a string and return it, if the field is invalid it will throw an
/// exception
/// @param str string to parse
/// @param field_name name of the field
/// @return the parsed bool value
bool parse_bool(std::string str, const std::string& field_name)
try
{
    std::transform(str.begin(),
                   str.end(),
                   str.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    if (str == "true")
    {
        return true;
    }
    if (str == "false")
    {
        return false;
    }
    throw 1;
}
catch (...)
{
    throw std::runtime_error("Invalid " + field_name + " value: '" + str
                             + "' (expected true or false)");
}

/// @brief Load Hydro from hydro.ini file path
/// @param ini_path hydro.ini file path to load
void Reservoir::loadHydroIni(const std::filesystem::path& ini_path)
{
    std::ifstream file(ini_path);
    if (!file)
    {
        throw std::runtime_error("Could not open : " + ini_path.string() + " while loading Hydro");
    }

    std::string line, current_section;
    bool found_capacity = false;
    bool found_efficiency = false;
    // computing water values requires that the area uses heuristic and not water
    bool use_water = true;      // expected: false
    bool use_heuristic = false; // expected: true

    while (std::getline(file, line))
    {
        // Remove trailing '\r' if it exists (handles Windows)
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        // Trim left
        line.erase(0, line.find_first_not_of(" \t"));
        if (line.empty() || line[0] == ';' || line[0] == '#')
        {
            continue;
        }

        if (line.front() == '[' && line.back() == ']')
        {
            current_section = line.substr(1, line.size() - 2);
            continue;
        }

        auto pos = line.find('=');
        if (pos != std::string::npos)
        {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            // Trim key and value
            key.erase(key.find_last_not_of(" \t") + 1);
            key.erase(0, key.find_first_not_of(" \t"));
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);

            if (key == area)
            {
                if (current_section == "reservoir capacity")
                {
                    capacity = parse_double(value, "capacity");
                    found_capacity = true;
                }
                else if (current_section == "pumping efficiency")
                {
                    efficiency = parse_double(value, "efficiency");
                    found_efficiency = true;
                }
                else if (current_section == "use water")
                {
                    use_water = parse_bool(value, "use water");
                }
                else if (current_section == "use heuristic")
                {
                    use_heuristic = parse_bool(value, "use heuristic");
                }
            }
        }
    }

    if (!found_capacity)
    {
        throw std::runtime_error("Missing capacity for area: " + area);
    }
    if (!found_efficiency)
    {
        throw std::runtime_error("Missing efficiency for area: " + area);
    }
    // computing water values requires that the area uses heuristic and not water
    if (!use_heuristic || use_water)
    {
        throw std::runtime_error("Area " + area
                                 + " should define [use water] as False and [use heuristic] as "
                                   "True in hydro.ini to compute water values.");
    }
}

/// @brief Load rule curves from study path
/// @param dir_study study path
void Reservoir::loadRuleCurves(const std::filesystem::path& dir_study)
{
    auto path = dir_study / ("input/hydro/common/capacity/reservoir_" + area + ".txt");
    std::ifstream file(path);
    if (!file)
    {
        throw std::runtime_error("Could not open rule curve file loacated at : " + path.string());
    }

    std::vector<std::vector<double>> rule_curves;
    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::vector<double> row;
        double val;
        while (iss >> val)
        {
            row.push_back(val);
        }
        if (row.size() >= 3)
        {
            rule_curves.push_back({row[0] * capacity, row[2] * capacity});
        }
    }

    if (rule_curves.empty())
    {
        throw std::runtime_error("Empty rule curve file");
    }

    if (rule_curves[0][0] != rule_curves[0][1])
    {
        throw std::runtime_error(
          "Initial level is not correctly defined by bottom and upper rule curves");
    }

    initial_level = rule_curves[0][0];

    for (size_t i = 0; i < rule_curves.size(); i += 7)
    {
        bottom_rule_curve.push_back(rule_curves[i][0]);
        upper_rule_curve.push_back(rule_curves[i][1]);
    }
}

/// @brief Load inflow from study path
/// @param dir_study study path
void Reservoir::loadInflow(const std::filesystem::path& dir_study)
{
    auto path = dir_study / ("input/hydro/series/" + area + "/mod.txt");
    std::ifstream file(path);
    if (!file)
    {
        throw std::runtime_error("Could not open inflow file");
    }

    std::vector<std::vector<double>> daily_inflow;
    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::vector<double> row;
        double val;
        while (iss >> val)
        {
            row.push_back(val);
        }
        if (!row.empty())
        {
            daily_inflow.push_back(row);
        }
    }

    if (daily_inflow.size() < days_in_year)
    {
        throw std::runtime_error("Not enough inflow data");
    }

    daily_inflow.resize(days_in_year);
    size_t nb_scenarios = daily_inflow[0].size();

    inflow.assign(weeks_in_year, std::vector<double>(nb_scenarios, 0.0));

    for (int w = 0; w < weeks_in_year; ++w)
    {
        for (int d = 0; d < days_in_week; ++d)
        {
            int idx = w * days_in_week + d;
            for (size_t s = 0; s < nb_scenarios; ++s)
            {
                inflow[w][s] += daily_inflow[idx][s];
            }
        }
    }
}

/// @brief Load max power from study path
/// @param dir_study study path
void Reservoir::loadMaxPower(const std::filesystem::path& dir_study)
{
    auto path = dir_study / ("input/hydro/common/capacity/maxpower_" + area + ".txt");
    std::ifstream file(path);
    if (!file)
    {
        throw std::runtime_error("Could not open maxpower file");
    }

    std::vector<std::vector<double>> data;
    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::vector<double> row;
        double val;
        while (iss >> val)
        {
            row.push_back(val);
        }
        if (!row.empty())
        {
            data.push_back(row);
        }
    }

    if (data.size() < days_in_year)
    {
        throw std::runtime_error("Not enough maxpower data");
    }

    data.resize(days_in_year);

    std::vector<std::vector<double>> daily_energy(days_in_year);
    for (size_t i = 0; i < days_in_year; ++i)
    {
        daily_energy[i].resize(data[i].size());
        std::transform(data[i].begin(),
                       data[i].end(),
                       daily_energy[i].begin(),
                       [](double v) { return v * hours_in_day; });
    }

    max_generating.assign(weeks_in_year, 0.0);
    max_pumping.assign(weeks_in_year, 0.0);

    for (int w = 0; w < weeks_in_year; ++w)
    {
        for (int d = 0; d < days_in_week; ++d)
        {
            int idx = w * days_in_week + d;
            if (daily_energy[idx].size() >= 3)
            {
                max_generating[w] += daily_energy[idx][0];
                max_pumping[w] += daily_energy[idx][2];
            }
        }
    }
}

ReservoirManagement::ReservoirManagement(Reservoir& reservoir,
                                         double penalty_bottom_rule_curve,
                                         double penalty_upper_rule_curve,
                                         double penalty_final_level,
                                         bool force_final_level,
                                         std::optional<double> final_level,
                                         bool overflow):
    reservoir(reservoir),
    penalty_bottom_rule_curve(penalty_bottom_rule_curve),
    penalty_upper_rule_curve(penalty_upper_rule_curve),
    penalty_final_level(penalty_final_level),
    force_final_level(force_final_level),
    final_level(final_level.value_or(reservoir.initial_level)),
    overflow(overflow)
{
}

/// @brief Get the penalty function, that can be evaluated by giving a level
/// @param week week to build the function for
/// @param len_week number of weeks
/// @return Return a function that takes a level as argument and return a penalty
std::function<double(double)> ReservoirManagement::get_penalty(int week, int len_week) const
{
    if (week == len_week && force_final_level)
    {
        std::vector<double> x = {0.0, final_level, reservoir.capacity};
        std::vector<double> y = {penalty_final_level * final_level,
                                 0.0,
                                 penalty_final_level * (reservoir.capacity - final_level)};
        return Interpolator::linearInterpolation(x, y);
    }
    else
    {
        std::vector<double> x = {0.0,
                                 reservoir.bottom_rule_curve[week],
                                 reservoir.upper_rule_curve[week],
                                 reservoir.capacity};
        std::vector<double> y = {penalty_bottom_rule_curve * reservoir.bottom_rule_curve[week],
                                 0.0,
                                 0.0,
                                 penalty_upper_rule_curve
                                   * (reservoir.capacity - reservoir.upper_rule_curve[week])};
        return Interpolator::linearInterpolation(x, y);
    }
}

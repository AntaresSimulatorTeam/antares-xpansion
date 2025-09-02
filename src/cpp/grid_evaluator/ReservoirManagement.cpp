
#include "antares-xpansion/grid_evaluator/ReservoirManagement.h"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "antares-xpansion/grid_evaluator/Interpolator.h"

Reservoir::Reservoir(const std::filesystem::path& path_to_input, const std::string& areaName):
    area(areaName),
    capacity(0.0),
    efficiency(0.0)
{
    loadHydroIni(path_to_input / "input/hydro/hydro.ini");
    readRuleCurves(path_to_input);
    readInflow(path_to_input);
    readMaxPower(path_to_input);
}

double parse_double(const std::string& str, const std::string& field_name)
{
    try
    {
        return std::stod(str);
    }
    catch (const std::exception&)
    {
        throw std::runtime_error("Invalid " + field_name + " value: '" + str + "'");
    }
}

void Reservoir::loadHydroIni(const std::filesystem::path& ini_path)
{
    std::ifstream file(ini_path);
    if (!file)
    {
        throw std::runtime_error("Could not open hydro.ini: " + ini_path.string());
    }

    std::string line, current_section;
    bool found_capacity = false;
    bool found_efficiency = false;

    while (std::getline(file, line))
    {
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
}

void Reservoir::readRuleCurves(const std::filesystem::path& dir_study)
{
    auto path = dir_study / ("input/hydro/common/capacity/reservoir_" + area + ".txt");
    std::ifstream file(path);
    if (!file)
    {
        throw std::runtime_error("Could not open rule curve file");
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

void Reservoir::readInflow(const std::filesystem::path& dir_study)
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

void Reservoir::readMaxPower(const std::filesystem::path& dir_study)
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

ReservoirManagement::ReservoirManagement(const Reservoir& reservoir,
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
    final_level(final_level ? final_level.value() : reservoir.initial_level),
    overflow(overflow)
{
}

std::function<double(double)> ReservoirManagement::get_penalty(int week, int len_week) const
{
    if (week == len_week && force_final_level)
    {
        std::vector<double> x = {0.0, final_level, reservoir.capacity};
        std::vector<double> y = {-penalty_final_level * final_level,
                                 0.0,
                                 -penalty_final_level * (reservoir.capacity - final_level)};
        return Interpolator::linearInterpolation(x, y);
    }
    else
    {
        std::vector<double> x = {0.0,
                                 reservoir.bottom_rule_curve[week - 1],
                                 reservoir.upper_rule_curve[week - 1],
                                 reservoir.capacity};
        std::vector<double> y = {penalty_bottom_rule_curve * reservoir.bottom_rule_curve[week - 1],
                                 0.0,
                                 0.0,
                                 penalty_upper_rule_curve
                                   * (reservoir.capacity - reservoir.upper_rule_curve[week - 1])};
        return Interpolator::linearInterpolation(x, y);
    }
}

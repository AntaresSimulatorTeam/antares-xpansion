#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

using IniMap = std::unordered_map<std::string,
                                  std::unordered_map<std::string, std::variant<double, bool>>>;

class Reservoir
{
public:
    static constexpr int weeks_in_year = 52;
    static constexpr int hours_in_week = 168;
    static constexpr int hours_in_day = 24;
    static constexpr int days_in_week = hours_in_week / hours_in_day;
    static constexpr int days_in_year = weeks_in_year * days_in_week;

    Reservoir(const std::filesystem::path& path_to_input, const std::string& areaName);

    std::string area;
    double capacity;
    double efficiency;
    std::vector<double> max_generating;
    std::vector<double> max_pumping;
    std::vector<std::vector<double>> inflow;

private:
    // void readRuleCurves(const std::filesystem::path& path_to_input);
    void readInflow(const std::filesystem::path& path_to_input);
    void readMaxPower(const std::filesystem::path& path_to_input);
    void loadHydroIni(const std::filesystem::path& path_to_input);
    std::vector<double> loadInflow(const std::filesystem::path& path_to_input,
                                   const std::string& areaName);
};

class ReservoirManagement
{
public:
    ReservoirManagement(const Reservoir& reservoir, bool overflow);

    Reservoir reservoir;
    bool overflow;
};

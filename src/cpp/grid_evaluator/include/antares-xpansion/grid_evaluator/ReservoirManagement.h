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

    Reservoir(const std::filesystem::path& inputPath, const std::string& areaName);

    std::string area;
    double capacity;
    double efficiency;
    std::vector<double> max_generating;
    std::vector<double> max_pumping;
    std::vector<std::vector<double>> inflow;

private:
    // void readRuleCurves(const std::filesystem::path& inputPath);
    void readInflow(const std::filesystem::path& inputPath);
    void readMaxPower(const std::filesystem::path& inputPath);
    void loadHydroIni(const std::filesystem::path& inputPath);
    std::vector<double> loadInflow(const std::filesystem::path& inputPath,
                                   const std::string& areaName);

    // Private test-only constructor
    Reservoir(const std::string& areaName,
              double capacity,
              double efficiency,
              std::vector<double> max_generating,
              std::vector<double> max_pumping,
              std::vector<std::vector<double>> inflow):
        area(areaName),
        capacity(capacity),
        efficiency(efficiency),
        max_generating(std::move(max_generating)),
        max_pumping(std::move(max_pumping)),
        inflow(std::move(inflow))
    {
    }

    friend class BellmanValuesComputeTest;
};

class ReservoirManagement
{
public:
    ReservoirManagement(const Reservoir& reservoir, bool overflow);

    Reservoir reservoir;
    bool overflow;
};

#pragma once

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

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
    double initial_level;
    std::vector<double> max_generating;
    std::vector<double> max_pumping;
    std::vector<std::vector<double>> inflow; // week, scenario
    std::vector<double> bottom_rule_curve;
    std::vector<double> upper_rule_curve;
    std::vector<std::vector<double>> optimal_trajectory; // week, scenario

private:
    void readRuleCurves(const std::filesystem::path& inputPath);
    void readInflow(const std::filesystem::path& inputPath);
    void readMaxPower(const std::filesystem::path& inputPath);
    void loadHydroIni(const std::filesystem::path& inputPath);

    // Private test-only constructor
    Reservoir(const std::string& areaName,
              double capacity,
              double efficiency,
              double initial_level,
              std::vector<double> max_generating,
              std::vector<double> max_pumping,
              std::vector<std::vector<double>> inflow,
              std::vector<double> bottom_rule_curve,
              std::vector<double> upper_rule_curve):
        area(areaName),
        capacity(capacity),
        efficiency(efficiency),
        initial_level(initial_level),
        max_generating(std::move(max_generating)),
        max_pumping(std::move(max_pumping)),
        inflow(inflow),
        bottom_rule_curve(std::move(bottom_rule_curve)),
        upper_rule_curve(std::move(upper_rule_curve)),
        optimal_trajectory(std::move(inflow)) // initializing the optimal trajectory here
    {
    }

    friend class BellmanValuesComputeTest;
};

class ReservoirManagement
{
public:
    ReservoirManagement(Reservoir& reservoir,
                        double penalty_bottom_rule_curve = 0,
                        double penalty_upper_rule_curve = 0,
                        double penalty_final_level = 0,
                        bool force_final_level = false,
                        std::optional<double> final_level = std::nullopt,
                        bool overflow = true);

    std::function<double(double)> get_penalty(int week, int len_week) const;

    void setReservoir(Reservoir& reservoir)
    {
        this->reservoir = reservoir;
    }

    Reservoir& reservoir;
    double penalty_bottom_rule_curve;
    double penalty_upper_rule_curve;
    double penalty_final_level;
    bool force_final_level;
    double final_level;
    bool overflow;
};

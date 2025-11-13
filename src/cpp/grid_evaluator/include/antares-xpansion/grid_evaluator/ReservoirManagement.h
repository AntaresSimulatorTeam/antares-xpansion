#pragma once

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <variant>
#include <vector>

/// @brief Represents a Reservoir loaded from a study
class Reservoir
{
public:
    static constexpr int weeks_in_year = 52;                          // number of weeks in a year
    static constexpr int hours_in_week = 168;                         // nubmer of hours in a week
    static constexpr int hours_in_day = 24;                           // number of hours in a day
    static constexpr int days_in_week = hours_in_week / hours_in_day; // number of days in a week
    static constexpr int days_in_year = weeks_in_year * days_in_week; // number of days in a year

    Reservoir(const std::filesystem::path& inputPath, const std::string& areaName);

    std::string area;                        // name of the area where reservoir is located
    double capacity;                         // capacity of the reservoir
    double efficiency;                       // efficiency of the reservoir
    double initial_level;                    // initial level of the reservoir
    std::vector<double> max_generating;      // max_generating power for each week
    std::vector<double> max_pumping;         // max_pumping power for each week
    std::vector<std::vector<double>> inflow; // inflow for hour of each week
    std::vector<double> bottom_rule_curve; // lowest level accepted without penalties for each week
    std::vector<double> upper_rule_curve;  // highest level accepted without penalties for each week

private:
    void loadRuleCurves(const std::filesystem::path& inputPath);
    void loadInflow(const std::filesystem::path& inputPath);
    void loadMaxPower(const std::filesystem::path& inputPath);
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
        inflow(std::move(inflow)),
        bottom_rule_curve(std::move(bottom_rule_curve)),
        upper_rule_curve(std::move(upper_rule_curve))
    {
    }

    friend class BellmanValuesComputeTest;
};

/// @brief Defines the rules that a reservoir follows
class ReservoirManagement
{
public:
    ReservoirManagement(const Reservoir& reservoir,
                        double penalty_bottom_rule_curve = 0,
                        double penalty_upper_rule_curve = 0,
                        double penalty_final_level = 0,
                        bool force_final_level = false,
                        std::optional<double> final_level = std::nullopt,
                        bool overflow = true);

    std::function<double(double)> get_penalty(int week, int len_week) const;

    Reservoir reservoir;              // Current reservoir
    double penalty_bottom_rule_curve; // penalty per MWh if bottom curve is violated
    double penalty_upper_rule_curve;  // penalty per MWh if upper curve is violated
    double penalty_final_level;       // penalty per MWh if final level is not reached
    bool force_final_level;           // true -> final level is forced to final_level
    double final_level; // value of the final level to reached if forces. If not given, default
                        // value is reservoir.inital_level
    bool overflow;      // true -> allow overflow of the reservoir
};

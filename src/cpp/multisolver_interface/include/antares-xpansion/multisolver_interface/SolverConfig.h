#pragma once

#include <filesystem>
#include <map>
#include <string>

/**
 * @brief Class to store the configuration of a solver
 * Invariant: name is lowercase
 */
class SolverConfig
{
    void init(std::string solver_name);
    std::string name;
    bool save_restore_supported{false};
    bool use_save_restore{false};

public:
    explicit SolverConfig(std::string name);
    SolverConfig(SolverConfig&&) = default;
    SolverConfig(const SolverConfig&) = default;
    SolverConfig& operator=(const SolverConfig&) = default;
    SolverConfig& operator=(SolverConfig&&) = default;

    [[nodiscard]] std::string Name() const
    {
        return name;
    }

    ~SolverConfig() = default;
    bool operator==(const std::string& rhs) const;
    SolverConfig& operator=(const std::string& rhs);
    std::filesystem::path FileName(const std::string& problemName);
};

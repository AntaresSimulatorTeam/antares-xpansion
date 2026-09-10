#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "WorkerMaster.h"
#include "common.h"

class IBendersProblemProvider;
class SolverLogManager;

class BendersMasterManager
{
public:
    BendersMasterManager() = default;
    ~BendersMasterManager() = default;

    // Lifecycle
    void CreateMaster(const VariableMap& variable_map,
                      const std::string& solver_name,
                      int log_level,
                      int subproblems_count,
                      SolverLogManager& solver_log_manager,
                      bool mps_has_alpha,
                      Logger logger,
                      ProblemsFormat format,
                      IBendersProblemProvider* benders_problem_provider,
                      double master_solution_tolerance,
                      const std::map<int, double>& subproblem_cut_coefficient_tolerance);
    void FreeMaster();
    [[nodiscard]] bool IsEmpty() const;
    [[nodiscard]] WorkerMasterPtr GetMaster() const;

    // Variable management
    void SetVariableMap(const VariableMap& variable_map);
    [[nodiscard]] const VariableMap& GetVariableMap() const;

    // Constraint management
    void AddRows(const std::vector<char>& qrtype,
                 const std::vector<double>& rhs,
                 const std::vector<double>& range,
                 const std::vector<int>& mstart,
                 const std::vector<int>& mclind,
                 const std::vector<double>& dmatval,
                 const std::vector<std::string>& row_names = {}) const;
    void ChangeRhs(int id_row, double val) const;
    [[nodiscard]] int GetNrows() const;

    // Objective management
    [[nodiscard]] std::vector<double> GetObjectiveFunctionCoeffs() const;
    void SetObjectiveFunction(const double* coeffs, int first, int last) const;
    void SetObjectiveFunctionCoeffsToZeros() const;

    // Integrity constraints
    void DeactivateIntegrityConstraints() const;
    void ActivateIntegrityConstraints() const;

    // Paths
    [[nodiscard]] std::filesystem::path GetMasterPath(const std::string& input_root,
                                                      const std::string& master_name,
                                                      ProblemsFormat format,
                                                      const std::string& solver_name) const;
    void WriteBasis(const std::filesystem::path& filename) const;

private:
    WorkerMasterPtr master_;
    VariableMap variable_map_;
    bool is_empty_ = true;
};

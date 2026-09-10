#include "antares-xpansion/benders/benders_core/BendersMasterManager.h"

#include <cassert>

void BendersMasterManager::CreateMaster(
  const VariableMap& variable_map,
  const std::string& solver_name,
  int log_level,
  int subproblems_count,
  SolverLogManager& solver_log_manager,
  bool mps_has_alpha,
  Logger logger,
  ProblemsFormat format,
  IBendersProblemProvider* benders_problem_provider,
  double master_solution_tolerance,
  const std::map<int, double>& subproblem_cut_coefficient_tolerance)
{
    master_ = std::make_shared<WorkerMaster>(variable_map,
                                             solver_name,
                                             log_level,
                                             subproblems_count,
                                             solver_log_manager,
                                             mps_has_alpha,
                                             logger,
                                             format,
                                             benders_problem_provider,
                                             master_solution_tolerance,
                                             subproblem_cut_coefficient_tolerance);
    is_empty_ = false;
}

void BendersMasterManager::FreeMaster()
{
    master_->free();
    is_empty_ = true;
}

bool BendersMasterManager::IsEmpty() const
{
    return is_empty_;
}

WorkerMasterPtr BendersMasterManager::GetMaster() const
{
    return master_;
}

void BendersMasterManager::SetVariableMap(const VariableMap& variable_map)
{
    variable_map_ = variable_map;
}

const VariableMap& BendersMasterManager::GetVariableMap() const
{
    return variable_map_;
}

void BendersMasterManager::AddRows(const std::vector<char>& qrtype,
                                   const std::vector<double>& rhs,
                                   const std::vector<double>& range,
                                   const std::vector<int>& mstart,
                                   const std::vector<int>& mclind,
                                   const std::vector<double>& dmatval,
                                   const std::vector<std::string>& row_names) const
{
    master_->AddRows(qrtype, rhs, range, mstart, mclind, dmatval, row_names);
}

void BendersMasterManager::ChangeRhs(int id_row, double val) const
{
    master_->ChangeRhs(id_row, val);
}

int BendersMasterManager::GetNrows() const
{
    return master_->Getnrows();
}

std::vector<double> BendersMasterManager::GetObjectiveFunctionCoeffs() const
{
    int ncols = master_->_solver->get_ncols();
    std::vector<double> obj(ncols);
    master_->_solver->get_obj(obj.data(), 0, ncols - 1);
    return obj;
}

void BendersMasterManager::SetObjectiveFunction(const double* coeffs, int first, int last) const
{
    assert(last >= first);
    master_->_solver->set_obj(coeffs, first, last);
}

void BendersMasterManager::SetObjectiveFunctionCoeffsToZeros() const
{
    auto master_vars_size = variable_map_.size();
    std::vector<double> zeros(master_vars_size, 0.0);
    SetObjectiveFunction(zeros.data(), 0, static_cast<int>(master_vars_size) - 1);
}

void BendersMasterManager::DeactivateIntegrityConstraints() const
{
    master_->DeactivateIntegrityConstraints();
}

void BendersMasterManager::ActivateIntegrityConstraints() const
{
    master_->ActivateIntegrityConstraints();
}

std::filesystem::path BendersMasterManager::GetMasterPath(const std::string& input_root,
                                                          const std::string& master_name,
                                                          ProblemsFormat format,
                                                          const std::string& solver_name) const
{
    if (format == ProblemsFormat::OPTIMIZED && solver_name == "XPRESS")
    {
        return std::filesystem::path(input_root) / (master_name + SAVE_SUFFIX);
    }
    else
    {
        return std::filesystem::path(input_root) / (master_name + MPS_SUFFIX);
    }
}

void BendersMasterManager::WriteBasis(const std::filesystem::path& filename) const
{
    master_->write_basis(filename);
}

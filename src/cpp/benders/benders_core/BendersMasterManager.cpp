#include "antares-xpansion/benders/benders_core/BendersMasterManager.h"

#include <cassert>

#include "antares-xpansion/helpers/Timer.h"

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

void BendersMasterManager::GetRhs(double& rhs, int id_row) const
{
    master_->GetRhs(&rhs, id_row);
}

int BendersMasterManager::GetNrows() const
{
    return master_->Getnrows();
}

int BendersMasterManager::GetNcols() const
{
    return master_->Getncols();
}

int BendersMasterManager::GetNElems() const
{
    return master_->_solver->get_nelems();
}

void BendersMasterManager::GetRowsCoeffs(std::vector<int>& mstart,
                                         std::vector<int>& mclind,
                                         std::vector<double>& dmatval,
                                         int size,
                                         std::vector<int>& nels,
                                         int first,
                                         int last) const
{
    master_->_solver
      ->get_rows(mstart.data(), mclind.data(), dmatval.data(), size, nels.data(), first, last);
}

void BendersMasterManager::GetRowType(std::vector<char>& qrtype, int first, int last) const
{
    master_->_solver->get_row_type(qrtype.data(), first, last);
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

void BendersMasterManager::SolveMaster(CurrentIterationData& data,
                                       bool bound_alpha,
                                       const std::string& output_root,
                                       const std::string& last_master_mps,
                                       const std::shared_ptr<Output::OutputWriter>& writer)
{
    Timer timer_master;

    data.single_subpb_costs_under_approx.resize(data.nsubproblem);
    data.master_only_vars_out.resize(master_->_id_master_only_vars.size());
    if (bound_alpha)
    {
        master_->fix_alpha(data.best_ub);
    }
    master_->solve(data.master_status, output_root, last_master_mps + MPS_SUFFIX, writer);

    master_->get(data.x_out,
                 data.overall_subpb_cost_under_approx,
                 data.single_subpb_costs_under_approx,
                 data.master_only_vars_out);
    master_->get_value(data.lb);

    for (const auto& [id, name]: master_->_id_to_name)
    {
        master_->_solver->get_ub(&data.max_invest[name], id, id);
        master_->_solver->get_lb(&data.min_invest[name], id, id);
    }

    data.timer_master = timer_master.elapsed();
}

void BendersMasterManager::ComputeInvestCost(CurrentIterationData& data) const
{
    data.invest_cost = 0;

    std::vector<double> obj(GetObjectiveFunctionCoeffs());

    for (const auto& [col_name, value]: data.x_cut)
    {
        int col_id = master_->_name_to_id[col_name];
        data.invest_cost += obj[col_id] * data.x_cut[col_name];
    }
    for (int i(0); i < data.master_only_vars_cut.size(); ++i)
    {
        int col_id = master_->_id_master_only_vars[i];
        data.invest_cost += obj[col_id] * data.master_only_vars_cut[i];
    }
}

void BendersMasterManager::UpdateOverallCosts(CurrentIterationData& data,
                                              double& best_invest_cost) const
{
    auto obj = GetObjectiveFunctionCoeffs();
    data.invest_cost = 0;
    for (const auto& [var_name, var_id]: variable_map_)
    {
        data.invest_cost += obj[var_id] * data.x_cut.at(var_name);
    }
    for (int i(0); i < data.master_only_vars_cut.size(); ++i)
    {
        int col_id = master_->_id_master_only_vars[i];
        data.invest_cost += obj[col_id] * data.master_only_vars_cut[i];
    }

    best_invest_cost = data.invest_cost;
}

void BendersMasterManager::AddAlphasFixingConstraints(
  std::vector<SubProblemNamesInCut>& names_in_cut,
  std::map<std::string, int>& problem_to_id) const
{
    master_->addAlphasFixingConstraints(names_in_cut, problem_to_id);
}

void BendersMasterManager::AddGroupSubproblemCut(std::vector<int> subproblem_ids,
                                                 const Point& subgradient,
                                                 const Point& x_cut,
                                                 const double& rhs) const
{
    master_->addGroupSubproblemCut(subproblem_ids, subgradient, x_cut, rhs);
}

const VariableMap& BendersMasterManager::GetNameToId() const
{
    return master_->_name_to_id;
}

const std::vector<int>& BendersMasterManager::GetMasterOnlyVarsIds() const
{
    return master_->_id_master_only_vars;
}

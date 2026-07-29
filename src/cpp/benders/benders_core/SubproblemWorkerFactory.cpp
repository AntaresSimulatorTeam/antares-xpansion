#include "antares-xpansion/benders/benders_core/SubproblemWorkerFactory.h"

#include <antares-xpansion/benders/benders_core/SolverIO.h>
#include <iostream>

#include "antares-xpansion/benders/benders_core/SkeletonSolverLoader.h"
#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"

SubproblemWorkerFactory::SubproblemWorkerFactory(
  const std::filesystem::path& input_root,
  Logger& logger,
  std::string solver_name,
  int log_level,
  ProblemsFormat format,
  std::vector<std::string> sub_problem_names,
  const SolverLogManager& solver_log_manager,
  boost::mpi::communicator* world):
    input_root_(input_root),
    memoptim_utils_(std::move(sub_problem_names))
{
    logger_ = logger;
    SkeletonSolverLoader loader(logger_);
    solver_ = loader.Load(input_root_ / "sub" / "sub.mps",
                          solver_name,
                          solver_log_manager,
                          log_level,
                          format);
    load_coefficient_sets();
    SubProblemSolverInitialSize_ = solver_->get_nrows();
}

void SubproblemWorkerFactory::GetBasis(std::string sub_name)
{
    int row_number = solver_->get_nrows();
    int col_number = solver_->get_ncols(); 
    std::vector<int> cstatus(col_number), rstatus(row_number) ;
    solver_->get_basis(rstatus.data(),cstatus.data());  
    subProblemBasis_[sub_name] = {std::move(rstatus), std::move(cstatus)};
}


SubproblemWorkerFactory::SubproblemWorkerFactory(const std::filesystem::path& input_root,
                                                 Logger& logger,
                                                 std::shared_ptr<SolverAbstract> solver,
                                                 std::vector<std::string> sub_problem_names,
                                                 boost::mpi::communicator* world):
    input_root_(input_root),
    solver_(std::move(solver)),
    memoptim_utils_(std::move(sub_problem_names))
{
    logger_ = logger;
    load_coefficient_sets();
}


std::shared_ptr<SolverAbstract> SubproblemWorkerFactory::GetSolver()
{
    return solver_ ; 
} 

void SubproblemWorkerFactory::load_coefficient_sets()
{
    auto dir = input_root_ / "sub";
    coef_set_.Load(memoptim_utils_,
                   dir / "coef.csv",
                   dir / "coef_cols.csv",
                   dir / "coef_rows.csv",
                   solver_);
    obj_set_.Load(memoptim_utils_,
                  dir / "obj_coef.csv",
                  dir / "obj_cols.csv",
                  std::nullopt,
                  solver_);
    rhs_set_.Load(memoptim_utils_, dir / "rhs.csv", std::nullopt, dir / "rhs_rows.csv", solver_);
}

int SubproblemWorkerFactory::GetSubNumber()
{
    return rhs_set_.Count();
}

std::shared_ptr<SubproblemWorker> SubproblemWorkerFactory::CreateSubSolverAbstract(
  std::string sub_name,
  VariableMap& variable_map,
  double cut_coefficient_tolerance,
  double slave_weight)
{

    //setting basis on the solver  
    if (subProblemBasis_.find(sub_name) != subProblemBasis_.end()) 
    {
        solver_->set_basis(subProblemBasis_[sub_name].first,subProblemBasis_[sub_name].second) ; 
    }
    
    solver_->chg_coefs(coef_set_.RowIndices(),
                       coef_set_.ColIndices(),
                       coef_set_.CoefficientsFor(sub_name));
    solver_->chg_obj(obj_set_.ColIndices(), obj_set_.CoefficientsFor(sub_name));
    solver_->chg_rhs_values(rhs_set_.RowIndices(), rhs_set_.CoefficientsFor(sub_name));

    auto subproblem_worker = std::make_shared<SubproblemWorker>(variable_map,
                                                                slave_weight,
                                                                solver_,
                                                                logger_);

    return subproblem_worker;
}

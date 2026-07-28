#include "antares-xpansion/benders/benders_core/FixedSkeletonSubProblemBuilder.h"

#include <antares-xpansion/benders/benders_core/SolverIO.h>


#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"
#include <iostream>



FixedSkeletonSubProblemBuilder::FixedSkeletonSubProblemBuilder(
  const std::filesystem::path& inputRoot,
  Logger& logger,
  std::string solver_name,
  int log_level,
  ProblemsFormat format,
  mpi::communicator* world,
  std::vector<std::string> sub_problem_names, 
  SolverLogManager& solver_log_manager,  
  std::shared_ptr<SolverAbstract> constraints_SolverAbstract):
    inputRoot_(inputRoot),
    memoptim_utils_(std::move(sub_problem_names)),
    constraints_SolverAbstract_(constraints_SolverAbstract)
{
    _world = world ;
    logger_ = logger;
    build_sub_skeleton(solver_name, solver_log_manager, log_level, format);
    read_coeffs_and_indices(CoeffType::constraints);
    read_coeffs_and_indices(CoeffType::objective);
    read_coeffs_and_indices(CoeffType::rhs);
    micro_iters_ = false;
    warm_start_ = false;
    std::cout<<"file path of solver log "<<solver_log_manager.log_file_path.c_str()<<std::endl ; 
}

FixedSkeletonSubProblemBuilder::FixedSkeletonSubProblemBuilder(
  const std::filesystem::path& inputRoot,
  Logger& logger,
  std::shared_ptr<SolverAbstract> solver,
  std::vector<std::string> sub_problem_names):
    inputRoot_(inputRoot),
    solver_(std::move(solver)),
    memoptim_utils_(std::move(sub_problem_names))
{
    logger_ = logger;
    read_coeffs_and_indices(CoeffType::constraints);
    read_coeffs_and_indices(CoeffType::objective);
    read_coeffs_and_indices(CoeffType::rhs);
    micro_iters_ = false;
    warm_start_ = false;
}

void FixedSkeletonSubProblemBuilder::read_coeffs_and_indices(CoeffType coeff_type)
{
    auto sub_dir = inputRoot_ / "sub";
    switch (coeff_type)
    {
    case CoeffType::constraints:
        memoptim_utils_.read_keyed_coeffs_csv(sub_dir / "coef.csv", coeffs_);
        memoptim_utils_.read_indices_csv(sub_dir / "coef_cols.csv",
                                         constraints_col_indices_,
                                         true,
                                         solver_);
        memoptim_utils_.read_indices_csv(sub_dir / "coef_rows.csv",
                                         constraints_row_indices_,
                                         false,
                                         solver_);
        break;
    case CoeffType::objective:
        memoptim_utils_.read_keyed_coeffs_csv(sub_dir / "obj_coef.csv", obj_coeffs_);
        memoptim_utils_.read_indices_csv(sub_dir / "obj_cols.csv", obj_col_indices_, true, solver_);
        break;
    case CoeffType::rhs:
        memoptim_utils_.read_keyed_coeffs_csv(sub_dir / "rhs.csv", rhs_);
        memoptim_utils_.read_indices_csv(sub_dir / "rhs_rows.csv",
                                         rhs_row_indices_,
                                         false,
                                         solver_);
        break;
    }
}

void FixedSkeletonSubProblemBuilder::build_sub_skeleton(std::string solver_name,
                                                        const SolverLogManager& solver_log_manager,
                                                        int log_level,
                                                        ProblemsFormat format)
{
    SolverFactory solver_factory(logger_);
    solver_ = solver_factory.create_solver(solver_name,
                                           SOLVER_TYPE::CONTINUOUS,
                                           solver_log_manager);

    solver_->set_threads(1);
    solver_->set_output_log_level(log_level);
    std::filesystem::path skeleton_sub = inputRoot_ / "sub" / "sub.mps";

    benders_problem_provider_ = std::make_shared<BendersProblemFromFile>(skeleton_sub);
    solver_IO_.configure(solver_name, format);

    benders_problem_provider_->provide_problem(solver_IO_, solver_);

    skeleton_initial_size_ = solver_->get_nrows() ; 
}

void FixedSkeletonSubProblemBuilder::set_added_constraints(
  std::string sub_name,
  std::vector<std::string>& added_constraints)
{
    added_constraints_per_sub_[sub_name].insert(added_constraints_per_sub_[sub_name].end(),
                                                std::make_move_iterator(added_constraints.begin()),
                                                std::make_move_iterator(added_constraints.end()));
}

int FixedSkeletonSubProblemBuilder::get_sub_number()
{
    return rhs_.size();
}

std::shared_ptr<SubproblemWorker> FixedSkeletonSubProblemBuilder::create_sub_solver_abstract(
  std::string sub_name,
  VariableMap& variable_map,
  double cut_coefficient_tolerance,
  double slave_weight)
{
    auto& coeffs_sub = coeffs_[sub_name];
    auto& coeffs_obj = obj_coeffs_[sub_name];
    auto& rhs_values = rhs_[sub_name];

    //remove added constraints to reset the sovler to its initial state 
    //before adding setting it for the ne
    int num_rows = solver_->get_nrows() ; 
    if (num_rows != skeleton_initial_size_)
    {
        num_rows-- ; 
        solver_->del_rows(skeleton_initial_size_,num_rows ) ; 
    }

    solver_->chg_coefs(constraints_row_indices_, constraints_col_indices_, coeffs_sub);
    solver_->chg_obj(obj_col_indices_, coeffs_obj);
    solver_->chg_rhs_values(rhs_row_indices_, rhs_values);

    auto subproblem_worker = std::make_shared<SubproblemWorker>(variable_map,
                                                                slave_weight,
                                                                solver_,
                                                                logger_);
    
    for (auto& solver_row_name: added_constraints_per_sub_[sub_name])
    {
        std::cout<<"adding constraints "<<std::endl ; 
        auto row_index = constraints_SolverAbstract_->get_row_index(solver_row_name) ; 
        if (row_index<0) [[unlikely]]
            std::cerr<<"can't find "<<solver_row_name<<" in contraints solver "<<std::endl ; 
        else 
        {
            auto row_representation = ConstraintsFileReader::get_row(constraints_SolverAbstract_,row_index) ;
            subproblem_worker->AddRows(row_representation.qrtype_p,
                                       row_representation.rhs,
                                       row_representation.range_p,
                                       row_representation.mstart,
                                       row_representation.mclind,
                                       row_representation.dmatval,
                                       row_representation.row_names);        
        }
    }
    return subproblem_worker;
}

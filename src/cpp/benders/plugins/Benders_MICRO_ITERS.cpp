/*
    Implementation of Benders_MICRO_ITERS
*/

#include "antares-xpansion/benders/plugins/Benders_MICRO_ITERS.h"

#include <cassert>
#include <chrono>
#include <exception>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string_view>

#include <boost/tokenizer.hpp>

#include "iostream"

Benders_MICRO_ITERS::Benders_MICRO_ITERS(const SimulationOptions& options,
                                         const CouplingMap& coupling_map,
                                         mpi::communicator* world):
    options_(options)
{
    coupling_map_ = coupling_map;

    CouplingMapGenerator::BuildSubProblemConstraintMap(coupling_map_,
                                                       subproblem_constraint_map_,
                                                       constraints_coupling_map_,
                                                       options_);

    warm_start_ = true;
    _world = world;
    InitialSubProblemSolverSize_ = 0;

    read_micro_iteration_config_file();
    read_variable_names_to_follow();
    std::filesystem::path plugin_lib_path = micro_iterations_config_["plugin_lib_path"];

    auto cpp_lib_absolute_path = std::filesystem::path(options_.INPUTROOT) / plugin_lib_path;
#ifdef _WIN32
    handle_ = LoadLibraryW(cpp_lib_absolute_path.wstring().c_str());
#else
    handle_ = dlopen(cpp_lib_absolute_path.c_str(), RTLD_NOW);
#endif
    if (handle_)
    {
        auto load_sym = [&](const char* name) -> void*
        {
#ifdef _WIN32
            return reinterpret_cast<void*>(GetProcAddress(handle_, name));
#else
            return dlsym(handle_, name);
#endif
        };

        onBendersStartPlugin_ = (on_Benders_start_Func)load_sym("OnBendersStart");
        if (!onBendersStartPlugin_)
        {
            std::cerr << "can't find OnBendersStart in the plugin" << std::endl;
            _world->abort(EXIT_FAILURE);
        }

        OnBendersEndPlugin_ = (on_Benders_end_Func)load_sym("OnBendersEnd");
        if (!OnBendersEndPlugin_)
        {
            std::cerr << "can't find OnBendersEnd in the plugin" << std::endl;
            _world->abort(EXIT_FAILURE);
        }

        OnBendersIterationStart_ = (on_Benders_iteration_start)load_sym("OnBendersIterationStart");
        if (!OnBendersIterationStart_)
        {
            std::cerr << "can't find OnBendersIterationStart in the plugin" << std::endl;
            _world->abort(EXIT_FAILURE);
        }

        OnBendersIterationEnd_ = (on_Benders_iteration_end)load_sym("OnBendersIterationEnd");
        if (!OnBendersIterationEnd_)
        {
            std::cerr << "can't find OnBendersIterationEnd in the plugin" << std::endl;
            _world->abort(EXIT_FAILURE);
        }

        OnBendersMasterResolutionStart_ = (on_Benders_master_resolution_start)load_sym(
          "OnBendersMasterResolutionStart");
        if (!OnBendersMasterResolutionStart_)
        {
            std::cerr << "can't find OnBendersMasterResolutionStart in the plugin" << std::endl;
            _world->abort(EXIT_FAILURE);
        }

        OnBendersMasterResolutionEnd_ = (on_Benders_master_resolution_end)load_sym(
          "OnBendersMasterResolutionEnd");
        if (!OnBendersMasterResolutionEnd_)
        {
            std::cerr << "can't find OnBendersMasterResolutionEnd in the plugin" << std::endl;
            _world->abort(EXIT_FAILURE);
        }

        OnBendersMicroIterationStart_ = (on_Benders_micro_iteration_start)load_sym(
          "OnBendersMicroIterationStart");
        if (!OnBendersMicroIterationStart_)
        {
            std::cerr << "can't find OnBendersMicroIterationStart in the plugin" << std::endl;
            _world->abort(EXIT_FAILURE);
        }

        OnBendersMicroIterationEnd_ = (on_Benders_micro_iteration_end)load_sym(
          "OnBendersMicroIterationEnd");
        if (!OnBendersMicroIterationEnd_)
        {
            std::cerr << "can't find OnBendersMicroIterationEnd in the plugin" << std::endl;
            _world->abort(EXIT_FAILURE);
        }

        OnBendersSubResolutionStart_ = (on_Benders_sub_resolution_start)load_sym(
          "OnBendersSubResolutionStart");
        if (!OnBendersSubResolutionStart_)
        {
            std::cerr << "can't find OnBendersSubResolutionStart_ in the plugin" << std::endl;
            _world->abort(EXIT_FAILURE);
        }

        OnBendersSubResolutionEnd_ = (on_Benders_sub_resolution_end)load_sym(
          "OnBendersSubResolutionEnd");
        if (!OnBendersSubResolutionEnd_)
        {
            std::cerr << "can't find OnBendersSubResolutionEnd_ in the plugin" << std::endl;
            _world->abort(EXIT_FAILURE);
        }
    }
    else
    {
        std::cerr << "failed to open the plugin given on path " << cpp_lib_absolute_path
                  << std::endl;
        _world->abort(EXIT_FAILURE);
    }
}

void Benders_MICRO_ITERS::read_micro_iteration_config_file()
{
    // Reading the micro iterations configuration file
    std::filesystem::path mirco_iterations_options_path = std::filesystem::path(options_.INPUTROOT)
                                                          / "micro_iterations_config.txt";
    std::ifstream micro_iterations_options_stream(mirco_iterations_options_path.string());

    if (micro_iterations_options_stream.is_open())
    {
        std::string line;
        while (std::getline(micro_iterations_options_stream, line))
        {
            std::istringstream iss(line);
            std::string key, value;

            if (std::getline(iss, key, '=') && std::getline(iss, value))
            {
                if (key == "warm_start")
                {
                    if (value == "0")
                    {
                        warm_start_ = false;
                    }
                }
                else
                {
                    micro_iterations_config_[key] = value;
                }
            }
        }
        for (auto [key, value]: micro_iterations_config_)
        {
            if (key == "plugin_lib_path")
            {
                std::string cpp_output_lib_path = micro_iterations_config_["plugin_lib_path"];
            }
        }
    }
    else
    {
        std::cerr << "unable to open : " << mirco_iterations_options_path.string() << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Benders_MICRO_ITERS::read_variable_names_to_follow()
{
    // Reading variable names from text file
    std::filesystem::path variable_names_path = std::filesystem::path(options_.INPUTROOT)
                                                / "variable_names.txt";
    std::ifstream variable_names_stream(variable_names_path.string());

    if (variable_names_stream.is_open())
    {
        std::string line;
        while (std::getline(variable_names_stream, line))
        {
            // Skip empty lines
            if (!line.empty())
            {
                variables_to_follow_.push_back(line);
            }
        }
    }
    else
    {
        std::cerr << "unable to open : " << variable_names_path.string() << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Benders_MICRO_ITERS::OnBendersStart(const SubproblemsMapPtr& subproblem_map,
                                         const Logger& logger,
                                         const BendersBaseOptions& options,
                                         const SolverLogManager& solver_log_manager,
                                         std::shared_ptr<SolverAbstract> sub_problem_solver)
{
    if (sub_problem_solver != nullptr)
    {
        sub_problem_solver_ = sub_problem_solver;
        InitialSubProblemSolverSize_ = sub_problem_solver_->get_nrows();
    }
    else if (!subproblem_map.empty())
    {
        // Assumption that every subproblem has the same base structure (rows/cols; only
        // coefficients differ)
        InitialSubProblemSolverSize_ = subproblem_map.begin()->second->get_problem_row_num();
    }
    _logger = logger;
    solver_log_manager_ = &solver_log_manager;

    switch (options.CACHE_PROBLEMS)
    {
    // as in case 1 we have nothing to build initially , we load the constraint file in
    // BendersSubResolutionstart
    case 0:
        build_subproblem_constraints_manager_map(subproblem_map, options, solver_log_manager);
        break;
    case 2:
        build_skeleton_constraint_set_loader(options);
        break;
    default:
        break;
    }

    build_variables_to_follow_indices_vector();

    onBendersStartPlugin_(options.INPUTROOT,
                          options.OUTPUTROOT,
                          warm_start_,
                          _world,
                          options.LOG_LEVEL);
}

void Benders_MICRO_ITERS::ResetSharedSolverToBase()
{
    auto num_rows = sub_problem_solver_->get_nrows();
    if (num_rows != InitialSubProblemSolverSize_) [[likely]]
    {
        num_rows--;
        sub_problem_solver_->del_rows(InitialSubProblemSolverSize_, num_rows);
    }
}

void Benders_MICRO_ITERS::OnBendersEnd()
{
    OnBendersEndPlugin_();
#ifdef _WIN32
    FreeLibrary(handle_);
#else
    dlclose(handle_);
#endif
}

bool Benders_MICRO_ITERS::ShouldRestoreSubproblemBasis() const
{
    return warm_start_;
}

void Benders_MICRO_ITERS::OnBendersIterationStart()
{
    OnBendersIterationStart_();
}

void Benders_MICRO_ITERS::OnBendersIterationEnd()
{
    OnBendersIterationEnd_();
    if (options_.CACHE_PROBLEMS < 2)
    {
        if (!warm_start_)
        {
            if (options_.CACHE_PROBLEMS == 0)
            {
                // Only CACHE_PROBLEMS==0 keeps the same manager alive across iterations, so
                // it is the only mode where added rows must be physically removed here.
                // CACHE_PROBLEMS==1 rebuilds a fresh manager from disk on the next
                // OnBendersSubResolutionStart, making this step moot for that mode.
                for (auto& [sub_name, constraint_mgr_name]: subproblem_constraint_map_)
                {
                    auto it = constraints_map_.find(constraint_mgr_name);
                    if (it != constraints_map_.end())
                    {
                        it->second->DeleteAddedRows(InitialSubProblemSolverSize_);
                    }
                }
            }
            ClearPersistedConstraintsTracking();
        }
    }
    else
    {
        if (!warm_start_)
        {
            ClearPersistedConstraintsTracking();
        }
    }
}

void Benders_MICRO_ITERS::OnBendersMasterResolutionEnd(std::map<std::string, double>& master_out,
                                                       int& num_iter)
{
    OnBendersMasterResolutionEnd_(master_out, num_iter, _world, options_.INPUTROOT);
}

void Benders_MICRO_ITERS::build_variables_to_follow_indices_vector()
{
    // Only meaningful for CACHE_PROBLEMS<2: constraints_map_ stays empty for >=2, whose
    // per-subproblem manager is instead built (and its indices filled in) on demand in
    // OnBendersSubResolutionStart.
    for (auto& [sub_name, constraint_mgr_name]: subproblem_constraint_map_)
    {
        variables_to_follow_indices_per_sub_[sub_name] = std::vector<int>();

        auto it = constraints_map_.find(constraint_mgr_name);
        if (it == constraints_map_.end())
        {
            // This subproblem is handled by another MPI rank
            continue;
        }
        auto constraint_manager = it->second;
        for (auto& variable: variables_to_follow_)
        {
            int variable_index = constraint_manager->GetVariableIndexInSub(variable);
            variables_to_follow_indices_per_sub_[sub_name].push_back(variable_index);
        }
    }
}

SubproblemConstraintsManagerPtr Benders_MICRO_ITERS::GetActiveManager(const std::string& sub_name)
{
    if (options_.CACHE_PROBLEMS < 2)
    {
        std::string constraints_manager_name = subproblem_constraint_map_[sub_name];
        return constraints_map_[constraints_manager_name];
    }
    return subproblem_constraints_manager_;
}

void Benders_MICRO_ITERS::TrackAddedConstraint(const std::string& sub_name,
                                               const std::string& constraint)
{
    // CACHE_PROBLEMS==0 keeps the same manager alive for the whole run, so its added rows
    // already persist on their own; nothing needs tracking for replay there.
    if (options_.CACHE_PROBLEMS == 0 || !warm_start_)
    {
        return;
    }
    added_constraints_per_sub_[sub_name].push_back(constraint);
}

void Benders_MICRO_ITERS::ReplayPersistedConstraints(const std::string& sub_name)
{
    if (!warm_start_)
    {
        return;
    }
    if (options_.CACHE_PROBLEMS == 1)
    {
        std::string constraints_manager_name = subproblem_constraint_map_[sub_name];
        for (auto& constraint_name: added_constraints_per_sub_[sub_name])
        {
            constraints_map_[constraints_manager_name]->AddRows(constraint_name);
        }
    }
    else if (options_.CACHE_PROBLEMS >= 2)
    {
        auto ContraintsSolver = constraint_set_loader_->GetSolver();
        for (auto& constraint_name: added_constraints_per_sub_[sub_name])
        {
            int pos = ContraintsSolver->get_row_index(constraint_name);
            auto SolverRow = SolverRowExtractor::GetRow(ContraintsSolver, pos);
            sub_problem_solver_->add_rows(1,
                                          static_cast<int>(SolverRow.dmatval.size()),
                                          SolverRow.qrtype_p.data(),
                                          SolverRow.rhs.data(),
                                          NULL,
                                          SolverRow.mstart.data(),
                                          SolverRow.mclind.data(),
                                          SolverRow.dmatval.data(),
                                          SolverRow.row_names);
        }
    }
}

void Benders_MICRO_ITERS::ClearPersistedConstraintsTracking()
{
    for (auto& [sub_name, constraint_mgr_name]: subproblem_constraint_map_)
    {
        added_constraints_per_sub_[sub_name].clear();
    }
}

void Benders_MICRO_ITERS::OnBendersMasterResolutionStart()
{
    if (OnBendersMasterResolutionStart_)
    {
        OnBendersMasterResolutionStart_();
    }
}

void Benders_MICRO_ITERS::OnBendersMicroIterationStart()
{
    if (OnBendersMicroIterationStart_)
    {
        OnBendersMicroIterationStart_();
    }
}

void Benders_MICRO_ITERS::OnBendersMicroIterationEnd(std::string sub_name,
                                                     bool& added_rows,
                                                     std::string solving_time,
                                                     int num_master_iter,
                                                     int num_micro_iter)
{
    auto sub_constraints_manager = GetActiveManager(sub_name);
    std::vector<double> sub_solution = sub_constraints_manager->GetSubSolution();

    std::vector<int> variables_indices = variables_to_follow_indices_per_sub_[sub_name];
    std::vector<std::string> constraints_to_add_vec;
    OnBendersMicroIterationEnd_(sub_name,
                                solving_time,
                                sub_solution,
                                variables_indices,
                                variables_to_follow_,
                                options_.INPUTROOT,
                                constraints_to_add_vec,
                                num_master_iter,
                                num_micro_iter);
    added_rows = constraints_to_add_vec.size();
    for (auto& constraint_to_add: constraints_to_add_vec)
    {
        // we add the constraint for the next microiteration
        sub_constraints_manager->AddRows(constraint_to_add);
        // We keep the constraint in a list to replay at a future Benders iteration, if
        // warm_start_ says it should persist
        TrackAddedConstraint(sub_name, constraint_to_add);
    }
}

void Benders_MICRO_ITERS::OnBendersSubResolutionStart(
  const std::shared_ptr<SubproblemWorker>& sub_worker,
  std::string sub_name)
{
    if (options_.CACHE_PROBLEMS == 2)
    {
        auto constraints_file_name = subproblem_constraint_map_[sub_name];
        auto skeleton_solver = constraint_set_loader_->LoadConstraintSet(constraints_file_name);

        // Several subproblems share this skeleton solver on the same rank, so it may
        // still carry rows left over from whichever subproblem was solved here last.
        // Reset it back to its base structure before building this subproblem's manager.
        ResetSharedSolverToBase();

        subproblem_constraints_manager_ = SubproblemConstraintsManager::FromSharedSolver(
          skeleton_solver,
          sub_worker);

        if (variables_to_follow_indices_per_sub_[sub_name].size() == 0)
        {
            for (auto& variable: variables_to_follow_)
            {
                int variable_index = subproblem_constraints_manager_->GetVariableIndexInSub(
                  variable);
                variables_to_follow_indices_per_sub_[sub_name].push_back(variable_index);
            }
        }

        // Replay the rows persisted for this subproblem (if warm_start_), so the solver's
        // row count matches what a cached warm-start basis expects (see SubproblemBasisCache),
        // and rows temporarily stripped by sharing this skeleton solver come back.
        ReplayPersistedConstraints(sub_name);
    }
    else if (options_.CACHE_PROBLEMS == 1)
    {
        // In this case we are supposed to have One mps loaded on RAM
        constraints_map_.clear();
        // Case 1 rebuilds a fresh SubproblemWorker from disk every iteration
        // (see BendersBase::GetSubproblemCutCache), so the constraints manager
        // built once in OnBendersStart (from the never-populated subproblem_map
        // for this cache level) must instead be rebuilt here, per subproblem,
        // every time a fresh worker is handed to us.
        std::string constraints_manager_name = subproblem_constraint_map_[sub_name];
        auto constraints_file_path = std::filesystem::path(options_.INPUTROOT)
                                     / constraints_manager_name;
        constraints_map_[constraints_manager_name] = SubproblemConstraintsManager::
          FromConstraintsFile(constraints_file_path,
                              options_.SOLVER_NAME,
                              *solver_log_manager_,
                              _logger,
                              options_.LOG_LEVEL,
                              options_.PROBLEMS_FORMAT,
                              sub_worker);

        if (variables_to_follow_indices_per_sub_[sub_name].size() == 0)
        {
            for (auto& variable: variables_to_follow_)
            {
                int variable_index = constraints_map_[constraints_manager_name]
                                       ->GetVariableIndexInSub(variable);
                variables_to_follow_indices_per_sub_[sub_name].push_back(variable_index);
            }
        }

        // Replay the rows micro-iterations added for this subproblem in previous
        // Benders iterations, so the freshly-rebuilt worker's row count matches
        // what a cached warm-start basis expects (see SubproblemBasisCache).
        ReplayPersistedConstraints(sub_name);
    }

    if (OnBendersSubResolutionStart_)
    {
        OnBendersSubResolutionStart_();
    }
}

void Benders_MICRO_ITERS::OnBendersSubResolutionEnd()
{
    if (OnBendersSubResolutionEnd_)
    {
        OnBendersSubResolutionEnd_();
    }
}

void Benders_MICRO_ITERS::SetSubProblemIDs(std::vector<std::string>& sub_names)
{
    sub_names_ = sub_names;
}

void Benders_MICRO_ITERS::build_subproblem_constraints_manager_map(
  const SubproblemsMapPtr& subproblem_map,
  const BendersBaseOptions& options,
  const SolverLogManager& solver_log_manager)
{
    for (auto& [sub, sub_worker]: subproblem_map)
    {
        added_constraints_per_sub_[sub] = std::vector<std::string>();
        std::string constraints_file_name = subproblem_constraint_map_[sub];
        auto constraints_file_path = std::filesystem::path(options.INPUTROOT)
                                     / constraints_file_name;
        constraints_map_[constraints_file_name] = SubproblemConstraintsManager::FromConstraintsFile(
          constraints_file_path,
          options.SOLVER_NAME,
          solver_log_manager,
          _logger,
          options.LOG_LEVEL,
          options.PROBLEMS_FORMAT,
          sub_worker);
    }
}

/*<
Building the constraint handler for the compact skeleton input format
*/
void Benders_MICRO_ITERS::build_skeleton_constraint_set_loader(const BendersBaseOptions& options)
{
    const std::string prefix = "sub/sub_";
    const std::string suffix = ".mps";
    std::vector<std::string> constraints_names;
    for (const auto& [sub_key, _]: subproblem_constraint_map_)
    {
        auto number_str = sub_key.substr(prefix.size(),
                                         sub_key.size() - prefix.size() - suffix.size());
        constraints_names.push_back("constraints/constraints_" + number_str + ".mps");
    }

    constraint_set_loader_ = std::make_shared<SkeletonConstraintSetLoader>(options.INPUTROOT,
                                                                           _logger,
                                                                           options.SOLVER_NAME,
                                                                           options.LOG_LEVEL,
                                                                           options.PROBLEMS_FORMAT,
                                                                           std::move(
                                                                             constraints_names),
                                                                           _world);
}

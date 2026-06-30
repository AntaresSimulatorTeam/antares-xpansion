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

    input_root_ = options_.INPUTROOT;
    warm_start_ = true;
    _world = world;

    read_micro_iteration_config_file();
    read_variable_names_to_follow();
    std::cout<<"subproblem constraints map "<<std::endl ; 
    for (auto& [sub,constraint] :subproblem_constraint_map_ ) 
    {
        std::cout<<"sub "<<sub<<" constraints "<<constraint<<std::endl ; 
    }

    std::filesystem::path plugin_lib_path = micro_iterations_config_["plugin_lib_path"];

    auto cpp_lib_absolute_path = input_root_ / plugin_lib_path;
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
    std::filesystem::path mirco_iterations_options_path = input_root_
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
    std::filesystem::path variable_names_path = input_root_ / "variable_names.txt";
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
                                         int cache_problems)
{
    _logger = logger;
    cache_problems_ = cache_problems;
    if (options.CACHE_PROBLEMS < 2)
        BuildSubproblemConstraintsManagerMap(subproblem_map, options, solver_log_manager);
    else
        BuildMemOptimConstraintsSkeleton(options);
    build_variables_to_follow_indices_vector();

    onBendersStartPlugin_(sub_names_,
                          _world->rank(),
                          options.INPUTROOT,
                          options.OUTPUTROOT,
                          warm_start_,
                          _world,
                          options.LOG_LEVEL);
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

void Benders_MICRO_ITERS::OnBendersIterationStart()
{
    OnBendersIterationStart_();
}

void Benders_MICRO_ITERS::OnBendersIterationEnd()
{
    OnBendersIterationEnd_();
    if (!warm_start_)
    {
        for (auto& [sub_name, constraint_reader_name]: subproblem_constraint_map_)
        {
            auto it = constraints_map_.find(constraint_reader_name);
            if (it == constraints_map_.end())
            {
                continue;
            }
            it->second->delete_added_rows();
        }
    }
}

void Benders_MICRO_ITERS::OnBendersMasterResolutionEnd(std::map<std::string, double>& master_out,
                                                       int& num_iter)
{
    OnBendersMasterResolutionEnd_(master_out,
                                  num_iter,
                                  _world,
                                  added_constraints_per_sub_,
                                  options_.INPUTROOT);
}

void Benders_MICRO_ITERS::build_variables_to_follow_indices_vector()
{
    for (auto& [sub_name, constraint_reader_name]: subproblem_constraint_map_)
    {
        auto it = constraints_map_.find(constraint_reader_name);
        if (it == constraints_map_.end())
        {
            // This subproblem is handled by another MPI rank
            continue;
        }
        variables_to_follow_indices_per_sub_[sub_name] = std::vector<int>();
        auto constraint_reader = it->second;
        for (auto& variable: variables_to_follow_)
        {
            int variable_index = constraint_reader->get_variable_index_in_solution(variable);
            variables_to_follow_indices_per_sub_[sub_name].push_back(variable_index);
        }
    }
}

void Benders_MICRO_ITERS::OnBendersMasterResolutionStart()
{
    if (OnBendersMasterResolutionStart_)
    {
        OnBendersMasterResolutionStart_();
    }
}

void Benders_MICRO_ITERS::OnBendersMicroIterationStart(
  const std::shared_ptr<SubproblemWorker>& sub_worker,
  std::string sub_name)
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
    std::string constraints_manager_name = subproblem_constraint_map_[sub_name];
    auto sub_constraints_manager = constraints_map_[constraints_manager_name];

    auto sub_solution = sub_constraints_manager->get_sub_solution();
    std::vector<int> variables_indices = variables_to_follow_indices_per_sub_[sub_name];

    std::vector<std::string> constraints_to_add_vec;
    OnBendersMicroIterationEnd_(sub_name,
                                added_rows,
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
        sub_constraints_manager->add_rows(constraint_to_add);
    }
}

void Benders_MICRO_ITERS::OnBendersSubResolutionStart()
{
    if (OnBendersSubResolutionStart_)
    {
        OnBendersSubResolutionStart_();
    }
}

void Benders_MICRO_ITERS::OnBendersSubResolutionEnd(std::string sub_name, int num_micro_iter)
{
    if (OnBendersSubResolutionEnd_)
    {
        OnBendersSubResolutionEnd_(sub_name, num_micro_iter);
    }
}

void Benders_MICRO_ITERS::SetSubProblemIDs(std::vector<std::string>& sub_names)
{
    sub_names_ = sub_names;
}

void Benders_MICRO_ITERS::BuildSubproblemConstraintsManagerMap(
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
        ConstraintsFileReader file_reader(constraints_file_path,
                                          options.SOLVER_NAME,
                                          solver_log_manager,
                                          _logger,
                                          options.LOG_LEVEL,
                                          options.PROBLEMS_FORMAT);
        constraints_map_[constraints_file_name] = std::make_shared<SubproblemConstraintsManager>(
          std::move(file_reader),
          sub_worker);
    }
}

void Benders_MICRO_ITERS::BuildMemOptimConstraintsSkeleton(const BendersBaseOptions& options)
{
    memoptim_constraints_builder_ = std::make_shared<MemOptimConstraintsBuilder>(
      options.INPUTROOT,
      _logger,
      options.SOLVER_NAME,
      options.LOG_LEVEL,
      options.PROBLEMS_FORMAT);
}

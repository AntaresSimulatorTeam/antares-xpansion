/*
    Implementation of Benders_MICRO_ITERS
*/

#include "antares-xpansion/benders/plugins/Benders_MICRO_ITERS.h"

#include <cassert>
#include <chrono>
#include <fstream>
#include <sstream>
#include <string_view>

#include <boost/tokenizer.hpp>

#include "iostream"

Benders_MICRO_ITERS::Benders_MICRO_ITERS(const SimulationOptions& options,
                                         const CouplingMap& coupling_map,
                                         mpi::communicator* world):
    options_(options)
{
    coupling_map_ = coupling_map;

    CouplingMapGenerator::BuildSubProblemConstaintMap(coupling_map_,
                                                      subproblem_constraint_map_,
                                                      constraints_coupling_map_,
                                                      options_);

    input_root_ = options_.INPUTROOT;
    warm_start_ = true;
    _world = world;

    for (auto& [sub_name, _]: coupling_map_)
    {
        is_variable_names_indices_created_[sub_name] = false;
    }
    read_micro_iteration_config_file();
    read_variable_names();

    std::filesystem::path plugin_lib_path = micro_iterations_config_["plugin_lib_path"];

    auto cpp_lib_absolute_path = input_root_ / plugin_lib_path;
    handle_ = dlopen(cpp_lib_absolute_path.c_str(), RTLD_NOW);
    if (handle_)
    {
        onBendersStartPlugin_ = (on_Benders_start_Func)dlsym(handle_, "OnBendersStart");
        if (!onBendersStartPlugin_)
        {
            std::cerr << "can't find OnBendersStart in the plugin" << std::endl;
            _world->abort(EXIT_FAILURE);
        }

        OnBendersEndPlugin_ = (on_Benders_end_Func)dlsym(handle_, "OnBendersEnd");
        if (!OnBendersEndPlugin_)
        {
            std::cerr << "can't find OnBendersEnd in the plugin" << std::endl;
            _world->abort(EXIT_FAILURE);
        }

        OnBendersIterationStart_ = (on_Benders_iteration_start)dlsym(handle_,
                                                                     "OnBendersIterationStart");
        if (!OnBendersIterationStart_)
        {
            std::cerr << "can't find OnBendersIterationStart in the plugin" << std::endl;
            _world->abort(EXIT_FAILURE);
        }

        OnBendersIterationEnd_ = (on_Benders_iteration_end)dlsym(handle_, "OnBendersIterationEnd");
        if (!OnBendersIterationEnd_)
        {
            std::cerr << "can't find OnBendersIterationEnd in the plugin" << std::endl;
            _world->abort(EXIT_FAILURE);
        }

        OnBendersMasterResolutionStart_ = (on_Benders_master_resolution_start)
          dlsym(handle_, "OnBendersMasterResolutionStart");
        if (!OnBendersMasterResolutionStart_)
        {
            std::cerr << "can't find OnBendersMasterResolutionStart in the plugin" << std::endl;
            _world->abort(EXIT_FAILURE);
        }

        OnBendersMasterResolutionEnd_ = (on_Benders_master_resolution_end)
          dlsym(handle_, "OnBendersMasterResolutionEnd");
        if (!OnBendersMasterResolutionEnd_)
        {
            std::cerr << "can't find OnBendersMasterResolutionEnd in the plugin" << std::endl;
            _world->abort(EXIT_FAILURE);
        }

        OnBendersMicroIterationStart_ = (on_Benders_micro_iteration_start)
          dlsym(handle_, "OnBendersMicroIterationStart");
        if (!OnBendersMicroIterationStart_)
        {
            std::cerr << "can't find OnBendersMicroIterationStart in the plugin" << std::endl;
            _world->abort(EXIT_FAILURE);
        }

        OnBendersMicroIterationEnd_ = (on_Benders_micro_iteration_end)
          dlsym(handle_, "OnBendersMicroIterationEnd");
        if (!OnBendersMicroIterationEnd_)
        {
            std::cerr << "can't find OnBendersMicroIterationEnd in the plugin" << std::endl;
            _world->abort(EXIT_FAILURE);
        }

        OnBendersSubResolutionStart_ = (on_Benders_sub_resolution_start)
          dlsym(handle_, "OnBendersSubResolutionStart");
        if (!OnBendersSubResolutionStart_)
        {
            std::cerr << "can't find OnBendersSubResolutionStart_ in the plugin" << std::endl;
            _world->abort(EXIT_FAILURE);
        }

        OnBendersSubResolutionEnd_ = (on_Benders_sub_resolution_end)
          dlsym(handle_, "OnBendersSubResolutionEnd");
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
    std::ifstream micro_iterations_options_stream(mirco_iterations_options_path.c_str());

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
            if (key == "cpp_output_package")
            {
                std::string cpp_output_lib_path = micro_iterations_config_["cpp_output_package"];
            }
        }
    }
    else
    {
        std::cerr << "unable to open : " << mirco_iterations_options_path.c_str() << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Benders_MICRO_ITERS::read_variable_names()
{
    // Reading variable names from text file
    std::filesystem::path variable_names_path = input_root_ / "variable_names.txt";
    std::ifstream variable_names_stream(variable_names_path.c_str());

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
        std::cerr << "unable to open : " << variable_names_path.c_str() << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Benders_MICRO_ITERS::OnBendersStart(const SubproblemsMapPtr& subproblem_map,
                                         const Logger& logger,
                                         const BendersBaseOptions& options,
                                         const SolverLogManager& solver_log_manager)
{
    _logger = logger;

    BuildConstraintsReaderMap(subproblem_map, options, solver_log_manager);

    onBendersStartPlugin_(sub_pb_ids_,
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
    dlclose(handle_);
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
            auto constraint_reader = constraints_map_[constraint_reader_name];
            constraint_reader->delete_added_rows();
        }
    }
}

void Benders_MICRO_ITERS::OnBendersMasterResolutionStart(std::map<std::string, double>& master_out,
                                                         int& num_iter)
{
    if (OnBendersMasterResolutionStart_)
    {
        OnBendersMasterResolutionStart_(master_out,
                                        num_iter,
                                        _world,
                                        added_constraints_per_sub_,
                                        options_.INPUTROOT);
    }
}

void Benders_MICRO_ITERS::build_variables_to_follow_indices_vector(std::string sub_name)
{
    if (!is_variable_names_indices_created_[sub_name])
    {
        variables_to_follow_indices_per_sub_[sub_name] = std::vector<int>();
        std::string constraint_reader_name = subproblem_constraint_map_[sub_name];
        auto constraint_reader = constraints_map_[constraint_reader_name];
        auto sub_solution = constraint_reader->get_sub_solution();
        for (auto& variable: variables_to_follow_)
        {
            int variable_index = constraint_reader->get_variable_index_in_solution(variable);
            variables_to_follow_indices_per_sub_[sub_name].push_back(variable_index);
        }
    }
}

void Benders_MICRO_ITERS::OnBendersMasterResolutionEnd()
{
    if (OnBendersMasterResolutionEnd_)
    {
        OnBendersMasterResolutionEnd_();
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
    if (!OnBendersMicroIterationEnd_)
    {
        return;
    }

    // Get the const

    std::string constraint_reader_name = subproblem_constraint_map_[sub_name];
    auto constraint_reader = constraints_map_[constraint_reader_name];

    // Get the complete subproblem solution vector
    auto sub_solution = constraint_reader->get_sub_solution();
    build_variables_to_follow_indices_vector(sub_name);
    // Get the pre-built indices vector for variables to follow for this subproblem
    std::vector<int> variables_indices = variables_to_follow_indices_per_sub_[sub_name];

    std::vector<std::string> constraints_to_add_vec;
    // Call the dynamically loaded function
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
    // for (auto& contraint_to_add)
    added_rows = constraints_to_add_vec.size();
    for (auto& contraint_to_add: constraints_to_add_vec)
    {
        std::string constraint_reader_name = subproblem_constraint_map_[sub_name];
        auto constraint_reader = constraints_map_[constraint_reader_name];
        constraint_reader->add_rows(contraint_to_add);
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

void Benders_MICRO_ITERS::SetSubProblemIDs(const char** subs_ids, int n_subs)
{
    sub_ids_storage_.clear();
    sub_ids_storage_.reserve(n_subs);
    for (int i = 0; i < n_subs; i++)
    {
        sub_ids_storage_.push_back(subs_ids[i]);
    }

    sub_ids_ptrs_.resize(n_subs);
    for (int i = 0; i < n_subs; i++)
    {
        sub_ids_ptrs_[i] = sub_ids_storage_[i].c_str();
    }

    sub_pb_ids_ = SubProblemIds{sub_ids_ptrs_.data(), n_subs};
}

void Benders_MICRO_ITERS::BuildConstraintsReaderMap(const SubproblemsMapPtr& subproblem_map,
                                                    const BendersBaseOptions& options,
                                                    const SolverLogManager& solver_log_manager)
{
    for (auto& [sub, sub_worker]: subproblem_map)
    {
        added_constraints_per_sub_[sub] = std::vector<std::string>();
        std::string constraints_file_name = subproblem_constraint_map_[sub];
        auto constraints_file_path = std::filesystem::path(options.INPUTROOT)
                                     / constraints_file_name;
        constraints_map_[constraints_file_name] = std::make_shared<ConstraintsReader>(
          constraints_file_path,
          options.SOLVER_NAME,
          solver_log_manager,
          _logger,
          options.LOG_LEVEL,
          options.PROBLEMS_FORMAT,
          sub_worker);
    }
}

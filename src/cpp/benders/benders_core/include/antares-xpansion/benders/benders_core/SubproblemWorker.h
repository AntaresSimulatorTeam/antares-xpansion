#pragma once

#include "SubproblemCut.h"
#include "Worker.h"

/*!
 * \class SubproblemWorker
 * \brief Class daughter of Worker Class, build and manage a subproblem
 */
class SubproblemWorker;
typedef std::shared_ptr<SubproblemWorker> SubproblemWorkerPtr;
typedef std::vector<SubproblemWorkerPtr> WorkerSlaves;
typedef std::map<std::string, SubproblemWorkerPtr> SubproblemsMapPtr;

class SubproblemWorker: public Worker
{
public:
    using Worker::Worker;
    SubproblemWorker(const VariableMap& variable_map,
                     double slave_weight,
                     const std::string& solver_name,
                     int log_level,
                     const SolverLogManager& solver_log_manager,
                     Logger logger,
                     ProblemsFormat format,
                     IBendersProblemProvider* benders_problem_provider,
                     double cut_coefficient_tolerance);
    virtual ~SubproblemWorker() = default;
    std::vector<double> get_solution() const;

public:
    void fix_to(const Point& x0) const;

    void get_subgradient(Point& subgradient) const;

    int get_variable_index(const std::string& variable_name) ; 

    void get_variables_values_in_csv(std::filesystem::path variables_values_csv,const std::map<std::string,std::pair<std::string,int>>& variable_indices) ; 
    
    void write_subproblem_variable_csv(std::filesystem::path variables_values_csv, std::map<std::string,double>& variables_id_value_map) ; 
};

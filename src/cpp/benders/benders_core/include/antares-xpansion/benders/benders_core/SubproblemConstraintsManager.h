#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "antares-xpansion/benders/benders_core/ConstraintsFileReader.h"
#include "antares-xpansion/benders/benders_core/SubproblemWorker.h"



class SubproblemConstraintsManager
{
public:
    SubproblemConstraintsManager(ConstraintsFileReader file_reader,
                                 const std::shared_ptr<SubproblemWorker>& subproblem_worker);

    SubproblemConstraintsManager(std::shared_ptr<SolverAbstract> solver,
                                 const std::shared_ptr<SubproblemWorker>& subproblem_worker);

    SolverRepresentedRows add_rows(std::string& row_name);
    std::vector<double> get_sub_solution();
    int get_variable_index_in_solution(std::string variable_id);
    int size_of_subproblem();
    void delete_added_rows();

private:
    void add_rows_to_subproblem(SolverRepresentedRows& new_row);

    ConstraintsFileReader file_reader_; 
    std::shared_ptr<SubproblemWorker> subproblem_worker_;
    int initial_sub_size_;
};

typedef std::shared_ptr<SubproblemConstraintsManager> SubproblemConstraintsManagerPtr;
typedef std::map<std::string, SubproblemConstraintsManagerPtr> SubproblemConstraintsManagerPtrMap;


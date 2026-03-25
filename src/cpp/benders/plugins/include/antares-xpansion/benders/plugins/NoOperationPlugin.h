#pragma once

#include <antares-xpansion/benders/plugins/BendersPlugin.h>

class NoOperationPlugin: public BendersPlugin
{
public:
    NoOperationPlugin();
    virtual ~NoOperationPlugin() = default;

    void OnBendersStart(const SubproblemsMapPtr& subproblem_map,
                        const Logger& logger,
                        const BendersBaseOptions& options,
                        const SolverLogManager& solver_log_manager);

    void OnBendersEnd();

    void OnBendersIterationStart();
    void OnBendersIterationEnd();

    void OnBendersMasterResolutionStart(std::map<std::string, double>& master_out, int& num_iter);
    void OnBendersMasterResolutionEnd();

    void OnBendersSubResolutionStart();
    void OnBendersSubResolutionEnd(std::string sub_name, int num_micro_iter);

    void OnBendersMicroIterationStart();
    void OnBendersMicroIterationEnd(std::string sub_name, bool& added_rows, std::string solve_time);
};

#pragma once

#include <antares-xpansion/benders/plugins/BendersPlugin.h>

class NoOperationPlugin final: public BendersPlugin
{
public:
    NoOperationPlugin();
    ~NoOperationPlugin() override = default;

    void OnBendersStart(const SubproblemsMapPtr& subproblem_map,
                        const Logger& logger,
                        const BendersBaseOptions& options,
                        const SolverLogManager& solver_log_manager,
                        std::shared_ptr<SolverAbstract> sub_problem_solver = nullptr) override;

    void OnBendersEnd() override;

    void OnBendersIterationStart() override;
    void OnBendersIterationEnd() override;

    void OnBendersMasterResolutionEnd(std::map<std::string, double>& master_out,
                                      int& num_iter) override;
    void OnBendersMasterResolutionStart() override;

    void OnBendersSubResolutionStart(const std::shared_ptr<SubproblemWorker>& sub_worker = nullptr,
                                     std::string sub_name = "") override;
    void OnBendersSubResolutionEnd(std::string sub_name,
                                   int num_micro_iter) override;

    void OnBendersMicroIterationStart() override;
    void OnBendersMicroIterationEnd(std::string sub_name,
                                    bool& added_rows,
                                    std::string solve_time,
                                    int num_master_iter,
                                    int num_micro_iter) override;
};

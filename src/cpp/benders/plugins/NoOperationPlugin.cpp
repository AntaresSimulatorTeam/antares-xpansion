#include <antares-xpansion/benders/plugins/NoOperationPlugin.h>

NoOperationPlugin::NoOperationPlugin()
{
}

void NoOperationPlugin::OnBendersStart(const SubproblemsMapPtr& subproblem_map,
                                       const Logger& logger,
                                       const BendersBaseOptions& options,
                                       const SolverLogManager& solver_log_manager,
                                       std::shared_ptr<SolverAbstract> sub_problem_solver)
{
}

void NoOperationPlugin::OnBendersEnd()
{
}

void NoOperationPlugin::OnBendersIterationStart()
{
}

void NoOperationPlugin::OnBendersIterationEnd()
{
}

void NoOperationPlugin::OnBendersMasterResolutionEnd(std::map<std::string, double>& master_out,
                                                     int& num_iter)
{
}

void NoOperationPlugin::OnBendersMasterResolutionStart()
{
}

void NoOperationPlugin::OnBendersMicroIterationStart()
{
}

void NoOperationPlugin::OnBendersMicroIterationEnd(std::string sub_name,
                                                   bool& added_rows,
                                                   std::string solve_time,
                                                   int num_master_iter,
                                                   int num_micro_iter)
{
}

void NoOperationPlugin::OnBendersSubResolutionStart(
  const std::shared_ptr<SubproblemWorker>& sub_worker,
  std::string sub_name)
{
}

void NoOperationPlugin::OnBendersSubResolutionEnd(std::string sub_name,
                                                  int num_micro_iter)
{
}

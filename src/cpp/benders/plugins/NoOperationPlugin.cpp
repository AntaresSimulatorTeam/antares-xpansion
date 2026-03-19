#include <antares-xpansion/benders/plugins/NoOperationPlugin.h>

NoOperationPlugin::NoOperationPlugin()
{
}

void NoOperationPlugin::OnBendersStart(const SubproblemsMapPtr& subproblem_map,
                        const Logger& logger,
                        const BendersBaseOptions& options,
                        const SolverLogManager& solver_log_manager)
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

void NoOperationPlugin::OnBendersMasterResolutionStart(std::map<std::string , double>& master_out,int& num_iter)
{
}

void NoOperationPlugin::OnBendersMasterResolutionEnd()
{
}

void NoOperationPlugin::OnBendersMicroIterationStart()
{
}

void NoOperationPlugin::OnBendersMicroIterationEnd(std::string sub_name, bool& added_rows,std::string solve_time)
{
}


void NoOperationPlugin::OnBendersSubResolutionStart()  
{

}

void NoOperationPlugin::OnBendersSubResolutionEnd(std::string sub_name,int num_micro_iter)
{

}
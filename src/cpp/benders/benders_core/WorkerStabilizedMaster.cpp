#include "WorkerStabilizedMaster.h"

WorkerStabilizedMaster::WorkerStabilizedMaster() : WorkerMaster()
{
}
WorkerStabilizedMaster::WorkerStabilizedMaster(std::map<std::string, int> const &variable_name, std::string const &problem_name, const std::string &solver_name, const int log_level, int nslaves) : WorkerMaster(variable_name, problem_name, solver_name, log_level, nslaves)
{
}
WorkerStabilizedMaster::~WorkerStabilizedMaster()
{
}

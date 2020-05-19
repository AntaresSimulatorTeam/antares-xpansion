#include "WorkerStabilizedMaster.h"



WorkerStabilizedMaster::WorkerStabilizedMaster():WorkerMaster() {

}
WorkerStabilizedMaster::WorkerStabilizedMaster(std::map<std::string, int> const & variable_name, std::string const & problem_name, BendersOptions const & options, int nslaves) : WorkerMaster(variable_name, problem_name, options, nslaves) {

}
WorkerStabilizedMaster::~WorkerStabilizedMaster() {

}

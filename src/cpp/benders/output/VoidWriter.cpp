#include "VoidWriter.h"
namespace Output {

void VoidWriter::updateBeginTime() {
  // keep this method empty
}

void VoidWriter::updateEndTime() {
  // keep this method empty
}

void VoidWriter::write_iterations(const IterationsData &iterations_data) {
  // keep this method empty
}

void VoidWriter::update_solution(const SolutionData &solution_data) {
  // keep this method empty
}

/*!
 *  \brief write a json output with a failure status in solution. If
 * optimization process exits before it ends, this failure will be available as
 * an output.
 *
 */
void VoidWriter::write_failure() {
  // keep this method empty
}

/*!
 *  \brief write the json data into a file
 */
void VoidWriter::dump() {
  // keep this method empty
}
void VoidWriter::initialize() {
  // keep this method empty
}

void VoidWriter::end_writing(const IterationsData &iterations_data) {
  // keep this method empty
}
void VoidWriter::write_solver_name(const std::string &solver_name) {
  // keep this method empty
}
void VoidWriter::write_master_name(const std::string &master_name) {
  // keep this method empty
}
void VoidWriter::write_log_level(const int log_level) {
  // keep this method empty
}
}  // namespace Output
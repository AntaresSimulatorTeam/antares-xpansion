
#pragma once

#include "OutputWriter.h"

namespace Output {
/*!
 * \class VoidWriter
 * \brief VoidWriter class to describe the execuion session of an antares
 * xpansion optimization
 */
class VoidWriter : public OutputWriter {
 public:
  /*!
   *  \brief VoidWriter default constructor
   */
  VoidWriter() = default;

  /*!
   *  \brief destructor of class VoidWriter
   */
  virtual ~VoidWriter() = default;

  /*!
   *  \brief saves some entries to be later written
   *
   *  \param nbWeeks_p : number of the weeks in the study
   *  \param bendersTrace_p : trace to be written ie iterations details
   *  \param bendersData_p : final benders data to get the best iteration
   *  \param min_abs_gap : minimum absolute gap wanted
   *  \param min_rel_gap : minimum relative gap wanted
   *  \param max_iter : maximum number of iterations
   */
  virtual void write_iterations(const IterationsData &iterations_data);

  /*!
   *  \brief  saves some entries to be later written
   *
   *  \param nbWeeks_p : number of the weeks in the study
   *  \param lb_p : solution lower bound
   *  \param ub_p : solution upper bound
   *  \param investCost_p : investment cost
   *  \param operationalCost_p : operational cost
   *  \param overallCost_p : total cost, sum of invest and operational
   *  \param solution_p : point giving the solution and the candidates
   *  \param optimality_p : indicates if optimality was reached
   */
  virtual void update_solution(const SolutionData &solution_data);

  /*!
   *  \brief write an a priori errored json output, overwritten if optimization
   * ends
   */
  virtual void write_failure();

  /*!
   *  \brief write the json data into a file
   */
  virtual void dump();
  virtual void initialize();
  virtual void end_writing(const IterationsData &iterations_data);
  void write_solver_name(const std::string &solver_name) override;
  void write_master_name(const std::string &master_name) override;
  void write_log_level(const int log_level) override;
  void write_solution(const SolutionData &solution) override;
  void write_outer_loop_solution(const SolutionData &solution) override;
  void write_iteration(const Iteration &iteration_data,
                       const size_t iteration_num) override;
  void updateBeginTime() override;
  void updateEndTime() override;
  void write_nbweeks(const int nb_weeks) override;
  void write_duration(const double duration) override;
  std::string solution_status() const override;
  void WriteProblem(const ProblemData &problem_data) override;
};
}  // namespace Output
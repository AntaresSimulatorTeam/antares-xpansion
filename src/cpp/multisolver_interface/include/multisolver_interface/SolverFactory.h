#pragma once

#include <set>

#include "multisolver_interface/SolverAbstract.h"

enum class SOLVER_TYPE { INTEGER, CONTINUOUS };
const std::string UNKNOWN_STR("UNKNOWN"), COIN_STR("COIN"), CBC_STR("CBC"),
    CLP_STR("CLP"), XPRESS_STR("XPRESS"), CPLEX_STR("CPLEX");

/*!
 * \class class SolverFactory
 * \brief Class to manage the creation of solvers from the different
 * implementations
 */
class SolverFactory {
 private:
  std::vector<std::string> _available_solvers;
  std::string other_solver_ = "";

 public:
  /**
   * @brief Constructor of SolverFactory, fills the list of available solvers
   */
  SolverFactory();

 public:
  /**
   * @brief Creates and returns to an object solver from the wanted
   * implementation
   *
   * @param solver_name : Name of the solver to use
   */
  SolverAbstract::Ptr create_solver(const std::string &solver_name) const;
  SolverAbstract::Ptr create_solver(
      const std::string &solver_name,
      const std::filesystem::path &log_name) const;

  /**
   * @brief Creates and returns to an object solver from the wanted
   * implementation
   *
   * @param solver_name : Name of the solver to use
   * @param solver_type : Name of the solver to use
   */
  SolverAbstract::Ptr create_solver(const std::string &solver_name,
                                    const SOLVER_TYPE solver_type) const;
  SolverAbstract::Ptr create_solver(
      const std::string &solver_name, const SOLVER_TYPE solver_type,
      const std::filesystem::path &log_name) const;

  /**
   * @brief Copy constructor : Creates and returns to an object solver from the
   * wanted implementation by copying datas from same solver implementation
   *
   * @param to_copy : solver to copy
   */
  SolverAbstract::Ptr copy_solver(SolverAbstract::Ptr to_copy);
  SolverAbstract::Ptr copy_solver(
      const std::shared_ptr<const SolverAbstract> &to_copy);

  /**
   * @brief Returns a reference to the list of available solvers
   */
  const std::vector<std::string> &get_solvers_list() const;
};

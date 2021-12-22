#pragma once

#include "multisolver_interface/SolverAbstract.h"

/*!
 * \class class SolverFactory
 * \brief Class to manage the creation of solvers from the different
 * implementations
 */
class SolverFactory {
public:
  std::vector<std::string> _available_solvers;

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
  SolverAbstract::Ptr create_solver(const std::string solver_name);

  /**
   * @brief Copy constructor : Creates and returns to an object solver from the
   * wanted implementation by copying datas from same solver implementation
   *
   * @param to_copy : solver to copy
   */
  SolverAbstract::Ptr create_solver(SolverAbstract::Ptr to_copy);

  /**
   * @brief Returns a reference to the list of available solvers
   */
  const std::vector<std::string> &get_solvers_list() const;
};
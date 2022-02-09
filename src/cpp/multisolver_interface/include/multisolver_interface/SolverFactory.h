#pragma once

#include "multisolver_interface/SolverAbstract.h"
#include <set>

enum class SOLVER_TYPE
{
    INTEGER,
    CONTINUOUS
};
const std::string
    UNKNOWN_STR("UNKNOWN"),
    COIN_STR("COIN"),
    CBC_STR("CBC"),
    CLP_STR("CLP"),
    XPRESS_STR("XPRESS"),
    CPLEX_STR("CPLEX");

enum class SOLVER_NAME
{
    UNKNOWN,
    COIN,
    CPLEX,
    XPRESS,
    CLP,
    CBC
};

SOLVER_NAME str_to_solver_name(std::string solver_name);

std::string
solver_to_str(SOLVER_NAME solver);
// SOLVER_NAME str_to_solver_name(std::string solver_name)
// {

//     if (solver_name == COIN_STR)
//     {
//         return SOLVER_NAME::COIN;
//     }
//     if (solver_name == CBC_STR)
//     {
//         return SOLVER_NAME::CBC;
//     }
//     if (solver_name == CPLEX_STR)
//     {
//         return SOLVER_NAME::CPLEX;
//     }
//     if (solver_name == CLP_STR)
//     {
//         return SOLVER_NAME::CLP;
//     }
//     else
//     {
//         return SOLVER_NAME::UNKNOWN;
//     }
// }
// std::string
// solver_to_str(SOLVER_NAME solver)
// {
//     switch (solver)
//     {
//     case SOLVER_NAME::COIN:
//         return COIN_STR;
//         break;

//     case SOLVER_NAME::CPLEX:
//         return CPLEX_STR;
//         break;

//     case SOLVER_NAME::CLP:
//         return CLP_STR;
//         break;

//     case SOLVER_NAME::CBC:
//         return CBC_STR;
//         break;

//     case SOLVER_NAME::XPRESS:
//         return XPRESS_STR;
//         break;

//     default:
//         return UNKNOWN_STR;
//         break;
//     }
// }

/*!
 * \class class SolverFactory
 * \brief Class to manage the creation of solvers from the different implementations
 */
class SolverFactory
{
private:
    std::vector<std::string> _available_solvers;

public:
    /**
     * @brief Constructor of SolverFactory, fills the list of available solvers
     */
    SolverFactory();

public:
    /**
     * @brief Creates and returns to an object solver from the wanted implementation
     *
     * @param solver_name : Name of the solver to use
     */
    SolverAbstract::Ptr create_solver(const std::string solver_name);

    /**
     * @brief Creates and returns to an object solver from the wanted implementation
     *
     * @param solver_name : Name of the solver to use
     * @param solver_type : Name of the solver to use
     */
    SolverAbstract::Ptr create_solver(const std::string solver_name, const SOLVER_TYPE solver_type);

    /**
     * @brief Copy constructor : Creates and returns to an object solver from the wanted
     * implementation by copying datas from same solver implementation
     *
     * @param to_copy : solver to copy
     */
    SolverAbstract::Ptr create_solver(SolverAbstract::Ptr to_copy);

    /**
     * @brief Returns a reference to the list of available solvers
     */
    const std::vector<std::string> &get_solvers_list() const;
};
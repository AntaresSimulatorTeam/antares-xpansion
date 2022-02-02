#pragma once

#include "multisolver_interface/SolverAbstract.h"
#include <set>

const std::string
    UNKNOWN_STR("UNKNOWN"),
    COIN_STR("COIN"),
    CBC_STR("CBC"),
    CLP_STR("CLP"),
    XPRESS_STR("XPRESS"),
    CPLEX_STR("CPLEX");

enum class SOLVER_TYPE
{
    UNKNOWN,
    COIN,
    CPLEX,
    XPRESS,
    CLP,
    CBC
};

SOLVER_TYPE str_to_solver_type(std::string solver_name);

std::string
solver_to_str(SOLVER_TYPE solver);
// SOLVER_TYPE str_to_solver_type(std::string solver_name)
// {

//     if (solver_name == COIN_STR)
//     {
//         return SOLVER_TYPE::COIN;
//     }
//     if (solver_name == CBC_STR)
//     {
//         return SOLVER_TYPE::CBC;
//     }
//     if (solver_name == CPLEX_STR)
//     {
//         return SOLVER_TYPE::CPLEX;
//     }
//     if (solver_name == CLP_STR)
//     {
//         return SOLVER_TYPE::CLP;
//     }
//     else
//     {
//         return SOLVER_TYPE::UNKNOWN;
//     }
// }
// std::string
// solver_to_str(SOLVER_TYPE solver)
// {
//     switch (solver)
//     {
//     case SOLVER_TYPE::COIN:
//         return COIN_STR;
//         break;

//     case SOLVER_TYPE::CPLEX:
//         return CPLEX_STR;
//         break;

//     case SOLVER_TYPE::CLP:
//         return CLP_STR;
//         break;

//     case SOLVER_TYPE::CBC:
//         return CBC_STR;
//         break;

//     case SOLVER_TYPE::XPRESS:
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
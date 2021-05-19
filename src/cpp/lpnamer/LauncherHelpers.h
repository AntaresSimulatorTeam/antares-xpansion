#pragma once

#include "ortools_utils.h"

#include "AdditionalConstraints.h"

#include <map>

/**
 * \brief adds binary variables and additional constraints to an existent solver
 *
 * \param master_p solver to which the constraints and variables will be added
 * \param additionalConstraints_p the additional constraints to add
 */
void treatAdditionalConstraints(operations_research::MPSolver & master_p, AdditionalConstraints additionalConstraints_p);

/**
 * \brief adds an additional constraint to an existent solver
 *
 * \param master_p solver to which the constraint will be added
 * \param additionalConstraint_p the additional constraint to add
 */
void addAdditionalConstraint(operations_research::MPSolver & master_p, AdditionalConstraint & additionalConstraint_p);

/**
 * \brief creates a binary variable and its corresponding linking constraint
 *
 * \param master_p solver to which the binary variable and the linking constraint will be added
 * \param variablesToBinarise_p map listing the variables to add and their corresponding ones
 *
 * for each entry (BinVar, CorrespondingVar) from the input map,
 *          creates the binary variable BinVar
 *          adds the linking constraint link_BinVar_CorrespondingVar : CorrespondingVar  <= UB(CorrespondingVar) * BinVar
 */
void addBinaryVariables(operations_research::MPSolver & master_p, std::map<std::string, std::string> const & variablesToBinarise_p);

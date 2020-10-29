#pragma once

#include "ortools_utils.h"

#include "AdditionalConstraints.h"

#include <map>

void treatAdditionalConstraints(operations_research::MPSolver & master_p, AdditionalConstraints additionalConstraints_p);
void addAdditionalConstraint(operations_research::MPSolver & master_p, AdditionalConstraint & additionalConstraint_p);
void addBinaryVariables(operations_research::MPSolver & master_p, std::map<std::string, std::string> const & variablesToBinarise_p);

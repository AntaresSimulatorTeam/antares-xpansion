//
// Created by s90365 on 23/08/2021.
//

#include <multisolver_interface/SolverAbstract.h>
#include "ActiveLinks.h"

#ifndef ANTARESXPANSION_MASTERPROBLEMBUILDER_H
#define ANTARESXPANSION_MASTERPROBLEMBUILDER_H

class MasterProblemBuilder {
public:
	std::shared_ptr<SolverAbstract> build(const std::string& solverName, const std::vector<ActiveLink>& links);
};

#endif //ANTARESXPANSION_MASTERPROBLEMBUILDER_H

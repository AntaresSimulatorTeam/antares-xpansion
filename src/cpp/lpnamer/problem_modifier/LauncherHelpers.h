#pragma once

#include <filesystem>
#include <map>

#include "ActiveLinks.h"
#include "AdditionalConstraints.h"
#include "ProblemGenerationLogger.h"
#include "antares-xpansion/helpers/solver_utils.h"
/**
 * \brief adds binary variables and additional constraints to an existent solver
 *
 * \param master_p solver to which the constraints and variables will be added
 * \param additionalConstraints_p the additional constraints to add
 */
void treatAdditionalConstraints(
    SolverAbstract::Ptr master_p,
    const AdditionalConstraints& additionalConstraints_p,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger);

char getConstraintSenseSymbol(
    const AdditionalConstraint& additionalConstraint_p,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger);

/**
 * \brief adds an additional constraint to an existent solver
 *
 * \param master_p solver to which the constraint will be added
 * \param additionalConstraint_p the additional constraint to add
 */
void addAdditionalConstraint(
    SolverAbstract::Ptr master_p, const AdditionalConstraint& additionalConstraint_p,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger);

/**
 * \brief creates a binary variable and its corresponding linking constraint
 *
 * \param master_p solver to which the binary variable and the linking
 * constraint will be added \param variablesToBinarise_p map listing the
 * variables to add and their corresponding ones
 *
 * for each entry (BinVar, CorrespondingVar) from the input map,
 *          creates the binary variable BinVar
 *          adds the linking constraint link_BinVar_CorrespondingVar :
 * CorrespondingVar  <= UB(CorrespondingVar) * BinVar
 */
void addBinaryVariables(
    SolverAbstract::Ptr master_p,
    std::map<std::string, std::string> const& variablesToBinarise_p,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger);

/**
 * \brief return Active Links Builder
 * \param root  path corresponding to the path to the simulation output
 * directory containing the lp directory \return ActiveLinksBuilder object
 */
ActiveLinksBuilder get_link_builders(
    const std::filesystem::path& xpansion_output_dir,
    ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger);
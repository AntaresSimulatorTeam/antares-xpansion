//
// Created by marechaljas on 27/10/23.
//

#pragma once

#include <antares/api/singleProblemGetter.h>
#include <antares/solver/lps/LpsFromAntares.h>

#include "ConfigurationManager.h"
#include "antares-xpansion/bellman_values/ProblemManager.h"

/// @brief Class to generate and modify problems in memory
class ProblemGenerationOptimSimu
{
public:
    explicit ProblemGenerationOptimSimu(ConfigurationManager::ConfigDirectories directories,
                                        Logger logger,
                                        std::shared_ptr<ProblemManager> problemManager,
                                        unsigned int startWeek = 1,
                                        unsigned int endWeek = 52);
    virtual ~ProblemGenerationOptimSimu() = default;
    ConfigurationManager::ConfigDirectories
      directories;          /// Directories, used for the original problems generation
    unsigned int startWeek; /// Start week of the problems to take into account
    unsigned int endWeek;   /// End week of the problems to take into account
    std::shared_ptr<ProblemManager> problemManager; /// The manager taking care of reading problems
                                                    /// from disk, problem formats, etc.
    Logger logger;                                  /// Logger used

private:
    void loadProblemsFromAntares();
    void generateAntaresProblems(Antares::Solver::SingleProblemGetter& spg);
    void performAntaresSimulation();
    void lpsToProblems(const Antares::Solver::LpsFromAntares& lps);
};
